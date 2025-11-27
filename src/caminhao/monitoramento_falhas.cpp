<<<<<<< Updated upstream
#include "caminhao/monitoramento_falhas.hpp"
#include "caminhao/contexto.hpp"
#include <iostream>
#include <chrono>

void tarefa_monitoramento_falhas(CaminhaoContext* ctx) {
    bool ultima_falha_eletrica = false;
    bool ultima_falha_hidraulica = false;
    bool ultima_sobretemperatura = false;
    
    while (ctx->rodando) {
        if (ctx->info.sensores.i_temperatura > 120) {
            ctx->eventos.ev_sobretemperatura = true;
            if (!ultima_sobretemperatura) {
                std::cout << "Caminhao " << ctx->info.id_caminhao << ": ALERTA - Sobretemperatura critica!" << std::endl;
            }
        } else if (ctx->info.sensores.i_temperatura < 95) {
            ctx->eventos.ev_sobretemperatura = false;
        }
        
        if (ctx->info.sensores.i_temperatura > 95 && !ctx->eventos.ev_sobretemperatura) {
            std::cout << "Caminhao " << ctx->info.id_caminhao << ": Alerta - Temperatura alta: " 
                      << ctx->info.sensores.i_temperatura << "C" << std::endl;
        }
        
        if (ctx->info.sensores.i_falha_eletrica && !ultima_falha_eletrica) {
            ctx->eventos.ev_falha_eletrica = true;
            std::cout << "Caminhao " << ctx->info.id_caminhao << ": FALHA ELETRICA detectada!" << std::endl;
        }
        ultima_falha_eletrica = ctx->info.sensores.i_falha_eletrica;
        
        if (ctx->info.sensores.i_falha_hidraulica && !ultima_falha_hidraulica) {
            ctx->eventos.ev_falha_hidraulica = true;
            std::cout << "Caminhao " << ctx->info.id_caminhao << ": FALHA HIDRAULICA detectada!" << std::endl;
        }
        ultima_falha_hidraulica = ctx->info.sensores.i_falha_hidraulica;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    std::cout << "Monitoramento de falhas finalizado para caminhao " << ctx->info.id_caminhao << std::endl;
=======
// caminhao/monitoramento_falhas.cpp
#include "caminhao/monitoramento_falhas.hpp"
#include <iostream>
#include <chrono>
#include <thread>

void tarefa_monitoramento_falhas(CaminhaoContext* ctx) {
    while (ctx->rodando) {
        // Assume que a tarefa lê o estado atual dos sensores no ctx->info
        // (Nota: T1 deve periodicamente copiar o dado mais recente para ctx->info
        // ou essa tarefa deve ler o dado diretamente do sensor, mas aqui lemos a info)

        // --- 1. Detecção de Condição de Falha (Exemplo: Temperatura) ---
        bool condicao_critica = false;
        
        // Acesso Protegido aos dados de sensores (leitura)
        {
            std::lock_guard<std::mutex> lock(ctx->mtx_info);
            // Requisito: gera defeito se T > 120°C.
            if (ctx->info.sensores.i_temperatura > 120) { 
                condicao_critica = true;
            }
        }
        
        // --- 2. Geração do Evento (SEÇÃO CRÍTICA) ---
        if (condicao_critica) {
            // Protege o acesso ao bloco de eventos
            std::lock_guard<std::mutex> lock_eventos(ctx->mtx_eventos);
            
            // Define o evento de sobretemperatura como TRUE
            ctx->eventos.ev_sobretemperatura = true; 
            
            // NOTIFICAÇÃO: Sinaliza que houve uma mudança de evento (Acorda T2, T3, T6)
            // Note: O mtx_eventos precisa ter uma CV associada para notificar outras threads.
            // Aqui estamos simplificando, mas o ideal seria usar CV aqui também.

            std::cerr << "--- [T4: FALHAS] Evento de SOBRETEMPERATURA GERADO! ---" << std::endl;
        }
        
        // Período de execução da tarefa (200ms)
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
>>>>>>> Stashed changes
}