cmake_minimum_required(VERSION 3.10)
project(TP_ATR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(include)

set(SOURCE_FILES
    src/main.cpp
    src/caminhao/caminhao.cpp
    src/caminhao/tratamento_sensores.cpp
    src/caminhao/logica_comando.cpp
    src/caminhao/monitoramento_falhas.cpp
    src/caminhao/controle_navegacao.cpp
    src/caminhao/coletor_dados.cpp
    src/caminhao/interface_local.cpp
)

add_executable(caminhoes_autonomos ${SOURCE_FILES})

if(UNIX AND NOT APPLE)
    target_link_libraries(caminhoes_autonomos pthread)
endif()

message(STATUS "Projeto configurado com sucesso! Use 'make' para compilar.")