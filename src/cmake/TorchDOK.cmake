# Workaround: CMake sux if we do not create the directories
# This is completely incoherent compared to INSTALL(FILES ...)
FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/dok")
FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/dokmedia")
FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/html")

INSTALL(DIRECTORY "${CMAKE_BINARY_DIR}/dok/" DESTINATION "${Torch_INSTALL_DOK_SUBDIR}")
INSTALL(DIRECTORY "${CMAKE_BINARY_DIR}/dokmedia/" DESTINATION "${Torch_INSTALL_DOKMEDIA_SUBDIR}")
INSTALL(DIRECTORY "${CMAKE_BINARY_DIR}/html/" DESTINATION "${Torch_INSTALL_HTML_SUBDIR}")

# Files for HTML creation
SET(TORCH_DOK_HTML_TEMPLATE "${Torch_SOURCE_CMAKE}/dok/doktemplate.html"
  CACHE FILEPATH "List of files needed for HTML doc creation")

SET(TORCH_DOK_HTML_FILES "${Torch_SOURCE_CMAKE}/dok/doctorch.css;${Torch_SOURCE_CMAKE}/dok/torchlogo.png;${Torch_SOURCE_CMAKE}/dok/jquery.js;${Torch_SOURCE_CMAKE}/dok/shCore.js;${Torch_SOURCE_CMAKE}/dok/shBrushLua.js;${Torch_SOURCE_CMAKE}/dok/shBrushCpp.js;${Torch_SOURCE_CMAKE}/dok/shBrushBash.js;${Torch_SOURCE_CMAKE}/dok/shCore.css;${Torch_SOURCE_CMAKE}/dok/jse_form.js"
  CACHE STRING "HTML template needed for HTML doc creation")

MARK_AS_ADVANCED(TORCH_DOK_HTML_FILES TORCH_DOK_HTML_TEMPLATE)

ADD_CUSTOM_TARGET(documentation-dok
  ALL
  COMMENT "Built documentation")

# copy extra files needed for HTML doc (main index)
IF(PROJECT_NAME STREQUAL "Torch")
  SET(htmldstdir "${CMAKE_BINARY_DIR}/html")
  SET(generatedfiles)

  FOREACH(extrafile ${TORCH_DOK_HTML_FILES})
    GET_FILENAME_COMPONENT(_file_ "${extrafile}" NAME)
    ADD_CUSTOM_COMMAND(OUTPUT "${htmldstdir}/${_file_}"
      COMMAND ${CMAKE_COMMAND} ARGS "-E" "copy" "${extrafile}" "${htmldstdir}/${_file_}"
      DEPENDS "${extrafile}")
    SET(generatedfiles ${generatedfiles} "${htmldstdir}/${_file_}")
  ENDFOREACH(extrafile ${TORCH_DOK_HTML_FILES})

  # the doc depends on all these files to be generated
  ADD_CUSTOM_TARGET(main-index-dok-files
    DEPENDS ${generatedfiles})
  ADD_DEPENDENCIES(documentation-dok main-index-dok-files)
ENDIF(PROJECT_NAME STREQUAL "Torch")

# used to make sure dokindex is built in a serial way
SET(all-dok-index "" CACHE INTERNAL "dokindex previous dependencies" FORCE)

