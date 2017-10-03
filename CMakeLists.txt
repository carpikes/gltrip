cmake_minimum_required (VERSION 3.5.1)
project(gltrip)

find_package( SDL REQUIRED)
find_package(OpenGL REQUIRED)

set(CMAKE_CXX_FLAGS "-wAll -Werror -pedantic")
add_executable(particles particles.c)
target_link_libraries(particles m pthread SDL GL)