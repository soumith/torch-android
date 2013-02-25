# -*- cmake -*-

PROJECT(Lua)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
CMAKE_POLICY(VERSION 2.6)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

# If you want to include Lua in a main project,
# you might want to define those variables yourself
IF(NOT Lua_IS_SUBPROJECT)
  INCLUDE(LuaPaths)
ENDIF()

FIND_PACKAGE(Readline)

INCLUDE(CheckLibraryExists)
INCLUDE(CheckSymbolExists)
INCLUDE(CheckFunctionExists)

CHECK_FUNCTION_EXISTS(_longjmp LUA_USE_ULONGJMP)
CHECK_SYMBOL_EXISTS(sigaction signal.h LUA_USE_POSIX_SIGNAL)
CHECK_SYMBOL_EXISTS(isatty unistd.h LUA_USE_ISATTY)
CHECK_SYMBOL_EXISTS(mkstemp stdlib.h LUA_USE_MKSTEMP)
CHECK_SYMBOL_EXISTS(popen stdio.h LUA_USE_POPEN)
CHECK_LIBRARY_EXISTS(m sin "" LUA_USE_LIBM)

IF(READLINE_FOUND)
  SET(CMAKE_REQUIRED_INCLUDES ${READLINE_INCLUDE_DIR})
  SET(CMAKE_REQUIRED_LIBRARIES ${READLINE_LIBRARIES})
  CHECK_LIBRARY_EXISTS(${READLINE_readline_LIBRARY} readline "" LUA_USE_READLINE)
ENDIF(READLINE_FOUND)

FIND_LIBRARY(DL_LIBRARY "dl")
IF(DL_LIBRARY)
  SET(CMAKE_REQUIRED_LIBRARIES ${DL_LIBRARY})
ENDIF(DL_LIBRARY)
CHECK_FUNCTION_EXISTS(dlopen LUA_USE_DLOPEN)
IF(NOT WIN32 AND NOT LUA_USE_DLOPEN)
  MESSAGE(FATAL_ERROR "Cannot compile a useful lua.
Function dlopen() seems not to be supported on your platform.
Apparently you are not on a Windows platform as well.
So lua has no way to deal with shared libraries!")
ENDIF(NOT WIN32 AND NOT LUA_USE_DLOPEN)

IF(WIN32)
  SET(LUA_BUILD_AS_DLL 1)
ENDIF(WIN32)

IF (CMAKE_SHARED_LIBRARY_SUFFIX STREQUAL CMAKE_SHARED_MODULE_SUFFIX)
  SET(LUA_USE_MODULE_AND_LIBRARY 0)
ELSE (CMAKE_SHARED_LIBRARY_SUFFIX STREQUAL CMAKE_SHARED_MODULE_SUFFIX)
  SET(LUA_USE_MODULE_AND_LIBRARY 1)
ENDIF (CMAKE_SHARED_LIBRARY_SUFFIX STREQUAL CMAKE_SHARED_MODULE_SUFFIX)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
CONFIGURE_FILE(src/luaconf.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/luaconf.h)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/luaconf.h src/lua.h src/lauxlib.h src/lualib.h
  DESTINATION "${Lua_INSTALL_INCLUDE_SUBDIR}")

SET(_lua_lib_src
  src/lapi.c src/lcode.c src/ldebug.c src/ldo.c src/ldump.c src/lfunc.c src/lgc.c src/llex.c src/lmem.c
  src/lobject.c src/lopcodes.c src/lparser.c src/lstate.c src/lstring.c src/ltable.c src/ltm.c
  src/lundump.c src/lvm.c src/lzio.c
  src/lauxlib.c src/lbaselib.c src/ldblib.c src/liolib.c
  src/lmathlib.c src/loslib.c src/ltablib.c src/lstrlib.c src/loadlib.c src/linit.c  
  src/luaconf.c ${CMAKE_CURRENT_BINARY_DIR}/luaconf.h)

SET(_lua_src src/lua.c)
SET(_luac_src src/luac.c src/print.c)

# Compile library as C (android)
#SET_SOURCE_FILES_PROPERTIES(${_lua_lib_src} ${_lua_src} ${_luac_src} PROPERTIES
#  OBJECT_DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/luaconf.h"
#  LANGUAGE CXX)

# Compile static executables for dealing with compile time.
ADD_LIBRARY(liblua-static STATIC ${_lua_lib_src})
SET_TARGET_PROPERTIES(liblua-static PROPERTIES 
  COMPILE_DEFINITIONS "liblua_STATIC"
  OUTPUT_NAME "lua-static")

ADD_EXECUTABLE(lua-static ${_lua_src})
TARGET_LINK_LIBRARIES(lua-static liblua-static)
SET_TARGET_PROPERTIES(lua-static PROPERTIES 
  COMPILE_DEFINITIONS "liblua_STATIC"
  LINKER_LANGUAGE CXX)

ADD_EXECUTABLE(luac ${_luac_src})
TARGET_LINK_LIBRARIES(luac liblua-static)
SET_TARGET_PROPERTIES(luac PROPERTIES
  COMPILE_DEFINITIONS "liblua_STATIC"
  LINKER_LANGUAGE CXX)

# Specify necessary libraries
IF(LUA_USE_LIBM)
  TARGET_LINK_LIBRARIES(liblua-static m)
ENDIF(LUA_USE_LIBM)

IF(LUA_USE_READLINE)
  INCLUDE_DIRECTORIES(${READLINE_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(lua ${READLINE_LIBRARIES})
  TARGET_LINK_LIBRARIES(lua-static ${READLINE_LIBRARIES})
  SET(LUA_READLINE_INCLUDE ${READLINE_INCLUDE_DIR})
  SET(LUA_READLINE_LIBRARIES ${READLINE_LIBRARIES})
ELSE(LUA_USE_READLINE)
  SET(LUA_READLINE_INCLUDE)
  SET(LUA_READLINE_LIBRARIES)
ENDIF(LUA_USE_READLINE)

IF(DL_LIBRARY)
  TARGET_LINK_LIBRARIES(liblua-static ${DL_LIBRARY})
ENDIF(DL_LIBRARY)

# Create internal FindLua.cmake
SET(LUA_EXECUTABLE lua-static)
SET(LUAC_EXECUTABLE torch-luac)
SET(LUA_INCLUDE_DIR 
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_BINARY_DIR}
  ${LUA_READLINE_INCLUDE_DIR})
CONFIGURE_FILE(cmake/LuaConfig.cmake.in "${Lua_INSTALL_FINDLUA_DIR}/FindLua.cmake")
