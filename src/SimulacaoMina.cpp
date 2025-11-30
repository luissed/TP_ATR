// src/SimulacaoMina.cpp
#include "SimulacaoMina.hpp"

#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <cmath>   // sqrt, pow
#include <random>  // para spawn aleatorio

using namespace std::chrono_literals;

// -----------------------------------------------------------------------------
// Parametros de spawn (coordenadas em "metros" do mundo)
// Janela: 1920x1080, ORIGEM = centro, SCALE = 4 px/m
// Mundo visivel aproximadamente:
//   X ∈ [-240, 240], Y ∈ [-135, 135]
// Vamos limitar um pouco para nao colar na borda:
// -----------------------------------------------------------------------------
namespace {
    constexpr double SPAWN_X_MIN      = -220.0;
    constexpr double SPAWN_X_MAX      =  220.0;
    constexpr double SPAWN_Y_MIN      = -120.0;
    constexpr double SPAWN_Y_MAX      =  120.0;
    // distancia minima entre centros de caminhões ao nascer
    constexpr double SPAWN_DIST_MIN   = 25.0; // metros (2~3 "quadradinhos" de 10m)
    constexpr int    SPAWN_MAX_TRIES  = 200;
}

// ============================================================================
// Construtor / Destrutor
// ============================================================================
SimulacaoMina::SimulacaoMina(int numCaminhoes, std::size_t capacidadeBufferPadrao)
    : capacidadeBufferPadrao_(capacidadeBufferPadrao),
      rodando_(false)
{
    if (numCaminhoes < 0) numCaminhoes = 0;

    caminhoes_.reserve(static_cast<std::size_t>(numCaminhoes));

    // Criação inicial (sequencial, garagem linear)
    for (int i = 0; i < numCaminhoes; ++i) {
        criarNovoCaminhao(capacidadeBufferPadrao);
    }
}

SimulacaoMina::~SimulacaoMina() {
    parar();
}

// ============================================================================
// Controle global de execucao
// ============================================================================
void SimulacaoMina::iniciar() {
    if (rodando_) return;

    // --- 1. Configura MQTT da Central ---
    mqtt_ = std::make_unique<MqttInterface>("simulacao_central", 
        [this](const std::string& topic, const std::string& payload) {
            this->processarMensagemCentral(topic, payload);
        }
    );
    
    mqtt_->conectar();
    mqtt_->assinar("mina/simulacao/cmd"); 

    std::cout << "[SimulacaoMina] Sistema iniciado. Aguardando comandos MQTT...\n";
    rodando_ = true;

    // --- 2. Inicia Threads dos Caminhões ---
    {
        std::lock_guard<std::mutex> lock(mtxCaminhoes_);
        for (auto& c : caminhoes_) {
            c->iniciar();
        }
    }

    // --- 3. Inicia Thread de Segurança ---
    thSeguranca_ = std::thread(&SimulacaoMina::tarefaMonitoramentoSeguranca, this);
}

void SimulacaoMina::parar() {
    bool expected = true;
    if (!rodando_.compare_exchange_strong(expected, false)) return;

    std::cout << "[SimulacaoMina] Parando sistema...\n";
    
    {
        std::lock_guard<std::mutex> lock(mtxCaminhoes_);
        for (auto& c : caminhoes_) {
            c->parar();
        }
    }

    if (thSeguranca_.joinable()) thSeguranca_.join();
    if (mqtt_) mqtt_->desconectar();
}

