#pragma once

#include <iostream>

#include "Util.hpp"
#include "ProjectData.hpp"
#include "ProjectLuaScriptStarter.hpp"

namespace NeoShafa {
	class ProjectBuild {
	public:
		ProjectBuild() = default;
		~ProjectBuild() = default;

		inline ProjectBuild(
			const ProjectEnvironment* projectEnvironment,
			ProjectStatistics* projectStatistics
		) noexcept : m_projectEnvironment(projectEnvironment), m_projectStatistics(projectStatistics) {}

		inline std::expected<void, Core::ProjectBuildErrors> full_build(
			const std::vector<std::filesystem::path>& diffSource
		)
		{ 
			if (!m_projectStatistics->projectPrebuild.empty())
				prebuild();

			const auto res = build_to_object(diffSource);
			if (!res) return res;

			if(!m_projectStatistics->projectPostbuild.empty())
				postbuild();

			return {};
		}

		inline std::expected<void , Core::ProjectBuildErrors > build_to_object(
			const std::vector<std::filesystem::path>& diffSource
		) {	
			std::vector<std::string> msvcCompileString {
				std::format("/nologo"),
				std::format("/c"),
				std::format("/Fo:{}", m_projectEnvironment->projectBinaryFolderPath.string().append("\\")),
				std::format("/Fd:{}", m_projectEnvironment->projectBinaryFolderPath.string().append("\\")),
				//std::format("/Fe:{}", (m_projectEnvironment->projectBinaryFolderPath / m_projectStatistics->projectName).string())
			};
			std::string otherCompileString{};

			if (m_projectStatistics->projectCompilationData.projectType == (*ProjectCompilationData::supportedProjectTypes)[ProjectCompilationData::supportedProjectTypes.Executable]) {
				switch (m_projectStatistics->projectCompilationData.projectCompilers)
				{
				case Core::SupportedCompilers::MSVC:
					for (const auto& filePath : diffSource)
						msvcCompileString.push_back(std::format("{}", filePath.string()));
					break;
				case Core::SupportedCompilers::Clang:
				case Core::SupportedCompilers::GCC:

					break;
				default:
					break;
				}
			}
			else if (m_projectStatistics->projectCompilationData.projectType == (*ProjectCompilationData::supportedProjectTypes)[ProjectCompilationData::supportedProjectTypes.StaticLibrary]) {
				switch (m_projectStatistics->projectCompilationData.projectCompilers)
				{
				case Core::SupportedCompilers::MSVC:
					msvcCompileString.push_back(std::format(" /LD "));
					for (const auto& filePath : diffSource)
						msvcCompileString.push_back(std::format(" {} ", filePath.string()));
					break;
				case Core::SupportedCompilers::Clang:
				case Core::SupportedCompilers::GCC:

					break;
				default:
					break;
				}
			}
			else if (m_projectStatistics->projectCompilationData.projectType == (*ProjectCompilationData::supportedProjectTypes)[ProjectCompilationData::supportedProjectTypes.DynamicLibrary]) {
				switch (m_projectStatistics->projectCompilationData.projectCompilers)
				{
				case Core::SupportedCompilers::MSVC:
					for (const auto& filePath : diffSource)
						msvcCompileString.push_back(std::format(" {} ", filePath.string()));
					break;
				case Core::SupportedCompilers::Clang:
				case Core::SupportedCompilers::GCC:

					break;
				default:
					break;
				}
			}

			for (const auto& str : msvcCompileString)
				std::print(" {} ", str);

			std::println();

			auto res = Util::run_command(
				m_projectStatistics->projectCompilationData.cppCompilerPath,
				msvcCompileString
			);
			if (!res) std::println(std::cerr, "ERROR TEST one: {}", static_cast<int32_t>(res.error()));

			std::string compilingResult = res.value();

			std::println("INFO: Compiling result (\n{}\n)", compilingResult);

			return {};
		}

		inline void prebuild() {
			auto res = ProjectLuaScriptStarter::run(
				m_projectStatistics->projectPrebuild
			);
			if (!res)
			{
				std::println(
					std::cerr,
					"ERROR: Lua error code({})",
					static_cast<int32_t>(res.error())
				);

				return;
			}
		}
		
		inline void postbuild() {
			auto res = ProjectLuaScriptStarter::run(
				m_projectStatistics->projectPostbuild.c_str()
			);
			if (!res)
			{
				std::println(
					std::cerr,
					"ERROR: Lua error code({})",
					static_cast<int32_t>(res.error())
				);

				return;
			}
		}

	private:
		const ProjectEnvironment* m_projectEnvironment{};
		ProjectStatistics* m_projectStatistics{};
	};
}