#pragma once

#include <print>
#include <iostream>

extern "C"
{
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}

#include "Util.hpp"

namespace NeoShafa {
	class ProjectLuaScriptStarter {
	public:
		ProjectLuaScriptStarter() = delete;
		~ProjectLuaScriptStarter() = delete;

		inline static std::expected<void, Core::LuaError> run(const std::filesystem::path& scriptPath) {
			int32_t status{};
			lua_State* m_luaState = luaL_newstate();
			luaL_openlibs(m_luaState);

			if (scriptPath.empty()) {
				std::println(std::cerr, "ERROR: No script path provided!");
				return std::unexpected(Core::LuaError::CannotReadFileError);
			}

			if (!std::filesystem::exists(scriptPath)) {
				std::println(
					std::cerr,
					"ERROR: Script file does not exist: {}",
					scriptPath.string()
				);
				return std::unexpected(Core::LuaError::FileNotFoundError);
			}

			status = luaL_dofile(m_luaState, scriptPath.string().c_str());
			if (status) {
				std::println(
					std::cerr,
					"ERROR: Lua error: {}",
					lua_tostring(m_luaState, -1)
				);
				return std::unexpected(Core::LuaError::ExecutionError);
			}

			return {};
		}

	};
}