// ============================================================================
// Criacao de Caminhoes (com garagem + spawn aleatorio visivel)
// ============================================================================
int SimulacaoMina::criarNovoCaminhao(std::size_t capacidadeBuffer) {
    // Bloqueia o vetor para proteger contra a thread de segurança
    std::lock_guard<std::mutex> lock(mtxCaminhoes_); 

    if (capacidadeBuffer == 0) capacidadeBuffer = capacidadeBufferPadrao_;

    int novoId = static_cast<int>(caminhoes_.size()) + 1;
    auto cam = std::make_unique<Caminhao>(novoId, capacidadeBuffer);

    // Ponteiro cru para poder iniciar depois, fora do unique_ptr
    Caminhao* ptrCru = cam.get();

    // ---------------------- CASO 1: Sistema ainda nao rodando ----------------
    // Usamos a "garagem" linear no eixo X (como antes), bom para testes.
    if (!rodando_) {
        int posX = (novoId - 1) * 30;  // 0, 30, 60, ...
        int posY = 0;
        cam->definirRota(posX, posY, posX, posY);

        caminhoes_.push_back(std::move(cam));

        std::cout << "[SimulacaoMina] Novo caminhao ID " << novoId 
                  << " criado na garagem (X=" << posX << ", Y=" << posY << ").\n";

        // Ainda nao chamamos iniciar() aqui, isso é feito em iniciar()
        return novoId;
    }

    // ------------------- CASO 2: Sistema rodando (botao + / MQTT) ------------
    // Agora queremos spawn ALEATORIO, DENTRO da area visivel, sem colidir.

    // RNG estático pra nao re-seedar toda hora
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> distX(SPAWN_X_MIN, SPAWN_X_MAX);
    std::uniform_real_distribution<double> distY(SPAWN_Y_MIN, SPAWN_Y_MAX);

    double spawnX = 0.0;
    double spawnY = 0.0;
    bool   found  = false;

    for (int tentativa = 0; tentativa < SPAWN_MAX_TRIES && !found; ++tentativa) {
        double candX = distX(rng);
        double candY = distY(rng);

        bool ok = true;

        // Checa distancia para todos caminhões existentes que estejam com dado no buffer
        for (auto& cPtr : caminhoes_) {
            RegistroBuffer reg{};
            if (!cPtr->lerUltimoRegistro(reg)) {
                // Se ainda nao tem dado (acabou de nascer etc.), ignoramos.
                continue;
            }

            double dx = candX - static_cast<double>(reg.sensores.i_posicao_x);
            double dy = candY - static_cast<double>(reg.sensores.i_posicao_y);
            double d2 = dx*dx + dy*dy;

            if (d2 < SPAWN_DIST_MIN * SPAWN_DIST_MIN) {
                ok = false;
                break;
            }
        }

        if (ok) {
            spawnX = candX;
            spawnY = candY;
            found  = true;
        }
    }

    if (!found) {
        // Fallback: se a tela estiver "lotada", cai na logica antiga linear
        int posX = (novoId - 1) * 30;
        int posY = 0;
        cam->definirRota(posX, posY, posX, posY);

        caminhoes_.push_back(std::move(cam));

        std::cout << "[SimulacaoMina] [AVISO] Nao foi possivel achar spawn sem colisao para ID "
                  << novoId << ". Usando fallback (garagem): X=" << posX << ", Y=" << posY << ".\n";

        // Como rodando_ == true, ja iniciamos as tarefas do novo caminhao
        ptrCru->iniciar();
        return novoId;
    }

    // Posicao aleatoria segura dentro da janela
    int spawnXi = static_cast<int>(std::lround(spawnX));
    int spawnYi = static_cast<int>(std::lround(spawnY));

    cam->definirRota(spawnXi, spawnYi, spawnXi, spawnYi);
    caminhoes_.push_back(std::move(cam));

    std::cout << "[SimulacaoMina] Novo caminhao ID " << novoId
              << " spawnado em (" << spawnXi << ", " << spawnYi << ") dentro da tela.\n";

    // Como rodando_ == true, iniciamos o caminhao imediatamente
    ptrCru->iniciar();

    return novoId;
}

// ============================================================================
// Callback MQTT (Central)
// ============================================================================
void SimulacaoMina::processarMensagemCentral(const std::string& /*topico*/, const std::string& payload) {
    std::cout << "[Mina Recv] " << payload << "\n";
    
    if (payload == "CMD:CRIAR_CAMINHAO") {
        // cria um caminhao em thread separada para nao travar callback do MQTT
        std::thread([this]() {
            this->criarNovoCaminhao();
        }).detach();
    }
   
    else if (payload.rfind("CMD:FALHA_TEMP:", 0) == 0) {
        int id = std::stoi(payload.substr(15));
        try { 
            injetarFalhaTemperatura(id); 
            std::cout << "[Simulacao] Injetando Falha de Temperatura no ID " << id << "\n"; 
        } catch(...) { std::cerr << "Erro ao injetar falha (ID invalido?)\n"; }
    }
    else if (payload.rfind("CMD:FALHA_ELET:", 0) == 0) {
        int id = std::stoi(payload.substr(15));
        try { 
            injetarFalhaEletrica(id); 
            std::cout << "[Simulacao] Injetando Falha Eletrica no ID " << id << "\n"; 
        } catch(...) { std::cerr << "Erro ao injetar falha\n"; }
    }
    else if (payload.rfind("CMD:FALHA_HIDR:", 0) == 0) {
        int id = std::stoi(payload.substr(15));
        try { 
            injetarFalhaHidraulica(id); 
            std::cout << "[Simulacao] Injetando Falha Hidraulica no ID " << id << "\n"; 
        } catch(...) { std::cerr << "Erro ao injetar falha\n"; }
    }
    // (pode expandir para outros comandos no futuro)
}

