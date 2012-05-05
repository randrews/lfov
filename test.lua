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

visible = {}

function opaque(x, y)
   if x<0 or y<0 or x>11 or y>9 then return true end

   return map[y+1]:sub(x+1,x+1) ~= '.'
end

function lit(x, y)
   if x<0 or y<0 or x>11 or y>9 then return true end

   visible[x+y*12] = true
end

require 'lfov'

lfov.beam(opaque, lit, 2, 4, 0, 180)

for y = 0, 9 do
   local s = ''
   for x = 0, 11 do
      local c = map[y+1]:sub(x+1, x+1)
      if not visible[x+y*12] then c = ' ' end
      s = s .. c
   end
   print(s)
end