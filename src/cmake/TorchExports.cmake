INSTALL(EXPORT torch-exports
  DESTINATION "${Torch_INSTALL_CMAKE_SUBDIR}"
  FILE "TorchExports.cmake")

CONFIGURE_FILE("cmake/TorchConfig.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/cmake-exports/TorchConfig.cmake" @ONLY)
INSTALL(
  FILES
  "${CMAKE_CURRENT_BINARY_DIR}/cmake-exports/TorchConfig.cmake"
  "cmake/TorchPathsInit.cmake"
  "cmake/TorchCFlags.cmake"
  "cmake/TorchPackage.cmake"
  "cmake/TorchWrap.cmake"
  "cmake/TorchDOK.cmake"
  "cmake/TorchLua2exe.cmake"
  "cmake/FindARM.cmake"
  DESTINATION "${Torch_INSTALL_CMAKE_SUBDIR}")

INSTALL(
  DIRECTORY
  "cmake/dok"
  "cmake/lua2exe"
  DESTINATION "${Torch_INSTALL_CMAKE_SUBDIR}")
