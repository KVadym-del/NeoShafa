#pragma once

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
			const ProjectStatistics* projectStatistics
		) noexcept : m_projectEnvironment(projectEnvironment), m_projectStatistics(projectStatistics) {}

		inline std::expected<void, Core::ProjectBuildErrors> full_build()
		{ 
			if (!m_projectStatistics->projectPrebuild.empty())
				prebuild();

			if(!m_projectStatistics->projectPostbuild.empty())
				postbuild();

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
		const ProjectStatistics* m_projectStatistics{};
	};
}