#include "caminhao/caminhao.hpp"
#include "caminhao/contexto.hpp"
#include <iostream>
#include <vector>
#include <csignal>

std::vector<CaminhaoRuntime> caminhoes;

void sinal_handler(int sinal) {
    std::cout << "\nEncerrando sistema..." << std::endl;
    for (auto& caminhao : caminhoes) {
        parar_caminhao(caminhao);
    }
}

int main() {
    std::signal(SIGINT, sinal_handler);
    
    std::cout << "Iniciando sistema de caminhoes autonomos..." << std::endl;
    
    int num_caminhoes = 3;
    for (int i = 0; i < num_caminhoes; i++) {
        std::cout << "Criando caminhao " << i << std::endl;
        caminhoes.push_back(criar_caminhao(i));
    }
    
    std::cout << "Sistema em operacao. Pressione Ctrl+C para encerrar." << std::endl;
    
    for (auto& caminhao : caminhoes) {
        join_caminhao(caminhao);
    }
    
    std::cout << "Sistema encerrado." << std::endl;
    return 0;
}