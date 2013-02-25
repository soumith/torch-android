include_directories (${PROJECT_SOURCE_DIR}/3rdparty/imgraph/graphcuts)

add_library(graphcuts maxflow.cpp)

set (CMAKE_CXX_FLAGS "-fPIC -g -DUNIXIO")
target_link_libraries(graphcuts)
