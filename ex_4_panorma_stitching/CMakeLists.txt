cmake_minimum_required(VERSION 2.8)
project( panst )
find_package( OpenCV REQUIRED )
add_executable( panst
    src/homographies.cpp
    src/local_maxima.cpp
    src/matching.cpp
    src/ps_main.cpp
    src/render.cpp
    src/saves.cpp
)
target_link_libraries( panst ${OpenCV_LIBS} )
set(CMAKE_CXX_FLAGS "-std=c++0x")