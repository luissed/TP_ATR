cmake_minimum_required(VERSION 3.10)
project(TP_ATR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PahoMqttCpp REQUIRED)

include_directories(include)

# PROCESSO CAMINHÃO (com múltiplas tarefas/threads)
set(CAMINHAO_SOURCES
    src/processo_caminhao/main.cpp
    src/processo_caminhao/caminhao.cpp
    src/processo_caminhao/tratamento_sensores.cpp
    src/processo_caminhao/logica_comando.cpp
    src/processo_caminhao/monitoramento_falhas.cpp
    src/processo_caminhao/controle_navegacao.cpp
    src/processo_caminhao/coletor_dados.cpp
    src/processo_caminhao/interface_local.cpp
)

add_executable(processo_caminhao ${CAMINHAO_SOURCES})
target_link_libraries(processo_caminhao PahoMqttCpp::PahoMqttCpp pthread)

# PROCESSO GESTÃO MINA
add_executable(processo_gestao
    src/processo_gestao/main.cpp
    src/processo_gestao/gestao_mina.cpp
)
target_link_libraries(processo_gestao PahoMqttCpp::PahoMqttCpp)

# PROCESSO SIMULAÇÃO MINA
add_executable(processo_simulacao
    src/processo_simulacao/main.cpp
    src/processo_simulacao/simulacao_mina.cpp
)
target_link_libraries(processo_simulacao PahoMqttCpp::PahoMqttCpp)