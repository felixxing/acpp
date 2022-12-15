cmake_minimum_required(VERSION 3.0.0)
project(acpp VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS off)

file(GLOB_RECURSE SRC_CPP src/*.cpp)
add_executable(acpp main.cpp ${SRC_CPP})
target_compile_definitions(acpp PUBLIC GLFW_INCLUDE_NONE)
target_compile_options(acpp PUBLIC -pg -no-pie)

# includes-cpp
target_include_directories(acpp PUBLIC include)

# libraries-cpp
target_link_libraries(acpp -pg -no-pie)
target_link_libraries(acpp glfw3)
target_link_libraries(acpp assimp)
target_link_libraries(acpp fmt)
target_link_libraries(acpp fltk)
