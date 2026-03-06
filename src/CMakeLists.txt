# CMakeList.txt : CMake project for LearnOpenGL, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (LongRoadEng "main.cpp" "main.h" "stb_image.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET LongRoadEng PROPERTY CXX_STANDARD 20)
endif()

# Define PROJECT_ROOT for asset pathsa
target_compile_definitions(LongRoadEng PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}")

target_link_libraries(LongRoadEng PRIVATE 
    glfw 
    glad 
    OpenGL::GL
)
