CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude -pthread
# ADICIONADO: Bibliotecas do Paho MQTT (-lpaho-mqttpp3 -lpaho-mqtt3a) obrigatórias
LDFLAGS  = -lsfml-graphics -lsfml-window -lsfml-system -pthread -lpaho-mqttpp3 -lpaho-mqtt3a

SRC_DIR  = src

# Lista de objetos comuns ao programa
COMMON_OBJS = \
	$(SRC_DIR)/BufferCircular.o \
	$(SRC_DIR)/Caminhao.o \
	$(SRC_DIR)/FilaEventos.o \
	$(SRC_DIR)/SimulacaoMina.o

# Executável principal
TARGET_BACKEND = simulacao_backend

# Só vamos construir esse executável
all: $(TARGET_BACKEND)

# Regra para o Backend (usa src/main.cpp)
$(TARGET_BACKEND): $(COMMON_OBJS) $(SRC_DIR)/main.o
	$(CXX) $^ -o $@ $(LDFLAGS)

# Regra genérica de compilação
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(SRC_DIR)/*.o $(TARGET_BACKEND) main
