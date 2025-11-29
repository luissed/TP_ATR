#include "SimulacaoMina.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>

using namespace std::chrono_literals;

SimulacaoMina::SimulacaoMina(int numCaminhoes, std::size_t capacidadeBufferPadrao)
    : capacidadeBufferPadrao_(capacidadeBufferPadrao),
      rodando_(false)
{
    if (numCaminhoes < 0) {
        numCaminhoes = 0;
    }

    caminhoes_.reserve(static_cast<std::size_t>(numCaminhoes));

    for (int i = 0; i < numCaminhoes; ++i) {
        int id = static_cast<int>(caminhoes_.size()) + 1;
        caminhoes_.push_back(
            std::make_unique<Caminhao>(id, capacidadeBufferPadrao_)
        );
    }

    std::cout << "[SimulacaoMina] Criados " << numCaminhoes
              << " caminhões (inicialmente).\n";
}

SimulacaoMina::~SimulacaoMina() {
    parar();
}

// -----------------------------------------------------------------------------
// Controle global de execução
// -----------------------------------------------------------------------------

void SimulacaoMina::iniciar() {
    if (rodando_) {
        return;
    }
    std::cout << "[SimulacaoMina] Iniciando todos os caminhões...\n";
    rodando_ = true;
    for (auto& c : caminhoes_) {
        c->iniciar();
    }
}

void SimulacaoMina::parar() {
    if (!rodando_ && caminhoes_.empty()) {
        // Se nunca rodou e não há caminhões, nada a fazer.
        return;
    }
    std::cout << "[SimulacaoMina] Parando todos os caminhões...\n";
    for (auto& c : caminhoes_) {
        c->parar();
    }
    rodando_ = false;
}

// -----------------------------------------------------------------------------
// Simulação da Mina: criação de caminhões e injeção de falhas
// -----------------------------------------------------------------------------

int SimulacaoMina::criarNovoCaminhao(std::size_t capacidadeBuffer) {
    if (capacidadeBuffer == 0) {
        capacidadeBuffer = capacidadeBufferPadrao_;
    }

    int novoId = static_cast<int>(caminhoes_.size()) + 1;

    auto cam = std::make_unique<Caminhao>(novoId, capacidadeBuffer);
    Caminhao* ptrCru = cam.get();
    caminhoes_.push_back(std::move(cam));

    std::cout << "[SimulacaoMina] Criado novo caminhão com id="
              << novoId << " (buffer=" << capacidadeBuffer << " entradas).\n";

    // Se a simulação já estiver rodando, iniciamos as tarefas deste caminhão agora.
    if (rodando_) {
        ptrCru->iniciar();
    }

    return novoId;
}

Caminhao& SimulacaoMina::getCaminhaoPorId(int id) {
    for (auto& c : caminhoes_) {
        if (c->getId() == id) {
            return *c;
        }
    }
    throw std::out_of_range("[SimulacaoMina] Caminhao com id=" +
                            std::to_string(id) + " não encontrado.");
}

const Caminhao& SimulacaoMina::getCaminhaoPorId(int id) const {
    for (const auto& c : caminhoes_) {
        if (c->getId() == id) {
            return *c;
        }
    }
    throw std::out_of_range("[SimulacaoMina] Caminhao com id=" +
                            std::to_string(id) + " não encontrado.");
}

void SimulacaoMina::injetarFalhaTemperatura(int idCaminhao) {
    Caminhao& c = getCaminhaoPorId(idCaminhao);
    c.injetarFalhaTemperaturaAlta();
}

void SimulacaoMina::injetarFalhaEletrica(int idCaminhao) {
    Caminhao& c = getCaminhaoPorId(idCaminhao);
    c.injetarFalhaEletrica();
}

void SimulacaoMina::injetarFalhaHidraulica(int idCaminhao) {
    Caminhao& c = getCaminhaoPorId(idCaminhao);
    c.injetarFalhaHidraulica();
}

// -----------------------------------------------------------------------------
// Gestão da Mina: mapa e alteração de rotas
// -----------------------------------------------------------------------------

void SimulacaoMina::definirRotaCaminhao(int idCaminhao,
                                        int x_inicial, int y_inicial,
                                        int x_destino, int y_destino) {
    Caminhao& c = getCaminhaoPorId(idCaminhao);
    c.definirRota(x_inicial, y_inicial, x_destino, y_destino);
}

void SimulacaoMina::imprimirMapaTexto() const {
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
}

void SimulacaoMina::rodarPorSegundos(int segundos) {
    for (int t = 0; t < segundos; ++t) {
        std::cout << "\n===== Tempo global = " << t << " s =====\n";
        imprimirMapaTexto();
        std::this_thread::sleep_for(1s);
    }
}

// -----------------------------------------------------------------------------
// Acesso bruto por índice
// -----------------------------------------------------------------------------

Caminhao& SimulacaoMina::getCaminhao(std::size_t indice) {
    return *caminhoes_.at(indice);
}

const Caminhao& SimulacaoMina::getCaminhao(std::size_t indice) const {
    return *caminhoes_.at(indice);
}

std::size_t SimulacaoMina::quantidadeCaminhoes() const {
    return caminhoes_.size();
}
