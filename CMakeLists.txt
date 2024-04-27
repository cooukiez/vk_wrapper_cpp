cmake_minimum_required (VERSION 3.8)

project(VCW)

file(GLOB MAIN_SOURCES *.cpp)
file(GLOB_RECURSE VK_SOURCES vk/*.cpp)
file(GLOB_RECURSE RENDER_SOURCES render/*.cpp)

set(SOURCES)
list(APPEND SOURCES ${MAIN_SOURCES} ${VK_SOURCES} ${RENDER_SOURCES})

add_executable(main ${SOURCES})

find_package(glfw3 REQUIRED)
target_link_libraries(main glfw)

find_package (Vulkan REQUIRED)
target_link_libraries(main Vulkan::Vulkan)

find_package(glm REQUIRED)
target_link_libraries(main glm::glm)

find_package(Stb REQUIRED)
target_include_directories(main PRIVATE ${Stb_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_custom_target(vert.spv
	COMMAND glslangValidator --quiet -V ${CMAKE_SOURCE_DIR}/shader.vert -o ${CMAKE_BINARY_DIR}/vert.spv)

add_custom_target(frag.spv
	COMMAND glslangValidator --quiet -V ${CMAKE_SOURCE_DIR}/shader.frag -o ${CMAKE_BINARY_DIR}/frag.spv)

add_dependencies(main vert.spv frag.spv)