// ============================================================================
// Monitoramento de Segurança (Anti-colisao)
// ============================================================================
void SimulacaoMina::tarefaMonitoramentoSeguranca() {
    // const double DIST_ALERTA = 25.0;
    const double DIST_ALERTA  = 10.0;
    const double DIST_CRITICA = 5.0;

    while (rodando_) {
        {
            std::lock_guard<std::mutex> lock(mtxCaminhoes_);
            std::vector<bool> precisaReduzir(caminhoes_.size(), false);

            for (size_t i = 0; i < caminhoes_.size(); ++i) {
                for (size_t j = i + 1; j < caminhoes_.size(); ++j) {
                    RegistroBuffer rI, rJ;
                    if (!caminhoes_[i]->lerUltimoRegistro(rI)) continue;
                    if (!caminhoes_[j]->lerUltimoRegistro(rJ)) continue;

                    double dx = static_cast<double>(rI.sensores.i_posicao_x - rJ.sensores.i_posicao_x);
                    double dy = static_cast<double>(rI.sensores.i_posicao_y - rJ.sensores.i_posicao_y);
                    double dist = std::sqrt(dx*dx + dy*dy);

                    if (dist < DIST_CRITICA) {
                        std::cerr << "[COLISAO] EMERGENCIA! ID " << rI.id_caminhao 
                                  << " e " << rJ.id_caminhao << " (Dist: " << dist << "m)\n";
                        caminhoes_[i]->comandarParadaEmergencia();
                        caminhoes_[j]->comandarParadaEmergencia();
                    }

                    else if (dist < DIST_ALERTA) {
                        precisaReduzir[i] = true;
                        precisaReduzir[j] = true;
                    }
                }
            }

            for (size_t i = 0; i < caminhoes_.size(); ++i) {
                caminhoes_[i]->setReducaoSeguranca(precisaReduzir[i]);
            }
        } 
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// ============================================================================
// Getters / Utilitários
// ============================================================================
Caminhao& SimulacaoMina::getCaminhaoPorId(int id) {
    std::lock_guard<std::mutex> lock(mtxCaminhoes_);
    for (auto& c : caminhoes_) if (c->getId() == id) return *c;
    throw std::out_of_range("Nao encontrado");
}

const Caminhao& SimulacaoMina::getCaminhaoPorId(int id) const {
    for (const auto& c : caminhoes_) if (c->getId() == id) return *c;
    throw std::out_of_range("Nao encontrado");
}

void SimulacaoMina::injetarFalhaTemperatura(int id) { getCaminhaoPorId(id).injetarFalhaTemperaturaAlta(); }
void SimulacaoMina::injetarFalhaEletrica(int id)   { getCaminhaoPorId(id).injetarFalhaEletrica(); }
void SimulacaoMina::injetarFalhaHidraulica(int id) { getCaminhaoPorId(id).injetarFalhaHidraulica(); }

void SimulacaoMina::definirRotaCaminhao(int id, int x1, int y1, int x2, int y2) {
    getCaminhaoPorId(id).definirRota(x1, y1, x2, y2);
}

void SimulacaoMina::imprimirMapaTexto() const {
    for (const auto& cPtr : caminhoes_) {
        RegistroBuffer reg{};
        if (cPtr->lerUltimoRegistro(reg))
            std::cout << "ID: " << cPtr->getId()
                      << " Pos: (" << reg.sensores.i_posicao_x << ", "
                                   << reg.sensores.i_posicao_y << ")\n";
    }
}

void SimulacaoMina::rodarPorSegundos(int segundos) {
    for (int t = 0; t < segundos; ++t) {
        imprimirMapaTexto();
        std::this_thread::sleep_for(1s);
    }
}

Caminhao& SimulacaoMina::getCaminhao(std::size_t indice) {
    std::lock_guard<std::mutex> lock(mtxCaminhoes_);
    return *caminhoes_.at(indice);
}

const Caminhao& SimulacaoMina::getCaminhao(std::size_t indice) const {
    return *caminhoes_.at(indice);
}

std::size_t SimulacaoMina::quantidadeCaminhoes() const {
    std::lock_guard<std::mutex> lock(mtxCaminhoes_);
    return caminhoes_.size();
}
