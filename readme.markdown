lfov - Lua wrapper for libfov
=============================

[libfov](http://code.google.com/p/libfov/) is a C library for calculating fields of view and lighting on low resolution rasters, suitable for roguelike games, written by [Gregory McIntyre](mailto:blue.puyo@gmail.com).

lfov is a small wrapper to easily call it from Lua, written by Ross Andrews.

Examples:
---------

```lua
-- Calculate a circular field of view centered on (2,4)
settings = lfov.new()
settings:opaque(some_function)
settings:callback(some_callback)
settings:circle(2, 4)
```

You can also do directional beams:

```lua
settings = lfov.new()
settings:opaque(some_function)
settings:callback(some_callback)
settings:beam(2, 4, lfov.northeast, 90) -- facing NE, 90 degrees wide
```

Callbacks work like this:

```lua
-- Opaque callbacks take a point and return true iff you can't see through them
function opaque(x, y)
    return my_map:wall_at(x, y)
end

-- Lighting callbacks are called for each cell that's visible,
-- with the cell's coordinates and distance from the origin
function lit(x, y, x_delta, y_delta)
    table.insert(visible_cells, {x, y})
end
```

Features:
---------

* Just like libfov, it uses callbacks to inspect your map and mark cells, so it doesn't care how the map is stored
* Just like libfov, released under the MIT license so you can incorporate it into your game.

Todo:
-----

* Doesn't allow you to set the FOV shape.
* Currently, doesn't use the light source pointer for anything. Making this accessible from Lua would be good.

License:
--------

lfov (lfov.c and lfov.h) is released under the [MIT license](http://www.opensource.org/licenses/mit-license.php), copyright Ross Andrews 2012. libfov (fov.c and fov.h) is also released under the MIT license, but was written by Gregory McIntyre instead.

You can replace fov.c and fov.h with new versions from his site and it should still work.