#pragma once 

#ifdef _WIN32
#include <windows.h>
#endif

#include "Util.hpp"
#include "ProjectData.hpp"

namespace NeoShafa {
	class ProjectConfigure {
	public:
		ProjectConfigure() = default;
		~ProjectConfigure() = default;

		inline ProjectConfigure(
			const ProjectEnvironment* projectEnvironment,
			const ProjectStatistics* projectStatistics
		) noexcept : m_projectEnvironment(projectEnvironment), m_projectStatistics(projectStatistics) {}

		inline std::expected<void, Core::ProjectConfigureErrors> setup_project_folders()
		{
			if (std::filesystem::exists(m_projectEnvironment->projectCachePath))
				std::println(
					"LOG: {} folder already exists.",
					m_projectEnvironment->projectCachePath.filename().string()
				);
			else {
				std::filesystem::create_directory(m_projectEnvironment->projectCachePath);
#ifdef _WIN32
				uint32_t attrs = ::GetFileAttributesW(
					m_projectEnvironment->projectCachePath.wstring().c_str()
				);
				if (attrs == INVALID_FILE_ATTRIBUTES) {
					std::println(
						std::cerr,
						"ERROR: GetFileAttributes failed {} ",
						GetLastError());
					return std::unexpected(Core::ProjectConfigureErrors::GetFileAttributesError);
				}
				if (!::SetFileAttributesW(
					m_projectEnvironment->projectCachePath.wstring().c_str(),
					attrs | FILE_ATTRIBUTE_HIDDEN)
					)
				{
					std::println(
						std::cerr, 
						"ERROR: SetFileAttributes failed {}",GetLastError()
					);
					return std::unexpected(Core::ProjectConfigureErrors::SetFileAttributesError);
				}
#endif
			}

			if (std::filesystem::exists(m_projectEnvironment->projectBinaryFolderPath))
				std::println(
					"LOG: {} folder already exists.",
					m_projectEnvironment->projectBinaryFolderPath.filename().string()
				);
			else std::filesystem::create_directory(m_projectEnvironment->projectBinaryFolderPath);
		
			return {};
		}

		inline std::expected<void, Core::ProjectConfigureErrors> get_all_source_files()
		{
			if (!m_projectEnvironment) {
				return std::unexpected(Core::ProjectConfigureErrors::InvalidEnvironmentError);
			}

			const std::vector<std::string> sourceExtensions { ".cpp", ".cxx", ".inl" };

			try
			{
				for (const auto& entry : std::filesystem::recursive_directory_iterator(m_projectEnvironment->projectRoot))
				{
					if (entry.is_regular_file())
					{
						const std::string extension = entry.path().extension().string();
						if (std::find(sourceExtensions.begin(), sourceExtensions.end(), extension) != sourceExtensions.end())
						{
							 auto res = Util::hash(entry.path());
							 if (!res.has_value())
								 return std::unexpected(Core::ProjectConfigureErrors::GeneratinFileHashError);
							 size_t fileHash = res.value();
							m_sourceFiles.push_back(std::make_pair(fileHash, entry.path()));
						}
					}
				}
			}
			catch (const std::filesystem::filesystem_error& e)
			{
				std::println(
					std::cerr,
					"ERROR: Failed to iterate through source files: {}",
					e.what()
				);
				return std::unexpected(Core::ProjectConfigureErrors::DirectoryIterationError);
			}

			save_source_cache();

			return {};

		}

		inline std::expected<void, Core::ProjectConfigureErrors> save_source_cache()
		{
			for (const auto& [hash, path] : m_sourceFiles)
			{
				std::string content{ std::to_string(hash) + "@" + path.string() };
				Util::write(m_projectEnvironment->projectSourceCacheFilePath, content);
			}

			return {};
		}

	private:
		const ProjectEnvironment* m_projectEnvironment{};
		const ProjectStatistics* m_projectStatistics{};

		std::vector<std::pair<size_t, std::filesystem::path>> m_sourceFiles{};
	};

}