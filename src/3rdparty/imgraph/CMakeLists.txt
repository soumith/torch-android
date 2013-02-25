CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
CMAKE_POLICY(VERSION 2.6)
FIND_PACKAGE(Torch REQUIRED)

add_subdirectory (pink)
add_subdirectory (graphcuts)

include_directories (
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/3rdparty/imgraph/pink
    ${PROJECT_SOURCE_DIR}/3rdparty/imgraph/graphcuts 
)

SET(src init.cpp)
SET(luasrc init.lua)

ADD_TORCH_PACKAGE(imgraph "${src}" "${luasrc}" "Image Processing")
TARGET_LINK_LIBRARIES(imgraph luaT TH graphcuts pink)
