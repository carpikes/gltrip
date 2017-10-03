cmake_minimum_required (VERSION 3.5.1)
project(gltrip)

find_package( SDL REQUIRED)
find_package(OpenGL REQUIRED)

set( WINDOW_WIDTH 1024 CACHE STRING "Set window width" )
set( WINDOW_HEIGHT 768 CACHE STRING "Set window height" )

add_definitions( -DWIDTH=${WINDOW_WIDTH} -DHEIGHT=${WINDOW_HEIGHT} -DCMAKE_BUILD)
set(CMAKE_CXX_FLAGS "-wAll -Werror -pedantic")
add_executable(particles particles.c)
target_link_libraries(particles m pthread SDL GL)