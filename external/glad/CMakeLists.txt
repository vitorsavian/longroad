cmake_minimum_required (VERSION 3.8)

# Cria uma biblioteca estática com o GLAD
add_library(glad STATIC
    src/glad.c
)

# Adiciona o diretório de include para que o GLAD encontre seus headers
target_include_directories(glad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)