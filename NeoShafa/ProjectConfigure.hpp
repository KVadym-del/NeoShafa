#pragma once 

#ifdef _WIN32
#include <windows.h>
#endif

#include <boost/process.hpp>

#include "Util.hpp"
#include "ProjectData.hpp"

namespace NeoShafa {
	namespace BoostProcess = boost::process;

	class ProjectConfigure {
	public:
		ProjectConfigure() = default;
		~ProjectConfigure() = default;

		inline ProjectConfigure(
			const ProjectEnvironment* projectEnvironment,
			ProjectStatistics* projectStatistics
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
				const uint32_t attrs = ::GetFileAttributesW(
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

			return {};

		}

		inline std::expected<void, Core::ProjectConfigureErrors> save_source_cache()
		{
			bool firstTime{ true };
			for (const auto& [hash, path] : m_sourceFiles)
			{
				std::string content{ std::to_string(hash) + m_sourceCacheDelimiter + path.string() };
				if (firstTime) {
					Util::write(m_projectEnvironment->projectSourceCacheFilePath, content);
					firstTime = false;
				}
				else
					Util::write(m_projectEnvironment->projectSourceCacheFilePath, content, true);
			}

			return {};
		}

		inline std::expected<
			std::vector<std::pair<size_t, std::filesystem::path>>,
			Core::ProjectConfigureErrors
		> get_source_cache() {
			std::vector<std::pair<size_t, std::filesystem::path>> sourceFiles{};
			auto res = Util::read(m_projectEnvironment->projectSourceCacheFilePath);
			if (!res.has_value()) {
				std::println(std::cerr, "ERROR: {}", static_cast<int32_t>(res.error()));
				return std::unexpected(Core::ProjectConfigureErrors::ReadingProjecCahedSourceError);
			}
			std::vector<std::string> filesContext = res.value();
			for (auto& string : filesContext)
			{
				std::vector<std::string_view> filteredString = split(string);
				sourceFiles.push_back(
					std::make_pair(
						std::stoll(filteredString.at(0).data()),
						filteredString.at(1)
					)
				);
			}

			return sourceFiles;
		}

		inline std::expected<
			std::vector<std::pair<size_t, std::filesystem::path>>,
			Core::ProjectConfigureErrors
		> get_difference_source_cache() {
			if (!m_projectStatistics) {
				return std::unexpected(Core::ProjectConfigureErrors::GenericProjectConfigureError);
			}
			auto res = get_source_cache();
			if (!res.has_value()) {
				return std::unexpected(Core::ProjectConfigureErrors::ReadingProjecCahedSourceError);
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

		inline std::expected<std::string, Core::ProjectConfigureErrors> run_command(
			const std::filesystem::path& executable,
			const std::vector<std::string>& args
		) {
			if (!std::filesystem::exists(executable)) {
				std::println(
					std::cerr,
					"ERROR: Executable not found at {}",\
					executable.string()
				);
				return "";
			}

			BoostProcess::ipstream pipeStream{};
			std::string out{};
			try {
				BoostProcess::child child{
					executable.string(),
					args,
					BoostProcess::std_out > pipeStream
				};

				std::stringstream sstream{};
				sstream << pipeStream.rdbuf();
				out = sstream.str();

				child.wait();
				if (child.exit_code() != 0) {
					std::println(
						std::cerr,
						"ERROR: {} exited with an error code: {}",
						g_projectMsvcFinderFileName,
						child.exit_code()
					);
				}
			}
			catch (const boost::process::process_error& error) {
				std::println(
					std::cerr,
					"ERROR: Error executing vswhere.exe. Is it installed at the specified path?\n\t{}",
					error.what()
				);
			}
			if (!out.empty()) {
				out.erase(out.find_last_not_of("\r\n") + 1);
			}
			return out;
		}

		inline std::expected<void, Core::ProjectConfigureErrors> where_is_cl()
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
						"WARNING: Cannot download file with error code({}).",
						static_cast<int32_t>(res.error())
					);
				}
			}

			std::vector<std::string> vswhereArgs {
				"-latest", "-prerelease", "-products", "*",
				"-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
				"-property", "installationPath"
			};

			auto res = run_command(
				m_projectEnvironment->projectMsvcFinderFilePath.string(),
				vswhereArgs
			);
			if (!res) return std::unexpected(res.error());
			
			std::filesystem::path installDir = res.value();


			if (installDir.empty()) {
				std::println(
					"ERROR: Could not find Visual Studio installation directory. Please ensure Visual Studio is installed and the required components are present."
				);
				return std::unexpected(Core::ProjectConfigureErrors::InvalidInstallationDirectoryError);
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
				std::println(
					"ERROR: Could not read MSVC version file at: {}",
					version_file.string()
				);
				return std::unexpected(Core::ProjectConfigureErrors::InvalidMsvcVersionError);
			}

			std::string msvcVersion = resFile.value().front();
			if (msvcVersion.empty()) {
				std::println(
					"ERROR: Could not read MSVC version file at: {}",
					version_file.string()
				);
				return std::unexpected(Core::ProjectConfigureErrors::InvalidMsvcVersionError);
			}
			boost::trim(msvcVersion);

			std::filesystem::path cl_path{ 
				installDir /
				"VC" /
				"Tools" /
				"MSVC" /
				msvcVersion /
				"bin" /
				hostArch /
				targetArch / 
				"cl.exe" 
			};
			
			if (!std::filesystem::exists(cl_path)) {
				std::println(
					"ERROR: Could not find cl.exe at the expected path: {}",
					cl_path.string()
				);
				return std::unexpected(Core::ProjectConfigureErrors::InvalidClPathError);
			}

			m_projectStatistics->projectCompilationData.cCompilerPath = cl_path.string();
			m_projectStatistics->projectCompilationData.cppCompilerPath = cl_path.string();
			

			std::println("INFO: Compiler paths set to: cCompilerPath = {}, cppCompilerPath = {}",
				m_projectStatistics->projectCompilationData.cCompilerPath,
				m_projectStatistics->projectCompilationData.cppCompilerPath);

			return {};
		}

	public:
		std::vector<std::string_view> split(std::string& string) {
			std::vector<std::string_view> tokens{};
			size_t pos{};
			std::string token{};
			while ((pos = string.find(m_sourceCacheDelimiter)) != std::string::npos) {
				token = string.substr(0, pos);
				tokens.push_back(token);
				string.erase(0, pos + (sizeof(m_sourceCacheDelimiter) / sizeof(char)));
			}
			tokens.push_back(string);

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