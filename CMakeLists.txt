cmake_minimum_required(VERSION 3.6)
project(cube)

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})

find_package(glfw3 REQUIRED)
include_directories(${GLFW_INCLUDE_DIRS})
link_libraries(${GLFW_LIBRARY_DIRS})

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

add_subdirectory(external)
include_directories(
	external/glm-0.9.7.1/
	.
)
add_subdirectory(imgui)

add_executable(
	cube
	src/main.cpp
	src/shader.cpp
	src/shader.hpp
	src/vertex.shader
	src/fragment.shader
)

target_link_libraries(
	cube
	${GLEW_LIBRARIES}
	glfw
	imgui
	${OPENGL_LIBRARIES}
)
