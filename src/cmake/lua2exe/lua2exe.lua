local luafile = arg[1]
local cfile = arg[2]

assert(luafile, 'no Lua file provided')
assert(cfile, 'no C file provided')

local code = [[
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "luaT.h"

static const unsigned char luacode[] = {
LUA_CODE
};

static long luacode_size = LUA_CODE_SIZE;

int main(int argc, char *argv[])
{
  lua_State *L;
  int i;

  lua_executable_dir(argv[0]);
  L = luaL_newstate();
  
  if(L == NULL)
  {
    printf("error: not enough memory for Lua state\n");
    return -1;
  }
  
  lua_gc(L, LUA_GCSTOP, 0);
  luaL_openlibs(L);
  lua_gc(L, LUA_GCRESTART, 0);
  
  lua_createtable(L, argc-1, 1);
  for(i = 0; i < argc; i++)
  {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i);
  }
  lua_setglobal(L, "arg");

  if(luaL_loadbuffer(L, (const char*)luacode, luacode_size, NULL))
  {
    const char *msg = lua_tostring(L, -1);
    printf("error: %s\n", msg);
    return -1;
  }

  if(lua_pcall(L, 0, 0, 0))
  {
    const char *msg = lua_tostring(L, -1);
    printf("error: %s\n", msg);
    return -1;
  }
    
  lua_close(L);
  return 0;
}
]]

local f = io.open(luafile)
if not f then
   error('could not open Lua file (read mode)')
end
local str = f:read('*all')
f:close()

function str2hex(str)
   local n = 0
   local hexlines = {}
   local hexline = {}
   str:gsub('.', function(char)
                    n = n + 1
                    table.insert(hexline, string.format('0x%0.2x', string.byte(char)))
                    if #hexline == 12 then
                       table.insert(hexlines, table.concat(hexline, ', '))
                       hexline = {}
                    end
                 end)
   
   if #hexline > 0 then
      table.insert(hexlines, table.concat(hexline, ', '))
   end
   return table.concat(hexlines, ',\n'), n
end

local f = io.open(cfile, 'w')
if not f then
   error('could not open C file (write mode)')
end
local lua_code, lua_code_size = str2hex(str)
code = code:gsub('LUA_CODE_SIZE', lua_code_size):gsub('LUA_CODE', lua_code)
f:write(code)
f:close()