MACRO(ADD_TORCH_DOK srcdir dstdir section title rank)

  SET(dokdstdir "${CMAKE_BINARY_DIR}/dok/${dstdir}")
  SET(dokmediadstdir "${CMAKE_BINARY_DIR}/dokmedia/${dstdir}")
  SET(htmldstdir "${CMAKE_BINARY_DIR}/html/${dstdir}")

  FILE(MAKE_DIRECTORY "${dokdstdir}")
  FILE(MAKE_DIRECTORY "${dokmediadstdir}")
  FILE(MAKE_DIRECTORY "${htmldstdir}")

  # Note: subdirectories are not handled (yet?)
  # http://www.cmake.org/pipermail/cmake/2008-February/020114.html
  FILE(GLOB dokfiles "${srcdir}/*")

  SET(generatedfiles)
  FOREACH(dokfile ${dokfiles})
    GET_FILENAME_COMPONENT(_ext_ "${dokfile}" EXT)
    GET_FILENAME_COMPONENT(_file_ "${dokfile}" NAME_WE)

    # we move the doc files together (in the same dok/ directory)
    # we also convert the .dok (meaningful) to .txt (meaningless)
    # such that dokuwiki understands it.
    IF(_ext_ STREQUAL ".dok")
      ADD_CUSTOM_COMMAND(OUTPUT "${dokdstdir}/${_file_}.txt" "${htmldstdir}/${_file_}.html"
        COMMAND  ${Torch_SOURCE_LUA} ARGS "${Torch_SOURCE_CMAKE}/dok/dokparse.lua" "${Torch_SOURCE_PKG}/dok/init.lua" "${TORCH_DOK_HTML_TEMPLATE}" "${dokfile}" "${title}" "${CMAKE_BINARY_DIR}/html" "${dokdstdir}/${_file_}.txt" "${htmldstdir}/${_file_}.html"
        DEPENDS ${Torch_SOURCE_LUA}
        "${Torch_SOURCE_PKG}/dok/init.lua"
        "${Torch_SOURCE_CMAKE}/dok/dokparse.lua"
        "${dokfile}"
        "${TORCH_DOK_HTML_TEMPLATE}")
      
      SET(generatedfiles ${generatedfiles} "${dokdstdir}/${_file_}.txt" "${htmldstdir}/${_file_}.html")
    ELSE(_ext_ STREQUAL ".dok")
      ADD_CUSTOM_COMMAND(OUTPUT "${dokmediadstdir}/${_file_}${_ext_}" "${htmldstdir}/${_file_}${_ext_}"
        COMMAND ${CMAKE_COMMAND} ARGS "-E" "copy" "${dokfile}" "${htmldstdir}/${_file_}${_ext_}"
        COMMAND ${CMAKE_COMMAND} ARGS "-E" "copy" "${dokfile}" "${dokmediadstdir}/${_file_}${_ext_}"
        DEPENDS "${dokfile}")
      SET(generatedfiles ${generatedfiles} "${dokmediadstdir}/${_file_}${_ext_}" "${htmldstdir}/${_file_}${_ext_}")
    ENDIF(_ext_ STREQUAL ".dok")
  ENDFOREACH(dokfile ${dokfiles})

  # copy extra files needed for HTML doc
  FOREACH(extrafile ${TORCH_DOK_HTML_FILES}) 
    GET_FILENAME_COMPONENT(_file_ "${extrafile}" NAME)
   
    ADD_CUSTOM_COMMAND(OUTPUT "${htmldstdir}/${_file_}"
      COMMAND ${CMAKE_COMMAND} ARGS "-E" "copy" "${extrafile}" "${htmldstdir}/${_file_}"
      DEPENDS "${extrafile}")
    SET(generatedfiles ${generatedfiles} "${htmldstdir}/${_file_}")
  ENDFOREACH(extrafile ${TORCH_DOK_HTML_FILES}) 

  # the doc depends on all these files to be generated
  ADD_CUSTOM_TARGET(${dstdir}-dok-files
    DEPENDS ${generatedfiles})
  ADD_DEPENDENCIES(documentation-dok ${dstdir}-dok-files)

  # Build the dok index if the package contains an index.dok file
  IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${srcdir}/index.dok")    

    # this target is always built
    # so the index is always up-to-date (even if the installed index changed)
    ADD_CUSTOM_TARGET(${dstdir}-dok-index
      ${Torch_SOURCE_LUA} "${Torch_SOURCE_CMAKE}/dok/dokindex.lua" "${Torch_SOURCE_PKG}/dok/init.lua" "${TORCH_DOK_HTML_TEMPLATE}" "${CMAKE_BINARY_DIR}/dokindex.lua" "${Torch_INSTALL_SHARE}/torch/dokindex.lua" "${CMAKE_BINARY_DIR}/dok/index.txt" "${CMAKE_BINARY_DIR}/html/index.html" "${dstdir}" "${section}" "${title}" "${rank}"
      DEPENDS ${Torch_SOURCE_LUA}
      "${Torch_SOURCE_CMAKE}/dok/dokindex.lua"
      "${CMAKE_CURRENT_SOURCE_DIR}/${srcdir}/index.dok"
      "${Torch_SOURCE_PKG}/dok/init.lua")
    
    # the main dok depends on this
    ADD_DEPENDENCIES(documentation-dok ${dstdir}-dok-index)

    # this one depends on all previous ones (serial build)
    FOREACH(target ${all-dok-index})
      ADD_DEPENDENCIES(${dstdir}-dok-index ${target})
    ENDFOREACH(target ${all-dok-index})

    SET(all-dok-index "${dstdir}-dok-index" ${all-dok-index} CACHE INTERNAL "dokindex previous dependencies" FORCE)

    # install (only once) the dokindex.lua for future reference
    LIST(LENGTH all-dok-index all-dok-index-length)
    IF(all-dok-index-length EQUAL 1)
      INSTALL(FILES "${CMAKE_BINARY_DIR}/dokindex.lua" DESTINATION "${Torch_INSTALL_SHARE_SUBDIR}/torch")
    ENDIF(all-dok-index-length EQUAL 1)

  ENDIF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${srcdir}/index.dok")

ENDMACRO(ADD_TORCH_DOK)

INSTALL(CODE "EXECUTE_PROCESS(COMMAND ${CMAKE_INSTALL_PREFIX}/bin/torch-lua -ltorch -ldok -e \"dok.installsearch()\")")
