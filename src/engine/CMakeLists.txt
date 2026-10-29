add_library(LongRoadEng
    core/engine.cpp
)

target_include_directories(LongRoadEng PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Public headers include Vulkan-Hpp, so everything that affects how it is
# parsed has to reach consumers too -- otherwise the engine and the game see
# different definitions of the same Vulkan types.
if(ENABLE_CXX_MODULES)
    target_link_libraries(LongRoadEng PUBLIC glfw SDL3::SDL3 glm::glm Vulkan::cppm)
    target_compile_definitions(LongRoadEng PUBLIC USE_CXX_MODULES)
else()
    target_link_libraries(LongRoadEng PUBLIC glfw SDL3::SDL3 glm::glm Vulkan::Vulkan)
endif()

target_compile_features(LongRoadEng PUBLIC cxx_std_23)

target_compile_definitions(LongRoadEng PUBLIC VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1)

if(MSVC)
    target_compile_options(LongRoadEng PRIVATE /W4)
else()
    target_compile_options(LongRoadEng PRIVATE -Wall -Wextra -Wpedantic)
endif()
