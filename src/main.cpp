#include <iostream>
#include <thread>
#include <chrono>
#include <csignal>
#include <atomic>
#include "SimulacaoMina.hpp"

// Variável para controlar o encerramento suave com Ctrl+C
std::atomic<bool> g_rodando(true);

void signalHandler(int signum) {
    std::cout << "\n[SISTEMA] Interrupcao recebida (" << signum << "). Encerrando...\n";
    g_rodando = false;
}

int main(int argc, char* argv[]) {
    // Configura o sinal de Ctrl+C
    std::signal(SIGINT, signalHandler);

    std::cout << "=== SIMULACAO BACKEND (Mina Inteligente) ===\n";
    std::cout << "Aguardando conexao MQTT...\n";

    // Define número de caminhões (Padrão 0, cria dinamicamente ou via argumento)
    int numCaminhoes = 0;
    if (argc > 1) {
        numCaminhoes = std::atoi(argv[1]);
    }

    // Cria a mina (Backend)
    SimulacaoMina mina(numCaminhoes, 200);

    // Se não passou argumento, cria 1 caminhão inicial para teste
    if (numCaminhoes == 0) {
        mina.criarNovoCaminhao(); 
    }

    // Inicia threads e MQTT
    mina.iniciar();

    std::cout << "Sistema rodando! Use a Interface Python ou terminais para controlar.\n";
    std::cout << "Pressione Ctrl+C para encerrar.\n";

    // Loop principal mantém o programa vivo até receber sinal de parada
    while (g_rodando) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "[SISTEMA] Parando simulacao...\n";
    mina.parar();
    std::cout << "[SISTEMA] Encerrado com sucesso.\n";

    return 0;
}