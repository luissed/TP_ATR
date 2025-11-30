CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude -pthread
LDFLAGS  = -lsfml-graphics -lsfml-window -lsfml-system -pthread

SRC_DIR  = src

TARGET   = main

OBJS = \
	$(SRC_DIR)/BufferCircular.o \
	$(SRC_DIR)/Caminhao.o \
	$(SRC_DIR)/FilaEventos.o \
	$(SRC_DIR)/SimulacaoMina.o \
	main.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
