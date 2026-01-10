cmake_minimum_required (VERSION 4.2)

# Cria uma biblioteca est�tica com o GLAD
add_library(glad STATIC
    src/glad.c
)

# Adiciona o diret�rio de include para que o GLAD encontre seus headers
target_include_directories(glad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)