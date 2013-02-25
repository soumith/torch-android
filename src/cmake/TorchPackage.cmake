# -*- cmake -*-

MACRO(ADD_TORCH_PACKAGE package src luasrc)

  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR} ${Torch_SOURCE_INCLUDES})

  ### C/C++ sources
  IF(src)      

    ADD_LIBRARY(${package} ${src})
    
    ### Torch packages supposes libraries prefix is "lib"
    SET_TARGET_PROPERTIES(${package} PROPERTIES
      PREFIX "lib"
      IMPORT_PREFIX "lib"
      INSTALL_NAME_DIR "@executable_path/${Torch_INSTALL_BIN2CPATH}")
        
  ENDIF(src)
      
ENDMACRO(ADD_TORCH_PACKAGE)
