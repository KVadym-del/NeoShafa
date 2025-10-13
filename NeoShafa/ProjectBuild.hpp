#pragma once

#include <iostream>
#include <print>

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

		inline Core::ExpectedVoid full_build(
			const std::vector<std::filesystem::path>& diffSource
		)
		{ 
			if (!m_projectStatistics->projectPrebuild.empty())
				prebuild();

			const auto res = build_to_object(diffSource);
			if (!res) return res;

			const auto resLink = linking();
			if (!resLink) return resLink;

			if(!m_projectStatistics->projectPostbuild.empty())
				postbuild();

			return {};
		}

		inline Core::ExpectedVoid build_to_object(
			const std::vector<std::filesystem::path>& diffSource
		) {	
			std::vector<std::string> msvcCompileString {
				std::format("/nologo"),
				std::format("/c"),
				std::format("/std:{}", m_projectStatistics->projectCompilationData.cppCompilerVersion),
				std::format("/Fo:{}", m_projectEnvironment->projectBinaryFolderPath.string().append("\\")),
				std::format("/Fd:{}", m_projectEnvironment->projectBinaryFolderPath.string().append("\\")),
				//std::format("/Fe:{}", (m_projectEnvironment->projectBinaryFolderPath / m_projectStatistics->projectName).string())
			};
			std::string otherCompileString{};

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

			std::println("COMPILING");
			for (const auto& str : msvcCompileString)
				std::print(" {} ", str);

			std::println();

			int32_t exitCode{};
			auto res = Util::run_command(
				m_projectStatistics->projectCompilationData.cppCompilerPath,
				msvcCompileString,
				exitCode
			);
			if (!res)
				std::println(std::cerr, "ERROR: {}({})", res.error().message, static_cast<int32_t>(res.error().code));

			std::println("INFO: \n|=>\n{}\n<=|", res.value());

			if (exitCode != 0)
				return std::unexpected(
					Core::make_error(
						Core::ErrorCode::RunningCommandError,
						std::format("Compiler exited with code: {}.", exitCode)
					)
				);

			return {};
		}

		inline Core::ExpectedVoid linking()
		{
			std::vector<std::filesystem::path> objectFiles{};

			for (const auto& entry : std::filesystem::directory_iterator(m_projectEnvironment->projectBinaryFolderPath)) {
				if (entry.is_regular_file()) {
					std::string fileExtension = entry.path().extension().string();
#ifdef _WIN32
					if (fileExtension == ".obj") {
#elif 	
					if (fileExtension == ".o") {
#endif // _WIN32
						objectFiles.push_back(entry.path());
					}
				}
			}

			std::vector<std::string> msvcCompileString{
				std::format("/nologo"), 
			};
			std::string otherCompileString{};

			std::filesystem::path process{};

			for (const auto& path : objectFiles)
				msvcCompileString.push_back(path.string());


			if (m_projectStatistics->projectCompilationData.projectType == (*ProjectCompilationData::supportedProjectTypes)[ProjectCompilationData::supportedProjectTypes.Executable]) {

				switch (m_projectStatistics->projectCompilationData.projectCompilers)
				{
					case Core::SupportedCompilers::MSVC:
					msvcCompileString.push_back(std::format("/Fe:{}", (m_projectEnvironment->projectBinaryFolderPath / m_projectStatistics->projectName).string()));
					process = m_projectStatistics->projectCompilationData.cppCompilerPath;
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

					break;
					case Core::SupportedCompilers::Clang:
					case Core::SupportedCompilers::GCC:

					break;
					default:
					break;
				}
			}

			std::println("\nLINKING");
			for (const auto& str : msvcCompileString)
				std::print(" {} ", str);

			std::println();

			int32_t exitCode{};
			auto res = Util::run_command(
				process,
				msvcCompileString,
				exitCode
			);
			if (!res)
				std::println(std::cerr, "ERROR: {}({})", res.error().message, static_cast<int32_t>(res.error().code));

			std::println("INFO: \n|=>\n{}\n<=|", res.value());

			if (exitCode != 0)
				return std::unexpected(
					Core::make_error(
						Core::ErrorCode::RunningCommandError,
						std::format("Compiler exited with code: {}.", exitCode)
					)
				);

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
					"ERROR: Lua error code: {}({})",
					res.error().message,
					static_cast<int32_t>(res.error().code)
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
					"ERROR: Lua error code: {}({})",
					res.error(). message,
					static_cast<int32_t>(res.error().code)
				);

				return;
			}
		}

	private:
		const ProjectEnvironment* m_projectEnvironment{};
		ProjectStatistics* m_projectStatistics{};
	};
}