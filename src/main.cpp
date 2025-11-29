#include <iostream>
#include <vector>
#include <thread>
#include <memory> 
#include <chrono>
#include <cmath> 

#include "../include/core/buffer.hpp"
#include "../include/rota/planejamento_rota.hpp"
#include "../include/caminhao/coletor_dados.hpp" 

const double PI = 3.14159265358979323846;

// Simulação Física agora recebe a LISTA de buffers para iterar sobre cada um
void threadSimulacaoFisica(std::vector<std::shared_ptr<Buffer>>& buffers, const std::vector<int>& ids, bool* rodando) {
    std::cout << "[SIMULADOR] Fisica para " << ids.size() << " caminhoes iniciada.\n";
    
    while (*rodando) {
        for (size_t i = 0; i < buffers.size(); i++) {
            // Pega o buffer e o ID correspondente ao índice i
            auto& buffer_ptr = buffers[i];
            int id = ids[i]; 
            
            InfoCaminhao info;
            
            // Acessa o buffer específico deste caminhão
            if (buffer_ptr->getInfoCaminhao(id, info)) {
                
                double alvoX = static_cast<double>(info.navegacao.x_setpoint);
                double alvoY = static_cast<double>(info.navegacao.y_setpoint);
                double atualX = static_cast<double>(info.sensores.i_posicao_x);
                double atualY = static_cast<double>(info.sensores.i_posicao_y);

                double dx = alvoX - atualX;
                double dy = alvoY - atualY;
                double distancia = std::sqrt(dx*dx + dy*dy);

                if (distancia > 1.0) {
                    double velocidade = 2.0; 
                    double angulo_rad = std::atan2(dy, dx); 
                    
                    atualX += velocidade * std::cos(angulo_rad);
                    atualY += velocidade * std::sin(angulo_rad);
                    
                    InfoCaminhao novaInfo = info;
                    novaInfo.sensores.i_posicao_x = static_cast<int>(atualX);
                    novaInfo.sensores.i_posicao_y = static_cast<int>(atualY);
                    novaInfo.sensores.i_angulo_x = static_cast<int>(angulo_rad * 180.0 / PI);
                    novaInfo.atuadores.o_aceleracao = static_cast<int>(velocidade * 10.0); 
                    
                    buffer_ptr->setInfoCaminhao(id, novaInfo);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Thread auxiliar para o Coletor pegar dados de TODOS os buffers
void threadColetorGlobal(std::vector<std::shared_ptr<Buffer>>& buffers, const std::vector<int>& ids, ColetorDados* coletor, bool* rodando) {
    while (*rodando) {
        for (size_t i = 0; i < buffers.size(); i++) {
            // Loga o estado de cada caminhão periodicamente
            // Em um sistema real, o coletor pode apenas escutar eventos
            InfoCaminhao info;
            if (buffers[i]->getInfoCaminhao(ids[i], info)) {
                 // Formata uma string simples de status
                 std::string msg = "Monitoramento Periodico - Pos: " + 
                                   std::to_string(info.sensores.i_posicao_x) + "," + 
                                   std::to_string(info.sensores.i_posicao_y);
                 coletor->log(ids[i], msg);
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    // IDs dos caminhões que vamos criar
    std::vector<int> ids_caminhoes;
    ids_caminhoes.push_back(101);
    ids_caminhoes.push_back(102);
    ids_caminhoes.push_back(103); // Adicionei mais um para testar N=3

    // --- ARQUITETURA N BUFFERS ---
    // Vetor de ponteiros inteligentes para os Buffers
    std::vector<std::shared_ptr<Buffer>> buffers;
    
    // Vetores de componentes
    std::vector<std::unique_ptr<PlanejamentoRota>> planejadores;
    std::vector<std::thread> threads_rota;

    // Inicialização da Arquitetura
    for (size_t i = 0; i < ids_caminhoes.size(); i++) {
        int id = ids_caminhoes[i];
        
        // 1. Cria um NOVO buffer para este caminhão específico
        auto buffer_ptr = std::make_shared<Buffer>();
        
        // Configura estado inicial
        buffer_ptr->atualizarPosicao(id, 0, 0);
        buffer_ptr->setModoAutomatico(id, true);
        
        // Guarda na lista global
        buffers.push_back(buffer_ptr);

        // 2. Cria o Planejador linkado a ESTE buffer específico
        // Note: Passamos *buffer_ptr (o objeto real) para o planejador
        auto planner = std::make_unique<PlanejamentoRota>(*buffer_ptr, id);
        
        // Configura rotas diferentes para ficar dinâmico
        if (id == 101) {
            planner->adicionarPonto(10, 10);
            planner->adicionarPonto(50, 50);
        } else if (id == 102) {
            planner->adicionarPonto(0, 20);
            planner->adicionarPonto(0, 40);
        } else {
            planner->adicionarPonto(100, 100); // Caminhão 103 vai longe
        }

        planejadores.push_back(std::move(planner));
    }

    // Como o ColetorDados espera 1 buffer no construtor (pelo seu código antigo),
    // vamos criar um "Dummy Buffer" ou alterar o Coletor. 
    // Para simplificar e manter seu código rodando agora, vamos instanciar o Coletor
    // passando o primeiro buffer apenas para inicializar, mas usaremos ele de forma customizada na thread.
    // (O ideal seria refatorar o Coletor para receber a lista, mas vamos adaptar na threadColetorGlobal)
    Buffer buffer_dummy; 
    ColetorDados coletor(buffer_dummy);

    // Inicia Threads de Planejamento de Rota
    for (size_t i = 0; i < planejadores.size(); i++) {
        threads_rota.emplace_back(&PlanejamentoRota::loop, planejadores[i].get());
    }

    // Controle geral
    bool sistema_rodando = true;

    // Inicia Simulador Físico (passando a lista de buffers)
    std::thread t_sim(threadSimulacaoFisica, std::ref(buffers), std::ref(ids_caminhoes), &sistema_rodando);
    
    // Inicia Coletor Global (passando a lista de buffers)
    std::thread t_log(threadColetorGlobal, std::ref(buffers), std::ref(ids_caminhoes), &coletor, &sistema_rodando);
    
    // Thread do loop interno do objeto Coletor (para manter arquivo aberto, etc)
    std::thread t_coletor_interno(&ColetorDados::loop, &coletor);

    std::cout << "\n=== SISTEMA DE N CAMINHOES INICIADO ===\n";
    std::this_thread::sleep_for(std::chrono::seconds(15));
    std::cout << "\n=== ENCERRANDO ===\n";

    // Parada limpa
    for (size_t i = 0; i < planejadores.size(); i++) {
        planejadores[i]->parar();
    }
    
    coletor.parar();
    sistema_rodando = false;

    if (t_sim.joinable()) t_sim.join();
    if (t_log.joinable()) t_log.join();
    if (t_coletor_interno.joinable()) t_coletor_interno.join();
    
    for (size_t i = 0; i < threads_rota.size(); i++) {
        if (threads_rota[i].joinable()) threads_rota[i].join();
    }

    return 0;
}