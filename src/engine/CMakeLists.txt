add_library(LongRoadEng
    core/engine.cpp
)

target_include_directories(LongRoadEng PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(ENABLE_CPP19_MODULES)
    target_link_libraries(LongRoadEng PUBLIC glfw SDL3::SDL3 glm::glm Vulkan::cppm)
    target_compile_definitions(LongRoadEng PRIVATE USE_CPP19_MODULES)
else()
    target_link_libraries(LongRoadEng PUBLIC glfw SDL3::SDL3 glm::glm Vulkan::Vulkan)
endif()

target_compile_features(LongRoadEng PRIVATE cxx_std_23)

target_compile_definitions(LongRoadEng PRIVATE VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1)
