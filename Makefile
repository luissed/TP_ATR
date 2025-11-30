CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude -pthread
# ADICIONADO: Bibliotecas do Paho MQTT (-lpaho-mqttpp3 -lpaho-mqtt3a) obrigatórias
LDFLAGS  = -lsfml-graphics -lsfml-window -lsfml-system -pthread -lpaho-mqttpp3 -lpaho-mqtt3a

SRC_DIR  = src

# Lista de objetos comuns a ambos os programas
COMMON_OBJS = \
	$(SRC_DIR)/BufferCircular.o \
	$(SRC_DIR)/Caminhao.o \
	$(SRC_DIR)/FilaEventos.o \
	$(SRC_DIR)/SimulacaoMina.o

# Definir os dois executáveis que queremos gerar
TARGET_BACKEND = simulacao_backend
TARGET_GUI     = gui_gestao

all: $(TARGET_BACKEND) $(TARGET_GUI)

# Regra para o Backend (Lógica)
$(TARGET_BACKEND): $(COMMON_OBJS) $(SRC_DIR)/main.o
	$(CXX) $^ -o $@ $(LDFLAGS)

# Regra para a GUI (Interface)
$(TARGET_GUI): $(COMMON_OBJS) $(SRC_DIR)/main_gui.o
	$(CXX) $^ -o $@ $(LDFLAGS)

# Regra genérica de compilação
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(SRC_DIR)/*.o $(TARGET_BACKEND) $(TARGET_GUI) main