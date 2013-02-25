MACRO(ADD_TORCH_LUA2EXE lua_exe lua_file)

  GET_FILENAME_COMPONENT(file_name ${lua_file} NAME_WE)
  INCLUDE_DIRECTORIES(${Torch_SOURCE_INCLUDES})

  ADD_CUSTOM_COMMAND(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${file_name}.c"
    COMMAND "${Torch_SOURCE_LUA}"
    ARGS "${Torch_SOURCE_CMAKE}/lua2exe/lua2exe.lua" "${lua_file}" "${CMAKE_CURRENT_BINARY_DIR}/${file_name}.c"
    DEPENDS "${lua_file}" "${Torch_SOURCE_LUA}" "${Torch_SOURCE_CMAKE}/lua2exe/lua2exe.lua")

  IF(WITH_LUA_JIT AND APPLE)
    SET(CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000 ${CMAKE_EXE_LINKER_FLAGS}")
  ENDIF(WITH_LUA_JIT AND APPLE)
  
  ADD_EXECUTABLE("${lua_exe}" "${CMAKE_CURRENT_BINARY_DIR}/${file_name}.c")
  TARGET_LINK_LIBRARIES("${lua_exe}" liblua-shared)

ENDMACRO(ADD_TORCH_LUA2EXE lua_exe lua_file)
