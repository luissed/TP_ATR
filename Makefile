CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude -pthread

LDFLAGS  = -lsfml-graphics -lsfml-window -lsfml-system -pthread -lpaho-mqttpp3 -lpaho-mqtt3a

SRC_DIR  = src


COMMON_OBJS = \
	$(SRC_DIR)/BufferCircular.o \
	$(SRC_DIR)/Caminhao.o \
	$(SRC_DIR)/FilaEventos.o \
	$(SRC_DIR)/SimulacaoMina.o


TARGET_GUI     = gui_gestao

all: $(TARGET_GUI)


$(TARGET_GUI): $(COMMON_OBJS) $(SRC_DIR)/main.o
	$(CXX) $^ -o $@ $(LDFLAGS)


%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(SRC_DIR)/*.o $(TARGET_GUI)