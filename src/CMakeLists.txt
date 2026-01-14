# CMakeList.txt : CMake project for LearnOpenGL, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (LongRoadEng "main.cpp" "main.h" "stb_image.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET LongRoadEng PROPERTY CXX_STANDARD 20)
endif()

configure_file(${CMAKE_SOURCE_DIR}/src/3.3.shader.vs ${CMAKE_BINARY_DIR}/3.3.shader.vs COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/src/3.3.shader.fs ${CMAKE_BINARY_DIR}/3.3.shader.fs COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/assets/texture/container.jpg ${CMAKE_BINARY_DIR}/container.jpg COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/assets/texture/awesomeface.png ${CMAKE_BINARY_DIR}/awesomeface.png COPYONLY)

target_link_libraries(LongRoadEng PRIVATE 
    glfw 
    glad 
    OpenGL::GL
)
