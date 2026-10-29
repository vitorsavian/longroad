add_executable(VulkanTutorial main.cpp)

target_link_libraries(VulkanTutorial PRIVATE LongRoadEng)

if(MSVC)
    target_compile_options(VulkanTutorial PRIVATE /W4)
else()
    target_compile_options(VulkanTutorial PRIVATE -Wall -Wextra -Wpedantic)
endif()
