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

		inline static Core::ExpectedVoid run(const std::filesystem::path& scriptPath) {
			int32_t status{};
			lua_State* m_luaState = luaL_newstate();
			luaL_openlibs(m_luaState);

			if (scriptPath.empty())
				return std::unexpected(Core::make_error(Core::ErrorCode::CannotReadFileError, "No script path provided!"));

			if (!std::filesystem::exists(scriptPath))
				return std::unexpected(Core::make_error(Core::ErrorCode::FileNotFoundError, std::format("Script file does not exist: {}", scriptPath.string())));

			status = luaL_dofile(m_luaState, scriptPath.string().c_str());
			if (status)
				return std::unexpected(Core::make_error(Core::ErrorCode::ExecutionError, std::format("Lua error: {}", lua_tostring(m_luaState, -1))));

			return {};
		}

	};
}