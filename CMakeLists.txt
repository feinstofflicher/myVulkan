cmake_minimum_required(VERSION 3.7)

project(myVulkan)

set(SDL2_DIR externals/SDL2-2.0.6)
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

include_directories(externals/stb)

set(VULKAN_SOURCES
    src/vulkan/basicrenderer.h
    src/vulkan/basicrenderer.cpp
    src/vulkan/swapchain.h
    src/vulkan/swapchain.cpp
    src/vulkan/vulkanhelper.h
    src/vulkan/vulkanhelper.cpp
    src/vulkan/debug.h
    src/vulkan/debug.cpp
    src/vulkan/device.h
    src/vulkan/device.cpp
    src/vulkan/shader.h
    src/vulkan/shader.cpp
    src/vulkan/pipeline.h
    src/vulkan/pipeline.cpp
    src/vulkan/renderpass.h
    src/vulkan/renderpass.cpp
    src/vulkan/framebuffer.h
    src/vulkan/framebuffer.cpp
    src/vulkan/vertexbuffer.h
    src/vulkan/vertexbuffer.cpp
    src/vulkan/texture.h
    src/vulkan/texture.cpp
)

set(SOURCES
    src/main.cpp
    src/simplerenderer.h
    src/simplerenderer.cpp
)

set(RESOURCE_DIR data)
set(TEXTURE_DIR ${RESOURCE_DIR}/textures)
set(SHADER_DIR ${RESOURCE_DIR}/shaders)
file(GLOB SHADERS "${SHADER_DIR}/*.vert" "${SHADER_DIR}/*.frag")
source_group("shaders" FILES ${SHADERS})
source_group("source" FILES ${SOURCES})
source_group("vulkan" FILES ${VULKAN_SOURCES})

add_executable(${PROJECT_NAME}
    ${VULKAN_SOURCES}
    ${SOURCES}
    ${SHADERS}
)

target_link_libraries(${PROJECT_NAME}
    ${SDL2_LIBRARIES}
    ${Vulkan_LIBRARY}
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE "/MP")
endif()

ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} ARGS -E copy_if_different ${SDL2_RUNTIME_LIBRARIES} ${CMAKE_CURRENT_BINARY_DIR}
)

file(COPY ${TEXTURE_DIR}
     DESTINATION ${CMAKE_BINARY_DIR}/${RESOURCE_DIR})

find_program(GLSLANGVALIDATOR glslangValidator)

set(SHADER_OUTPUT_DIR ${CMAKE_BINARY_DIR}/${SHADER_DIR}/)
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})
foreach(SHADER ${SHADERS})
    get_filename_component(filename ${SHADER} NAME)
    add_custom_command(
        OUTPUT ${SHADER}
	    COMMAND ${GLSLANGVALIDATOR} -V ${SHADER} -o ${SHADER_OUTPUT_DIR}${filename}.spv
        COMMENT "Rebuilding ${SHADER}.spv"
    )
endforeach(SHADER)