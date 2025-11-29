#include "../../include/rota/planejamento_rota.hpp"
#include <iostream>
#include <chrono>

PlanejamentoRota::PlanejamentoRota(Buffer& b) : buffer(b), ativo(false) {
}

PlanejamentoRota::~PlanejamentoRota() {
    parar();
}

void PlanejamentoRota::iniciar() {
    ativo = true;
    th_planejamento = std::thread(&PlanejamentoRota::loop, this);
    std::cout << "[PLANEJAMENTO] Sistema de rotas iniciado para N caminhoes.\n";
}

void PlanejamentoRota::parar() {
    ativo = false;
    if (th_planejamento.joinable()) {
        th_planejamento.join();
    }
}

void PlanejamentoRota::adicionarDestino(int id, int x, int y) {
    std::lock_guard<std::mutex> lock(mtx_rotas);
    filas_de_rotas[id].push({x, y});
    std::cout << "[PLANEJAMENTO] Novo destino agendado para Caminhao " << id 
              << ": (" << x << ", " << y << ")\n";
}

void PlanejamentoRota::limparRota(int id) {
    std::lock_guard<std::mutex> lock(mtx_rotas);
    // Swap com fila vazia é a forma padrão de limpar queue em C++
    std::queue<Coordenada> vazio;
    std::swap(filas_de_rotas[id], vazio);
    std::cout << "[PLANEJAMENTO] Rota limpa para Caminhao " << id << "\n";
}

void PlanejamentoRota::loop() {
    while (ativo) {
        // 1. Obtém o estado atual de TODA a frota do Buffer
        auto frota = buffer.getFrotaCompleta();

        // 2. Itera sobre cada caminhão encontrado
        for (const auto& caminhao : frota) {
            int id = caminhao.id_caminhao;

            // Verifica se precisamos enviar um novo comando
            // Critérios:
            // a) Caminhão está no modo AUTOMÁTICO (só planejamos para quem está auto)
            // b) Caminhão sinalizou que CHEGOU ao destino anterior
            // c) Temos pontos na fila para este caminhão
            
            bool precisa_novo_ponto = false;
            
            // Acesso protegido ao mapa de rotas local
            {
                std::lock_guard<std::mutex> lock(mtx_rotas);
                if (!filas_de_rotas[id].empty() && caminhao.estados.e_automatico) {
                    // Se o caminhão marcou "chegou" ou se é a primeira vez (setpoint zerado/inicial)
                    if (caminhao.navegacao.chegou) {
                        precisa_novo_ponto = true;
                    }
                }
            }

            if (precisa_novo_ponto) {
                Coordenada prox;
                
                // Retira o próximo ponto da fila
                {
                    std::lock_guard<std::mutex> lock(mtx_rotas);
                    prox = filas_de_rotas[id].front();
                    filas_de_rotas[id].pop();
                }

                // Envia para o Buffer (Isso vai disparar via MQTT ou memória interna)
                // O método atualizarSetpoint já deve setar 'chegou = false' internamente no Buffer
                buffer.atualizarSetpoint(id, prox.x, prox.y);

                std::cout << "[PLANEJAMENTO] Enviando Caminhao " << id 
                          << " para (" << prox.x << ", " << prox.y << ")\n";
            }
        }

        // Dorme um pouco para não consumir 100% da CPU verificando rotas
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}