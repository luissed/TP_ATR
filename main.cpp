// main.cpp
#include <iostream>
#include <thread>
#include <chrono>

#include "SimulacaoMina.hpp"

int main() {
    using namespace std::chrono_literals;

    std::cout << "===== Interface de Simulacao da Mina =====\n";

    SimulacaoMina mina(2, 200);

    mina.getCaminhao(0).definirRota(0, 0, 100, 50);

    mina.iniciar();

    mina.rodarPorSegundos(10);

    std::cout << "\n>>> [Interface] Colocando CAMINHAO 1 em modo MANUAL...\n";
    mina.getCaminhao(0).comandarManual();

    mina.rodarPorSegundos(5);

    std::cout << "\n>>> [Interface] Voltando CAMINHAO 1 para modo AUTOMATICO...\n";
    mina.getCaminhao(0).comandarAutomatico();

    mina.rodarPorSegundos(10);

    std::cout << "\n>>> [Interface] Enviando comando de REARME para CAMINHAO 1...\n";
    mina.getCaminhao(0).comandarRearme();

    mina.rodarPorSegundos(5);

    mina.parar();

    std::cout << "===== Simulacao da Mina encerrada =====\n";
    return 0;
}
