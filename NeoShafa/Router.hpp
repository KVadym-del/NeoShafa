#pragma once  

#define _CRT_SECURE_NO_WARNINGS

#include <print>  
#include <string_view>  
#include <vector>  
#include <iostream>

#include <gsl/gsl>

#include <boost/program_options.hpp>

#include "Util.hpp"
#include "ProjectData.hpp"
#include "ProjectDataScraper.hpp"
#include "ProjectConfigure.hpp"
#include "ProjectBuild.hpp"

namespace NeoShafa {
    using namespace boost;
    class Router {
    public:
        inline explicit Router(int32_t argc, char** argv) {
            auto result = Util::is_null(argv);
            if (!result) {
                auto errorValue = static_cast<int32_t>(result.error());
                std::println(std::cerr, "ERROR: {}", errorValue);

                exit(errorValue);
            }
            const gsl::span<char*> spanArgs{ argv, static_cast<size_t>(argc) };
            const size_t argSize{ spanArgs.size() };
            m_cmdArgs.reserve(argSize);
            for (int32_t i = 0; i < argSize; ++i) {
                m_cmdArgs.emplace_back(gsl::at(spanArgs, i));
            }

            m_projectEnvironment.neoShafaPath = m_cmdArgs.at(0);
			m_projectStatistics.projectCompilationData = m_projectCompilationData;
            m_projectDataScraper = { &m_projectEnvironment, &m_projectStatistics };
        }

        std::expected <void, Core::RouterErrors> run() {
            try {
                auto addOptions = m_description.add_options();
                addOptions("help,h", "Produce help message.");
                addOptions("version,v", "Print version information and exit.");
                addOptions("configure,c", "Configure the application.");
				addOptions("build,b", "Build the project.");
				addOptions("full_build,B", "Build the project.");
				addOptions("compilers", "List available compilers.");
				addOptions("targets", "List available targets.");

                program_options::store(
                    program_options::command_line_parser(m_cmdArgs)
                    .options(m_description)
                    .run(),
                    m_variableMap
                );

                check_help();
                check_version();
                check_configure();

                // TODO: without config there is no sourcecache, so there is memory error.
                check_build();
                check_full_build();

                program_options::notify(m_variableMap);
            }
            catch (const program_options::error& exception) {
                // TODO: Add more errors to the enum, for now just print error.
                std::println(std::cerr, "TEMP(ERROR): {}", exception.what());
                return std::unexpected(Core::RouterErrors::GenericError);
            }
            return {};
        }

    private:
        void check_help() {
            if (m_variableMap.count("help")) {
                std::stringstream buffer{};
                buffer << m_description;
                std::println("{}", buffer.str());
                exit(0);
            }
        }

        void check_version() {
            if (m_variableMap.count("version")) {
                std::println(
                    "Current version is: {}", std::string{ Core::NEOSHAFA_VERSION }
                ); 
                exit(0);
            }
		}

        void check_compilers() {
            if (m_variableMap.count("compilers")) {
                //m_projectDataScraper.print_available_compilers();
                exit(0);
            }
		}

        void scrape_data()
        {
            if (const auto res = m_projectDataScraper.project_setup();
                !res
                )
                std::println("ERROR: {}", static_cast<int32_t>(res.error()));
        }
        void configure()
        {
            scrape_data();
            if (const auto res = m_projectConfigure.setup_project_folders();
                !res
                )
                std::println("ERROR: {}", static_cast<int32_t>(res.error()));

            if (const auto res = m_projectConfigure.get_all_source_files();
                !res
                )
                std::println("ERROR: {}", static_cast<int32_t>(res.error()));
            m_projectConfigure.create_source_cache();
#ifdef _WIN32
            if (const auto res = m_projectConfigure.where_is_cl();
                !res
                )
                std::println("ERROR: {}", static_cast<int32_t>(res.error()));
#endif
        }

        void check_configure() {
            if (m_variableMap.count("configure"))
                configure();
        }

        void check_build()
        {
            if (m_variableMap.count("build"))
            {
                scrape_data();
                if (const auto res = m_projectConfigure.get_all_source_files();
                    !res
                    )
                    std::println("ERROR: {}", static_cast<int32_t>(res.error()));
#ifdef _WIN32
                if (const auto res = m_projectConfigure.where_is_cl();
                    !res
                    )
                    std::println("ERROR: {}", static_cast<int32_t>(res.error()));
#endif

                std::vector<std::filesystem::path> diffSource{};
                auto res = m_projectConfigure.get_difference_source_cache();
                if (!res) std::println(std::cerr, "ERROR: code({})", static_cast<int32_t>(res.error()));
                
                if (res->empty()) {
                    std::println("INFO: No source files to compile, skipping compilation step.");
                    return; 
                };
                for (const auto& [_, path] : *res)
                    diffSource.push_back(path);
                if (const auto resScope = m_projectBuild.full_build(diffSource);
                    !resScope
                    )
                    std::println("ERROR: {}", static_cast<int32_t>(resScope.error()));
                m_projectConfigure.save_source_cache(); // TODO: Do not save if an error.
            }
        }

        void check_full_build() {
            if (m_variableMap.count("full_build"))
            {
                configure();
                std::vector<std::filesystem::path> diffSource{};
                auto res = m_projectConfigure.get_difference_source_cache();
                if (!res) std::println(std::cerr, "ERROR: code({})", static_cast<int32_t>(res.error()));

                if (res->empty()) return;
                for (const auto& [_, path] : *res)
                    diffSource.push_back(path);
                if (const auto resScope = m_projectBuild.full_build(diffSource);
                    !resScope
                    )
                    std::println("ERROR: {}", static_cast<int32_t>(resScope.error()));
                m_projectConfigure.save_source_cache();
            }
        }

    private:
        std::vector<std::string> m_cmdArgs{};

        program_options::options_description m_description{ "Allowed options" };
        program_options::variables_map m_variableMap{};

		ProjectCompilationData m_projectCompilationData{};
        ProjectEnvironment m_projectEnvironment{};
        ProjectStatistics m_projectStatistics{};

        ProjectDataScraper m_projectDataScraper{ &m_projectEnvironment, &m_projectStatistics };
        ProjectConfigure m_projectConfigure{ &m_projectEnvironment, &m_projectStatistics };
        ProjectBuild m_projectBuild{ &m_projectEnvironment, &m_projectStatistics };
    };
}
