cmake_minimum_required(VERSION 3.8)
project(FillRate)

find_library(LIBGLFW NAMES libglfw libglfw3 glfw glfw3 PATHS ${PROJECT_SOURCE_DIR})
find_library(LIBOPENGL NAMES GL opengl32)
find_library(LIBGLEW NAMES GLEW glew32 glew32s glew32sd PATHS ${PROJECT_SOURCE_DIR})
set(LINK_LIBS ${LIBGLFW} ${LIBOPENGL} ${LIBGLEW})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	add_definitions(-DGLEW_STATIC)
endif()

add_executable(FillRate_attachment0 FillRate_attachment0.cpp)
add_executable(FillRate_attachment0_masked FillRate_attachment0_masked.cpp)
add_executable(FillRate_attachment1 FillRate_attachment1.cpp)
add_executable(FillRate_attachment1_masked FillRate_attachment1_masked.cpp)
target_link_libraries(FillRate_attachment0 ${LINK_LIBS})
target_link_libraries(FillRate_attachment0_masked ${LINK_LIBS})
target_link_libraries(FillRate_attachment1 ${LINK_LIBS})
target_link_libraries(FillRate_attachment1_masked ${LINK_LIBS})
