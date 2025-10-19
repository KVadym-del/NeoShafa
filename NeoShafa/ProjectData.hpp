#pragma once

#include <filesystem>
#include <string_view>
#include <print>
#include <vector>
#include <variant>
#include <unordered_map>
#include <array>
#include <algorithm>

#include "Util.hpp"

namespace NeoShafa {
	using basicUnified = std::variant<
		std::string*,
		std::vector<std::string>*,
		Core::SupportedCompilers*,
		Core::SupportedTargets*
	>;

	static constexpr std::string_view g_projectConfigureFileName{ "config.toml" };

	static constexpr std::string_view g_projectCacheFolderName{ ".shafaCache" };
	static constexpr std::string_view g_projectSourceCacheFileName{ "source.cache" };

	static constexpr std::string_view g_projectCacheBinaryFolderName{ "bin" };
	static constexpr std::string_view g_projectMsvcFinderUrl{ 
		"https://github.com/microsoft/vswhere/releases/download/3.1.7/vswhere.exe" 
	};
	static constexpr std::string_view g_projectMsvcFinderFileName{ "vswhere.exe" };


	static constexpr std::string_view g_projectBinaryFolderName{ "bin" };

	struct ProjectEnvironment
	{
		inline ProjectEnvironment(void)
		{
			try
			{
				projectRoot = std::filesystem::current_path();
				projectCachePath = projectRoot / g_projectCacheFolderName;
				projectCacheBinaryFolderPath = projectCachePath / g_projectCacheBinaryFolderName;
				projectMsvcFinderFilePath = projectCacheBinaryFolderPath / g_projectMsvcFinderFileName;
				projectBinaryFolderPath = projectRoot / g_projectBinaryFolderName;
				projectSourceCacheFilePath = projectCachePath / g_projectSourceCacheFileName;
			}
			catch (const std::exception& exception)
			{
				std::println("ERROR: {}", exception.what());
			}
		}

		std::filesystem::path neoShafaPath{};

		std::filesystem::path projectRoot{};
		std::filesystem::path projectCachePath{};
		std::filesystem::path projectCacheBinaryFolderPath{};
		std::filesystem::path projectMsvcFinderFilePath{};
		std::filesystem::path projectSourceCacheFilePath{};
		std::filesystem::path projectBinaryFolderPath{};
	};

	struct ProjectCompilationData {
		std::string projectType{};
		struct SupportedProjectTypes {
			static constexpr std::array<std::string_view, 3> supportedProjectTypes{
				"Executable",
				"StaticLibrary",
				"DynamicLibrary"
			};
			enum supportedProjectTypesEnum {
				Executable,
				StaticLibrary,
				DynamicLibrary
			};

			constexpr const auto& operator*() const {
				return supportedProjectTypes;
			}
		};
		static constexpr SupportedProjectTypes supportedProjectTypes;

#ifdef _WIN32
		Core::SupportedCompilers projectCompilers{ Core::SupportedCompilers::MSVC };
		Core::SupportedTargets projectTargets{ Core::SupportedTargets::Windows };
#elifdef linux
		Core::SupportedCompilers projectCompilers{ Core::SupportedCompilers::GCC };
		Core::SupportedTargets projectTargets{ Core::SupportedTargets::Linux };
#elif
		Core::SupportedCompilers projectCompilers{ Core::SupportedCompilers::Clang };
		Core::SupportedTargets projectTargets{ Core::SupportedTargets::Unknown };
#endif

		std::string cCompilerVersion{"c99"};
		std::string cCompilerPath{};

		std::string cppCompilerVersion{"c++20"};
		std::string cppCompilerPath{};

		std::string projectLibPath{};

		std::string projectLinkerPath{};

		std::vector<std::string> cCompilerFlags{};
		std::vector<std::string> cppCompilerFlags{};
		std::vector<std::string> msvcCompilerFlags{};
		std::vector<std::string> projectLibFlags{};
		std::vector<std::string> MSVCProjectLibFlags{};
		std::vector<std::string> projectLinkerFlags{};
		std::vector<std::string> MSVCProjectLinkerFlags{};
	};

	struct ProjectStatistics {
		inline ProjectStatistics() {
			std::string_view nameToHash{ "ProjectName" };
			variablesSignatures.emplace(
				Util::hash(nameToHash),
				&projectName
			);
			nameToHash = "ProjectVersion";
			variablesSignatures.emplace(
				Util::hash(nameToHash),
				&projectVersion
			);
			nameToHash = "ProjectLanguage";
			variablesSignatures.emplace(
				Util::hash(nameToHash),
				&projectLanguage
			);
			nameToHash = "ProjectType";
			variablesSignatures.emplace(
				Util::hash(nameToHash),
				&projectCompilationData.projectType
			);
			nameToHash = "ProjectCompilers";
			variablesSignatures.emplace(
				Util::hash(nameToHash),
				&projectCompilationData.projectCompilers
			);
			nameToHash = "ProjectTargets";
			variablesSignatures.emplace(
				Util::hash(nameToHash),
				&projectCompilationData.projectTargets
			);
			nameToHash = "cCompilerVersion";
			variablesSignatures.emplace(
				Util::hash(nameToHash),
				&projectCompilationData.cCompilerVersion
			);
			nameToHash = "cCompilerFlags";
			variablesSignatures.emplace(
				Util::hash(nameToHash),
				&projectCompilationData.cCompilerFlags
			);
			nameToHash = "cppCompilerVersion";
			variablesSignatures.emplace(
				Util::hash(nameToHash),
				&projectCompilationData.cppCompilerVersion
			);
			nameToHash = "cppCompilerFlags";
			variablesSignatures.emplace(
				Util::hash(nameToHash),
				&projectCompilationData.cppCompilerFlags
			);
			nameToHash = "msvcCompilerFlags";
			variablesSignatures.emplace(
				Util::hash(nameToHash),
				&projectCompilationData.msvcCompilerFlags
			);


			nameToHash = "ProjectPrebuild";
			variablesSignatures.emplace(
				Util::hash(nameToHash),
				&projectPrebuild
			);
			nameToHash = "ProjectPostbuild";
			variablesSignatures.emplace(
				Util::hash(nameToHash),
				&projectPostbuild
			);

			nameToHash = "projectLibFlags";
			variablesSignatures.emplace(
				Util::hash(nameToHash),
				&projectCompilationData.projectLibFlags
			);
			nameToHash = "MSVCProjectLibFlags";
			variablesSignatures.emplace(
				Util::hash(nameToHash),
				&projectCompilationData.MSVCProjectLibFlags
			);

			nameToHash = "projectLinkerFlags";
			variablesSignatures.emplace(
				Util::hash(nameToHash),
				&projectCompilationData.projectLinkerFlags
			);
			nameToHash = "MSVCProjectLinkerFlags";
			variablesSignatures.emplace(
				Util::hash(nameToHash),
				&projectCompilationData.MSVCProjectLinkerFlags
			);

		}

		constexpr static inline bool is_project_type_supported(std::string_view projectType) {
			return std::ranges::find(*ProjectCompilationData::supportedProjectTypes, projectType) != (*ProjectCompilationData::supportedProjectTypes).end();
		}

		// ----------------Required Project Information----------------

		std::string projectName{};
		std::string projectVersion{};
		std::string projectLanguage{};

		// ----------------Optional Project Information----------------
		std::string projectPrebuild{};
		std::string projectPostbuild{};

		std::unordered_map<size_t, basicUnified> variablesSignatures{};

		ProjectCompilationData projectCompilationData{};
	};
	
}