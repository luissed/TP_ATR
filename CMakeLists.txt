cmake_minimum_required(VERSION 3.10)
project(TP_ATR_Mina)

# Define padrão C++ 17 (necessário para paho e recursos modernos)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Habilita mensagens de debug se necessário
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- 1. CONFIGURAÇÃO DE DIRETÓRIOS ---
# Onde o compilador vai procurar os arquivos .hpp
include_directories(include)

# --- 2. ENCONTRAR PACOTES ---
# Encontra biblioteca de Threads (necessária para o Paho e para o seu código)
find_package(Threads REQUIRED)

# Encontra SFML (para a GUI) - Se não tiver instalado a SFML, comente essa linha
find_package(SFML 2.5 COMPONENTS graphics window system)

# --- 3. DEFINIR FONTES COMUNS ---
# Aqui listamos APENAS as classes. NÃO coloque arquivos com função main() aqui.
set(COMMON_SOURCES
    src/BufferCircular.cpp
    src/Caminhao.cpp
    src/FilaEventos.cpp
    src/SimulacaoMina.cpp
    # src/MqttInterface.cpp  <-- Se você criou um .cpp separado para o MQTT, descomente.
                             # Se fez tudo no .hpp (header-only), não precisa dessa linha.
)

# --- 4. EXECUTÁVEL 1: BACKEND DA SIMULAÇÃO ---
# Este é o programa que roda a lógica pesada e comunica com o MQTT
add_executable(simulacao_backend
    src/main.cpp          # Arquivo que contém o main() da simulação
    ${COMMON_SOURCES}     # Junta com as classes
)

# Linkar as bibliotecas necessárias
target_link_libraries(simulacao_backend
    paho-mqttpp3          # Wrapper C++ do MQTT
    paho-mqtt3a           # Biblioteca C Assíncrona do MQTT
    Threads::Threads      # Pthreads
    sfml-graphics sfml-window sfml-system

)