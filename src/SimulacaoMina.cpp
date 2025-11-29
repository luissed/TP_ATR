// src/SimulacaoMina.cpp
#include "SimulacaoMina.hpp"

#include <iostream>
#include <thread>
#include <chrono>

SimulacaoMina::SimulacaoMina(int numCaminhoes, std::size_t capacidadeBuffer) {
    if (numCaminhoes <= 0) {
        numCaminhoes = 1;
    }

    caminhoes_.reserve(static_cast<std::size_t>(numCaminhoes));
    for (int i = 0; i < numCaminhoes; ++i) {
        int id = i + 1;
        caminhoes_.push_back(std::make_unique<Caminhao>(id, capacidadeBuffer));
    }

    std::cout << "[SimulacaoMina] Criados " << numCaminhoes << " caminhões.\n";
}

SimulacaoMina::~SimulacaoMina() {
    parar();
}

void SimulacaoMina::iniciar() {
    std::cout << "[SimulacaoMina] Iniciando todos os caminhões...\n";
    for (auto& c : caminhoes_) {
        c->iniciar();
    }
}

void SimulacaoMina::parar() {
    std::cout << "[SimulacaoMina] Parando todos os caminhões...\n";
    for (auto& c : caminhoes_) {
        c->parar();
    }
}

void SimulacaoMina::rodarPorSegundos(int segundos) {
    using namespace std::chrono_literals;

    for (int t = 0; t < segundos; ++t) {
        std::cout << "\n===== Tempo global = " << t << " s =====\n";

        for (const auto& cPtr : caminhoes_) {
            const Caminhao& c = *cPtr;
            RegistroBuffer reg{};
            bool ok = c.lerUltimoRegistro(reg);

            std::cout << "Caminhao " << c.getId();
            if (ok) {
                std::cout << " | Estado=" << estadoToString(reg.estado)
                          << " | x=" << reg.sensores.i_posicao_x
                          << " | y=" << reg.sensores.i_posicao_y
                          << " | ang=" << reg.sensores.i_angulo_x << " deg"
                          << " | Tmot=" << reg.sensores.i_temperatura << " C"
                          << " | e_defeito=" << (reg.estados.e_defeito ? 1 : 0)
                          << " | e_auto=" << (reg.estados.e_automatico ? 1 : 0);
            } else {
                std::cout << " | (sem dados ainda no buffer)";
            }
            std::cout << "\n";
        }

        std::this_thread::sleep_for(1s);
    }
}

Caminhao& SimulacaoMina::getCaminhao(std::size_t indice) {
    return *caminhoes_.at(indice);
}

const Caminhao& SimulacaoMina::getCaminhao(std::size_t indice) const {
    return *caminhoes_.at(indice);
}

std::size_t SimulacaoMina::quantidadeCaminhoes() const {
    return caminhoes_.size();
}
