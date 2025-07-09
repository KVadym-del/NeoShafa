#pragma once  

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
            m_projectDataScraper = { &m_projectEnvironment, &m_projectStatistics };
        }

        std::expected <void, Core::RouterErrors> run() {
            try {
                auto addOptions = m_description.add_options();
                addOptions("help,h", "Produce help message.");
				addOptions("version,v", "Print version information and exit.");
				addOptions("configure,c", "Configure the application.");

                program_options::store(
                    program_options::command_line_parser(m_cmdArgs)
                    .options(m_description)
                    .run(),
                    m_variableMap
                );

                check_help();
                check_version();
                check_configure();

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

        void check_configure() {
            if (m_variableMap.count("configure")) {
                if (const auto err = m_projectDataScraper.project_setup();
                    !err.has_value()
                    )
                    std::println("ERROR: {}", static_cast<int32_t>(err.error()));

                m_projectConfigure = { &m_projectEnvironment, &m_projectStatistics };
                if (const auto err = m_projectConfigure.setup_project_folders();
                    !err.has_value()
                    )
                    std::println("ERROR: {}", static_cast<int32_t>(err.error()));

                if (const auto err = m_projectConfigure.get_all_source_files();
                    !err.has_value()
                    )
                    std::println("ERROR: {}", static_cast<int32_t>(err.error()));
            }
        }

    private:
        std::vector<std::string> m_cmdArgs{};

        program_options::options_description m_description{ "Allowed options" };
        program_options::variables_map m_variableMap{};

        ProjectEnvironment m_projectEnvironment{};
        ProjectStatistics m_projectStatistics{};

        ProjectDataScraper m_projectDataScraper{};
        ProjectConfigure m_projectConfigure{};
    };
}
