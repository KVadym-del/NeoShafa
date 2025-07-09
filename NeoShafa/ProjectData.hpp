#pragma once

#include <filesystem>
#include <string_view>
#include <print>
#include <unordered_map>

#include "Util.hpp"

namespace NeoShafa {
	static constexpr std::string_view g_projectConfigureFileName{ "config.toml" };

	static constexpr std::string_view g_projectCacheFolderName{ ".shafaCache" };
	static constexpr std::string_view g_projectSourceCacheFileName{ "source.cache" };

	static constexpr std::string_view g_projectBinaryFolderName{ "bin" };

	struct ProjectEnvironment
	{
		inline ProjectEnvironment(void)
		{
			try
			{
				projectRoot = std::filesystem::current_path();
				projectCachePath = projectRoot / g_projectCacheFolderName;
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
		std::filesystem::path projectSourceCacheFilePath{};
		std::filesystem::path projectBinaryFolderPath{};
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
		}

		std::string projectName{};
		std::string projectVersion{};
		std::string projectLanguage{};

		std::unordered_map<size_t, std::string*> variablesSignatures{};
	};
	
}