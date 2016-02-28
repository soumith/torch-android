CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
CMAKE_POLICY(VERSION 2.6)

MESSAGE(STATUS "Including android directory: ${ANDROID_NDK_SYSROOT_INCLUDE}")
INCLUDE_DIRECTORIES(${ANDROID_NDK_SYSROOT_INCLUDE})
INCLUDE_DIRECTORIES(${LuaJIT_SOURCE_DIR}/src ${LuaJIT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/distro/pkg/torch/lib/TH ${CMAKE_BINARY_DIR}/distro/pkg/torch/lib/TH)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/distro/pkg/torch/lib ${CMAKE_BINARY_DIR}/distro/pkg/torch/lib)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/distro/pkg/torch/lib/luaT)
LINK_DIRECTORIES(${ANDROID_NDK_SYSROOT_LIBDIR})

SET(src init.c torchandroid.cpp torchcall.cpp android_fopen.c)
SET(luasrc init.lua)

ADD_LIBRARY(torchandroid MODULE ${src})

SET_TARGET_PROPERTIES(torchandroid PROPERTIES
      PREFIX "lib"
      IMPORT_PREFIX "lib")

INSTALL(TARGETS torchandroid
      RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
      LIBRARY DESTINATION  ${CMAKE_INSTALL_PREFIX}/lib)

# ADD_TORCH_PACKAGE(torchandroid "${src}" "${luasrc}" "Torch Android plugins")
TARGET_LINK_LIBRARIES(torchandroid luaT TH android log)
IF(LUALIB)
  TARGET_LINK_LIBRARIES(torchandroid ${LUALIB})
ENDIF()

INSTALL(FILES
	init.lua
        DESTINATION "${CMAKE_INSTALL_PREFIX}/share/lua/5.1/torchandroid"
)

INSTALL(FILES
  THApkFile.h
  torchandroid.h
  android_fopen.h
  DESTINATION "${CMAKE_INSTALL_PREFIX}/include"
)
