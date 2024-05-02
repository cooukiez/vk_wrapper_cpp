cmake_minimum_required(VERSION 3.8)

project(VCW)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++20")

set(IMGUI_DIR imgui)

file(GLOB MAIN_SOURCES *.cpp)
file(GLOB_RECURSE VK_SOURCES vk/*.cpp)
file(GLOB_RECURSE RENDER_SOURCES render/*.cpp)

set(SOURCES)
list(APPEND SOURCES ${MAIN_SOURCES} ${VK_SOURCES} ${RENDER_SOURCES})

add_executable(main ${SOURCES})

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)
find_package(Stb REQUIRED)

target_link_libraries(main Vulkan::Vulkan)
target_link_libraries(main glfw)
target_link_libraries(main glm::glm)

target_include_directories(main PRIVATE ${Stb_INCLUDE_DIR})

target_sources(main PRIVATE
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp)

target_include_directories(main PRIVATE ${IMGUI_DIR})

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message(STATUS "Detected MinGW compiler.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    message(STATUS "Detected MSVC compiler.")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Od")
endif()

add_custom_target(vert.spv
        COMMAND glslangValidator --quiet -V ${CMAKE_SOURCE_DIR}/shader.vert -o ${CMAKE_BINARY_DIR}/vert.spv)

add_custom_target(frag.spv
        COMMAND glslangValidator --quiet -V ${CMAKE_SOURCE_DIR}/shader.frag -o ${CMAKE_BINARY_DIR}/frag.spv)

add_dependencies(main vert.spv frag.spv)

file(COPY ${CMAKE_SOURCE_DIR}/textures DESTINATION ${CMAKE_BINARY_DIR})
