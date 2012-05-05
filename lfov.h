#include <stdlib.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "fov.h"

typedef struct SettingsWrapper {
    fov_settings_type *settings;
    lua_State *L;
    int opacity;
    int callback;
} SettingsWrapper;

int luaopen_lfov(lua_State *lua);
