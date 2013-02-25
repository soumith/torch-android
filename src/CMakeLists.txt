# -*- cmake -*-

#test for ANDROID and look for stuff not relevant to android.
if(NOT ANDROID)
#jni is available by default on android
find_package(JNI REQUIRED)
include_directories(${JNI_INCLUDE_DIRS})
endif()

set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)

SET( C_HAS_inline_EXITCODE	
     "PLEASE_FILL_OUT-FAILED_TO_RUN"
      CACHE STRING "Result from TRY_RUN" FORCE)

PROJECT(Torch)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
CMAKE_POLICY(VERSION 2.6)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake"
                      "${CMAKE_BINARY_DIR}/cmake" "${CMAKE_MODULE_PATH}")

INCLUDE(TorchPaths)
INCLUDE(TorchPathsInit)
INCLUDE(TorchCFlags)
INCLUDE(TorchPackage)
INCLUDE(TorchWrap)

# Torch executables
ADD_SUBDIRECTORY(exe)

# Torch libraries
ADD_SUBDIRECTORY(lib)

# Torch base packages
ADD_SUBDIRECTORY(pkg)

# Torch extra packages
ADD_SUBDIRECTORY(3rdparty)

# External packages support
INCLUDE(TorchExports)

# Packaging support
INCLUDE(TorchCPack)
