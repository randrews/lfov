/*
Copyright (c) 2012 Ross Andrews

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "lfov.h"

/* Userdata boilerplate */
int newSettings(lua_State *L);
SettingsWrapper* checkSettings(lua_State *L, int i);
int settingsToString(lua_State *L);
int gcSettings(lua_State *L);

/* Accessors */
int setOpacity(lua_State *L);
int setCallback(lua_State *L);

/* Methods */
int lfov_beam(lua_State *L);
int lfov_circle(lua_State *L);

/* libfov callbacks */
bool opacity_test(void *Lv, int x, int y);
void lighting_callback(void *Lv, int x, int y, int dx, int dy, void *src);

/****************************************/

int luaopen_lfov(lua_State *lua){
    luaL_Reg lfov[] = {
        {"new", newSettings},
        {NULL, NULL}
    };

    luaL_openlib(lua, "lfov", lfov, 0);

    luaL_Reg SettingsMethods[] = {
        {"__tostring", settingsToString},
        {"__gc", gcSettings},
        {"opacity", setOpacity},
        {"callback", setCallback},
        {"beam", lfov_beam},
        {"circle", lfov_circle},
        {NULL, NULL}
    };

    luaL_newmetatable(lua, "lfov.settings");
    lua_pushstring(lua, "__index");
    lua_pushvalue(lua, -2);
    lua_settable(lua, -3);
    luaL_openlib(lua, NULL, SettingsMethods, 0);

    /* Set constants for all the directions */
    lua_getglobal(lua, "lfov");
    int tbl = lua_gettop(lua);

    lua_pushstring(lua, "east");
    lua_pushinteger(lua, 0);
    lua_settable(lua, tbl);

    lua_pushstring(lua, "northeast");
    lua_pushinteger(lua, 1);
    lua_settable(lua, tbl);

    lua_pushstring(lua, "north");
    lua_pushinteger(lua, 2);
    lua_settable(lua, tbl);

    lua_pushstring(lua, "northwest");
    lua_pushinteger(lua, 3);
    lua_settable(lua, tbl);

    lua_pushstring(lua, "west");
    lua_pushinteger(lua, 4);
    lua_settable(lua, tbl);

    lua_pushstring(lua, "southwest");
    lua_pushinteger(lua, 5);
    lua_settable(lua, tbl);

    lua_pushstring(lua, "south");
    lua_pushinteger(lua, 6);
    lua_settable(lua, tbl);

    lua_pushstring(lua, "southeast");
    lua_pushinteger(lua, 7);
    lua_settable(lua, tbl);

    return 0;
}

/****************************************/

int newSettings(lua_State *L){
    SettingsWrapper *sw = lua_newuserdata(L, sizeof(SettingsWrapper));
    luaL_getmetatable(L, "lfov.settings");
    lua_setmetatable(L, -2);
    sw->L = L;
    sw->settings = malloc(sizeof(fov_settings_type));
    fov_settings_init(sw->settings);
    fov_settings_set_opacity_test_function(sw->settings, opacity_test);
    fov_settings_set_apply_lighting_function(sw->settings, lighting_callback);
    sw->opacity = 0;
    sw->callback = 0;
    return 1;
}

SettingsWrapper* checkSettings(lua_State *L, int n){
    void *ud = luaL_checkudata(L, n, "lfov.settings");
    luaL_argcheck(L, ud != NULL, n, "`lfov.settings' expected");
    return (SettingsWrapper*)ud;
}

int settingsToString(lua_State *L){
    SettingsWrapper *sw = checkSettings(L, 1);
    char *s = malloc(64);
    sprintf(s, "<lfov.settings 0x%lx>", (unsigned long)(sw->settings));
    lua_pushstring(L, s);
    free(s);
    return 1;
}

int gcSettings(lua_State *L){
    SettingsWrapper *sw = checkSettings(L, 1);
    fov_settings_free(sw->settings);
    free(sw->settings);

    if(sw->opacity){
        lua_pushinteger(L, sw->opacity);
        lua_pushnil(L);
        lua_settable(L, LUA_REGISTRYINDEX);
    }

    if(sw->callback){
        lua_pushinteger(L, sw->callback);
        lua_pushnil(L);
        lua_settable(L, LUA_REGISTRYINDEX);
    }

    return 0;
}

int setOpacity(lua_State *L){
    SettingsWrapper *sw = checkSettings(L, 1);

    if(lua_gettop(L) > 1){
        if(!lua_isfunction(L, 2)) luaL_typerror(L, 2, "function");
        int fn = luaL_ref(L, LUA_REGISTRYINDEX);
        sw->opacity = fn;
    }

    if(sw->opacity){
        lua_pushinteger(L, sw->opacity);
        lua_gettable(L, LUA_REGISTRYINDEX);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

int setCallback(lua_State *L){
    SettingsWrapper *sw = checkSettings(L, 1);

    if(lua_gettop(L) > 1){
        if(!lua_isfunction(L, 2)) luaL_typerror(L, 2, "function");
        int fn = luaL_ref(L, LUA_REGISTRYINDEX);
        sw->callback = fn;
    }

    if(sw->callback){
        lua_pushinteger(L, sw->callback);
        lua_gettable(L, LUA_REGISTRYINDEX);
    } else {
        lua_pushnil(L);
    }
    return 1;
}

/****************************************/

int lfov_beam(lua_State *L){
    SettingsWrapper *sw = checkSettings(L, 1);

    if(!sw->opacity)
        return luaL_error(L, "No opacity test function set");
    if(!sw->callback)
        return luaL_error(L, "No lighting callback function set");

    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    int d = luaL_checkinteger(L, 4);

    if(d<0 || d>7)
        return luaL_error(L, "Direction must be 0..7, got %d", d);
    fov_direction_type dir = d;

    float angle = (lua_gettop(L) > 4 ? luaL_checknumber(L, 5) : 90.0);
    unsigned radius = (lua_gettop(L) > 5 ? luaL_checkinteger(L, 6) : 20);
    
    fov_beam(sw->settings, sw, 0, x, y, radius, dir, angle);

    lua_pushboolean(L, 1);
    return 1;
}

int lfov_circle(lua_State *L){
    SettingsWrapper *sw = checkSettings(L, 1);

    if(!sw->opacity)
        return luaL_error(L, "No opacity test function set");
    if(!sw->callback)
        return luaL_error(L, "No lighting callback function set");

    int x = luaL_checkinteger(L, 2);
    int y = luaL_checkinteger(L, 3);
    unsigned radius = (lua_gettop(L) > 3 ? luaL_checkinteger(L, 4) : 20);

    fov_circle(sw->settings, sw, 0, x, y, radius);

    lua_pushboolean(L, 1);
    return 1;
}

bool opacity_test(void *swv, int x, int y){
    SettingsWrapper *sw = swv;
    lua_State *L = sw->L;

    lua_pushinteger(L,sw->opacity);
    lua_gettable(L, LUA_REGISTRYINDEX);

    lua_pushinteger(L, x);
    lua_pushinteger(L, y);

    if(lua_pcall(L, 2, 1, 0))
        return luaL_error(L, "Error calling opacity function for (%d, %d)", x, y);

    bool opaque = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return opaque;
}

void lighting_callback(void *swv, int x, int y, int dx, int dy, void *src){
    SettingsWrapper *sw = swv;
    lua_State *L = sw->L;

    lua_pushinteger(L, sw->callback);
    lua_gettable(L, LUA_REGISTRYINDEX);
    
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    lua_pushinteger(L, dx);
    lua_pushinteger(L, dy);

    lua_pcall(L,4,0,0); /* Don't particularly care if it fails */
}
