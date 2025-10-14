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
			ProjectStatistics* projectStatistics
		) noexcept : m_projectEnvironment(projectEnvironment), m_projectStatistics(projectStatistics) {}

		inline Core::ExpectedVoid setup_project_folders()
		{
			if (std::filesystem::exists(m_projectEnvironment->projectCachePath))
				std::println(
					"LOG: {} folder already exists.",
					m_projectEnvironment->projectCachePath.filename().string()
				);
			else {
				std::filesystem::create_directory(m_projectEnvironment->projectCachePath);
#ifdef _WIN32
				const uint32_t attrs = ::GetFileAttributesW(
					m_projectEnvironment->projectCachePath.wstring().c_str()
				);
				if (attrs == INVALID_FILE_ATTRIBUTES) {
					return std::unexpected(
						Core::make_error(
							Core::ErrorCode::GetFileAttributesError,
							std::format("GetFileAttributes failed {}", GetLastError())
						)
					);
				}
				if (!::SetFileAttributesW(
					m_projectEnvironment->projectCachePath.wstring().c_str(),
					attrs | FILE_ATTRIBUTE_HIDDEN)
					)
				{
					return std::unexpected(
						Core::make_error(
							Core::ErrorCode::SetFileAttributesError,
							std::format("SetFileAttributes failed {}", GetLastError())
						)
					);
				}
#endif
			}

			if (std::filesystem::exists(m_projectEnvironment->projectCacheBinaryFolderPath))
				std::println(
					"LOG: {} folder already exists.",
					m_projectEnvironment->projectCacheBinaryFolderPath.filename().string()
				);
			else std::filesystem::create_directory(m_projectEnvironment->projectCacheBinaryFolderPath);

			if (std::filesystem::exists(m_projectEnvironment->projectBinaryFolderPath))
				std::println(
					"LOG: {} folder already exists.",
					m_projectEnvironment->projectBinaryFolderPath.filename().string()
				);
			else std::filesystem::create_directory(m_projectEnvironment->projectBinaryFolderPath);
		
			return {};
		}

		inline Core::ExpectedVoid get_all_source_files()
		{
			if (!m_projectEnvironment) {
				return std::unexpected(
					Core::make_error(
						Core::ErrorCode::InvalidEnvironmentError, "Environment iis not valid.")
				);
			}

			const std::array<std::string, 4> sourceExtensions { ".cpp", ".cxx", ".inl", ".toml" };

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
								 return std::unexpected(
									 Core::make_error(
										 Core::ErrorCode::GeneratinFileHashError,
										 "Cannot generate hash.")
								 );
							 size_t fileHash = res.value();
							m_sourceFiles.push_back(std::make_pair(fileHash, entry.path()));
						}
					}
				}
			}
			catch (const std::filesystem::filesystem_error& e)
			{
				return std::unexpected(
					Core::make_error(
						Core::ErrorCode::DirectoryIterationError,
						std::format("Failed to iterate through source files: {}", e.what())
					)
				);
			}

			return {};

		}

		inline Core::ExpectedVoid save_source_cache()
		{
			bool firstTime{ true };
			for (const auto& [hash, path] : m_sourceFiles)
			{
				std::string content{ std::to_string(hash) + m_sourceCacheDelimiter + path.string() + "\n"};
				if (firstTime) {
					Util::write(m_projectEnvironment->projectSourceCacheFilePath, content);
					firstTime = false;
				}
				else
					Util::write(m_projectEnvironment->projectSourceCacheFilePath, content, true);
			}

			return {};
		}
		inline Core::ExpectedVoid create_source_cache() {
			Util::write(m_projectEnvironment->projectSourceCacheFilePath, "");
			return {};
		}

		inline Core::Expected<std::vector<std::pair<size_t, std::filesystem::path>>> get_source_cache() {
			std::vector<std::pair<size_t, std::filesystem::path>> sourceFiles{};
			auto res = Util::read(m_projectEnvironment->projectSourceCacheFilePath);
			if (!res) {
				return std::unexpected(
					Core::make_error(
						Core::ErrorCode::ReadingProjectCahedSourceError,
						std::format("Reading project cahed source error: {}({})", res.error().message, static_cast<int32_t>(res.error().code))
					)
				);
			}
			std::vector<std::string> filesContext = res.value();
			for (auto& string : filesContext)
			{
				std::vector<std::string_view> filteredString = split(string);
				sourceFiles.push_back(
					std::make_pair(
						std::stoull(filteredString.at(0).data()),
						filteredString.at(1)
					)
				);

			}

			return sourceFiles;
		}

		inline Core::Expected<std::vector<std::pair<size_t, std::filesystem::path>>> get_difference_source_cache() {
			if (!m_projectStatistics) {
				return std::unexpected(
					Core::make_error(
						Core::ErrorCode::GenericProjectConfigureError, "Project statistics is not valid."
					)
				);
			}
			auto res = get_source_cache();
			if (!res.has_value()) {
				return std::unexpected(
					Core::make_error(
						Core::ErrorCode::ReadingProjectCahedSourceError, "Reading project cahed source error."
					)
				);
			}
			std::vector<std::pair<size_t, std::filesystem::path>> sourceFiles{ res.value() };
			std::vector<std::pair<size_t, std::filesystem::path>> differenceFiles{};
			for (const auto& [hash, path] : m_sourceFiles)
			{
				if (std::find_if(
					sourceFiles.begin(),
					sourceFiles.end(),
					[&](const auto& pair) {
						return pair.first == hash && pair.second == path;
					}
				) == sourceFiles.end())
				{
					differenceFiles.push_back(std::make_pair(hash, path));
				}
			}
			return differenceFiles;
		}



		inline Core::ExpectedVoid where_is_cl()
		{
			if (!std::filesystem::exists(m_projectEnvironment->projectMsvcFinderFilePath))
			{
				auto res = Util::download_file(
					g_projectMsvcFinderUrl,
					m_projectEnvironment->projectMsvcFinderFilePath
				);
				if (!res)
				{
					std::println(
						"WARNING: Cannot download file with error: {}({}).",
						res.error().message,
						static_cast<int32_t>(res.error().code)
					);
				}
			}

			std::vector<std::string> vswhereArgs {
				"-latest", "-prerelease", "-products", "*",
				"-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
				"-property", "installationPath"
			};

			int32_t exitCode{};
			auto res = Util::run_command(
				m_projectEnvironment->projectMsvcFinderFilePath.string(),
				vswhereArgs,
				exitCode
			);
			if (!res) return std::unexpected(res.error());
			
			std::filesystem::path installDir = res.value();


			if (installDir.empty()) {
				return std::unexpected(
					Core::make_error(
						Core::ErrorCode::InvalidInstallationDirectoryError, "Could not find Visual Studio installation directory. Please ensure Visual Studio is installed and the required components are present."
					)
				);
			}

			const std::string hostArch{ "HostX64" };
			const std::string targetArch{ "x64" };

			std::filesystem::path version_file{ 
				installDir /
				"VC" /
				"Auxiliary" /
				"Build" /
				"Microsoft.VCToolsVersion.default.txt"
			};
			auto resFile = Util::read(version_file);
			if (!resFile) {
				return std::unexpected(
					Core::make_error(
						Core::ErrorCode::InvalidMsvcVersionError, std::format("Could not read MSVC version file at: {}.", version_file.string())
					)
				);
			}

			std::string msvcVersion = resFile.value().front();
			if (msvcVersion.empty()) {
				return std::unexpected(
					Core::make_error(
						Core::ErrorCode::InvalidMsvcVersionError, std::format("MSVC version is empty in file: {}.", version_file.string())
					)
				);
			}
			boost::trim(msvcVersion);

			std::filesystem::path msvc_tools_path{ 
				installDir /
				"VC" /
				"Tools" /
				"MSVC" /
				msvcVersion /
				"bin" /
				hostArch /
				targetArch
			};
			std::filesystem::path cl_path{ msvc_tools_path / "cl.exe" };
			std::filesystem::path lib_path{ msvc_tools_path / "lib.exe" };
			std::filesystem::path link_path{ msvc_tools_path / "link.exe" };
			
			if (!std::filesystem::exists(cl_path)) {
				return std::unexpected(
					Core::make_error(
						Core::ErrorCode::InvalidClPathError, std::format("Could not find cl.exe at path: {}.", cl_path.string())
					)
				);
			}
			else if (!std::filesystem::exists(lib_path))
			{
				return std::unexpected(
					Core::make_error(
						Core::ErrorCode::InvalidLibPathError, std::format("Could not find lib.exe at path: {}.", lib_path.string())
					)
				);
			}
			else if (!std::filesystem::exists(link_path))
			{
				return std::unexpected(
					Core::make_error(
						Core::ErrorCode::InvalidLinkPathError, std::format("Could not find link.exe at path: {}.", link_path.string())
					)
				);
			}

			m_projectStatistics->projectCompilationData.cCompilerPath = cl_path.string();
			m_projectStatistics->projectCompilationData.cppCompilerPath = cl_path.string();
			m_projectStatistics->projectCompilationData.projectLibPath = lib_path.string();	
			m_projectStatistics->projectCompilationData.projectLinkerPath = link_path.string();	
			

			std::println("INFO: Compiler paths set to: \ncCompilerPath = {}, \ncppCompilerPath = {} \nprojectLibPath = {}, \nprojectLinkerPath = {}",
				m_projectStatistics->projectCompilationData.cCompilerPath,
				m_projectStatistics->projectCompilationData.cppCompilerPath,
				m_projectStatistics->projectCompilationData.projectLibPath,
				m_projectStatistics->projectCompilationData.projectLinkerPath
			);

			return {};
		}

	public:
		std::vector<std::string_view> split(const std::string& string) const {
			std::vector<std::string_view> tokens{};
			size_t start = 0;
			size_t end = 0;

			while ((end = string.find(m_sourceCacheDelimiter, start)) != std::string::npos) {
				tokens.emplace_back(string.data() + start, end - start);
				start = end + 1;
			}

			if (start < string.length()) {
				tokens.emplace_back(string.data() + start, string.length() - start);
			}

			return tokens;
		}

	public:
		inline const std::vector<std::pair<size_t, std::filesystem::path>>& get_source_files() const {
			return m_sourceFiles;
		}

	private:
		const ProjectEnvironment* m_projectEnvironment{};
		ProjectStatistics* m_projectStatistics{};

		constexpr static const char m_sourceCacheDelimiter{ '@' };

		std::vector<std::pair<size_t, std::filesystem::path>> m_sourceFiles{};
	};

}