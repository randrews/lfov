#include <stdlib.h>
#include "fov.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

int calc_fov(lua_State *L);

int luaopen_lfov(lua_State *lua){
    luaL_Reg lfov[] = {
        {"beam", calc_fov},
        {NULL, NULL}
    };

    luaL_openlib(lua, "lfov", lfov, 0);
    return 0;
}

bool opacity_test(void *Lv, int x, int y);
void lighting_callback(void *Lv, int x, int y, int dx, int dy, void *src);

int calc_fov(lua_State *L){
    if(!lua_isfunction(L, 1))
        return luaL_error(L, "Expected an opacity function for argument 1");
    if(!lua_isfunction(L, 2))
        return luaL_error(L, "Expected a visible callback for argument 2");

    int x = luaL_checkinteger(L, 3);
    int y = luaL_checkinteger(L, 4);
    int d = luaL_checkinteger(L, 5);

    if(d<0 || d>7)
        return luaL_error(L, "Direction must be 0..7, got %d", d);
    fov_direction_type dir = d;

    float angle = (lua_gettop(L) > 5 ? luaL_checknumber(L, 6) : 90.0);
    unsigned radius = (lua_gettop(L) > 6 ? luaL_checkinteger(L, 7) : 20);

    fov_settings_type *settings = malloc(sizeof(fov_settings_type));
    fov_settings_init(settings);
    fov_settings_set_opacity_test_function(settings, opacity_test);
    fov_settings_set_apply_lighting_function(settings, lighting_callback);
    
    fov_beam(settings, L, 0, x, y, radius, dir, angle);

    fov_settings_free(settings);
    free(settings);
    return 0;
}

bool opacity_test(void *Lv, int x, int y){
    lua_State *L = Lv;
    lua_pushvalue(L,1);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    if(lua_pcall(L, 2, 1, 0)) return 0;

    bool opaque = lua_toboolean(L, -1);
    lua_pop(L, 1);
    return opaque;
}

void lighting_callback(void *Lv, int x, int y, int dx, int dy, void *src){
    lua_State *L = Lv;
    lua_pushvalue(L, 2);
    lua_pushinteger(L, x);
    lua_pushinteger(L, y);
    lua_pushinteger(L, dx);
    lua_pushinteger(L, dy);
    lua_pcall(L,4,0,0);
}
