-- Define a map as an array of strings
map = {
   "............",
   "............",
   "..####......",
   ".....#......",
   "............",
   ".....#......",
   ".....#......",
   "............",
   "............",
   "............"
}

-- List of visible spaces: this will get populated by lfov
visible = {}

-- Take a point, return whether it's opaque (a #) on the map or not
function opaque(x, y)
   if x<0 or y<0 or x>11 or y>9 then return true end

   return map[y+1]:sub(x+1,x+1) ~= '.'
end

-- Called for each visible space, including visible opaque ones
function lit(x, y)
   if x<0 or y<0 or x>11 or y>9 then return true end

   visible[x+y*12] = true
end

----------------------------------------

-- First require the library
require 'lfov'

-- Make an lfov settings
sets = lfov.new()

-- Set the two callbacks
sets:opacity(opaque)
sets:callback(lit)

-- This will have the player look northeast from (2,4) with a 180 deg FOV
-- sets:beam(2, 4, lfov.northeast, 180)

-- This will have the player at (3,4) looking in a circle (defaults to 20 radius)
sets:circle(3, 4)

-- Setting the callbacks actually worked?
assert(sets:opacity() == opaque)
assert(sets:callback() == lit)

-- The direction constants are there?
assert(lfov.east == 0);
assert(lfov.west == 4); -- etc...

-- Print out the map, printing spaces for anything not visible
for y = 0, 9 do
   local s = ''
   for x = 0, 11 do
      local c = map[y+1]:sub(x+1, x+1)
      if not visible[x+y*12] then c = ' ' end
      s = s .. c
   end
   print(s)
end