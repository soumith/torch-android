# We include all Torch libraries
FILE(GLOB _all_library_files "${Torch_SOURCE_DIR}/libraries/*")
FOREACH(_library_file ${_all_library_files})
  IF(EXISTS "${_library_file}/CMakeLists.txt")
    INCLUDE_DIRECTORIES(${_library_file})
  ENDIF(EXISTS "${_library_file}/CMakeLists.txt")
ENDFOREACH(_library_file ${_all_library_files})

# We compile all existing packages
FILE(GLOB _all_files *)
FOREACH(_file ${_all_files})
  IF(EXISTS "${_file}/CMakeLists.txt")
    ADD_SUBDIRECTORY(${_file})
  ENDIF(EXISTS "${_file}/CMakeLists.txt")
ENDFOREACH(_file ${_all_files})
