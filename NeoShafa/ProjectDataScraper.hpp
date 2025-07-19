#pragma once

#include <expected>
#include <functional>
#include <string>
#include <variant>

#include <toml.hpp>

#include "Util.hpp"
#include "ProjectData.hpp"

namespace NeoShafa {
	class ProjectDataScraper {
	public:
		ProjectDataScraper() = default;
		~ProjectDataScraper() = default;

		inline ProjectDataScraper(
			const ProjectEnvironment* projectEnvironment,
			ProjectStatistics* projectStatistics
		) noexcept : m_projectEnvironment{ projectEnvironment }, m_projectStatistics(projectStatistics) {}

	public:
		inline std::expected<void, Core::ProjectDataScraperErrors> project_setup()
		{
			if (
				const auto err = Util::is_null(
					m_projectEnvironment,
					"Project environment is null."
				); !err) {
				return std::unexpected(Core::ProjectDataScraperErrors::GenericProjectSetupError);
			}

			if (
				const auto err = Util::is_null(
					m_projectStatistics,
					"Project statistics is null."
				); !err) {
				return std::unexpected(Core::ProjectDataScraperErrors::GenericProjectSetupError);
			}

			const std::filesystem::path projectConfigFilePath = m_projectEnvironment->projectRoot / g_projectConfigureFileName;

			if (!std::filesystem::exists(projectConfigFilePath))
				return std::unexpected(Core::ProjectDataScraperErrors::ProjectConfigFileNotExist);


			auto tryData = toml::try_parse(
				projectConfigFilePath,
				toml::spec::v(1, 1, 0)
			);
			if (!tryData.is_ok())
				return std::unexpected(Core::ProjectDataScraperErrors::UnexpectedParsingError);

			auto data = tryData.unwrap();

			std::function<std::expected<void, Core::ProjectDataScraperErrors>(
				std::string_view,
				Core::ProjectDataScraperErrors
			)> check_for_required_key = [&](
				std::string_view arg_key,
				Core::ProjectDataScraperErrors error
				) -> std::expected<void, Core::ProjectDataScraperErrors> {
				const std::string_view key{ arg_key };
				if (data.contains(key.data()) && data.at(key.data()).is_string()) {
					const auto mapKey = Util::hash(key);

					if (const auto it = m_projectStatistics->variablesSignatures.find(mapKey);
						it != m_projectStatistics->variablesSignatures.end()) {
						*std::get<std::string*>(it->second) = data.at(key.data()).as_string();
						// TODO: cast to other types                       .
						return {};
					}
					return std::unexpected(Core::ProjectDataScraperErrors::UnexpectedParsingError);

				}
				else
					return std::unexpected(error);
			};

			std::function<void(std::string_view)> check_for_key = [&](
				std::string_view arg_key
			) {
				const std::string_view key{ arg_key };
				if (data.contains(key.data()) && data.at(key.data()).is_string()) {
					const auto mapKey = Util::hash(key);
					if (const auto it = m_projectStatistics->variablesSignatures.find(mapKey);
						it != m_projectStatistics->variablesSignatures.end()) {
						std::string dataStr = data.at(key.data()).as_string();
						if (dataStr.empty()) std::println("WARNING: field of {} is empty.", key.data());
						*std::get<std::string*>(it->second) = dataStr;
					}
				}
			};

			auto res = check_for_required_key(
				"ProjectName",
				Core::ProjectDataScraperErrors::MissingProjectName
			);
			if (!res) return res;

			res = check_for_required_key(
				"ProjectVersion",
				Core::ProjectDataScraperErrors::MissingProjectVersion
			);
			if (!res) return res;

			res = check_for_required_key(	
				"ProjectLanguage",
				Core::ProjectDataScraperErrors::MissingProjectLanguage
			);
			if (!res) return res;

			res = check_for_required_key(
				"ProjectType",
				Core::ProjectDataScraperErrors::MissingProjectType
			);
			if (!res) return res;
			if (!ProjectStatistics::is_project_type_supported(
				m_projectStatistics->projectCompilationData.projectType)
				)
				return std::unexpected(Core::ProjectDataScraperErrors::UnexpectedProjectTypeError);
			
			check_for_key("ProjectPrebuild");
			check_for_key("ProjectPostbuild");

			return {};
		}

	private:
		const ProjectEnvironment* m_projectEnvironment{};
		ProjectStatistics* m_projectStatistics{};
	};
}