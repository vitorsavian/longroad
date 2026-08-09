add_executable(VulkanTutorial main.cpp)

target_link_libraries(VulkanTutorial PRIVATE LongRoadEng)

target_compile_features(VulkanTutorial PRIVATE cxx_std_23)