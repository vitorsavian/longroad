# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 4.3)

project(LongRoad LANGUAGES C CXX)

option(ENABLE_CXX_MODULES    "Consume Vulkan-Hpp as a C++20 module" OFF)
option(BUILD_VULKAN_TUTORIAL "Build the VulkanTutorial sample"      ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# Enable Hot Reload for MSVC compilers.
set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")

find_package(Vulkan REQUIRED)

if(ENABLE_CXX_MODULES)
    message(STATUS "Building Vulkan C++ Module support")
    add_library(VulkanCppModule)
    add_library(Vulkan::cppm ALIAS VulkanCppModule)

    target_compile_definitions(VulkanCppModule PUBLIC
            VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
            VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
    )

    target_include_directories(VulkanCppModule PRIVATE "${Vulkan_INCLUDE_DIR}")

    target_link_libraries(VulkanCppModule PUBLIC Vulkan::Vulkan)

    # Must match the standard of everything importing the module.
    target_compile_features(VulkanCppModule PUBLIC cxx_std_23)

    target_sources(VulkanCppModule
            PUBLIC
            FILE_SET cxx_modules TYPE CXX_MODULES
            BASE_DIRS "${Vulkan_INCLUDE_DIR}"
            FILES "${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm"
    )
else()
    message(STATUS "Vulkan C++ Module support DISABLED")
endif()

# Build GLFW from vendored source
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

# Build SDL3 from vendored source, static like GLFW
set(SDL_SHARED       OFF CACHE BOOL "" FORCE)
set(SDL_STATIC       ON  CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS        OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES     OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL      OFF CACHE BOOL "" FORCE)
add_subdirectory(external/SDL)

add_subdirectory(external/glm)

# Include sub-projects.
add_subdirectory(src)
