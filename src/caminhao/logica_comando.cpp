<<<<<<< Updated upstream
#include "caminhao/logica_comando.hpp"
#include "caminhao/contexto.hpp"
#include "core/buffer.hpp"
#include <iostream>
#include <chrono>

extern BufferCircular<InfoCaminhao> buffer_sensores;

void tarefa_logica_comando(CaminhaoContext* ctx) {
    InfoCaminhao info_buffer;
    
    while (ctx->rodando) {
        if (buffer_sensores.ler(info_buffer)) {
            ctx->info.sensores = info_buffer.sensores;
            
            if (ctx->info.comandos.c_automatico) {
                ctx->info.estados.e_automatico = true;
                ctx->info.comandos.c_automatico = false;
                std::cout << "Caminhao " << ctx->info.id_caminhao << ": Modo AUTOMATICO ativado" << std::endl;
            }
            
            if (ctx->info.comandos.c_man) {
                ctx->info.estados.e_automatico = false;
                ctx->info.comandos.c_man = false;
                std::cout << "Caminhao " << ctx->info.id_caminhao << ": Modo MANUAL ativado" << std::endl;
            }
            
            if (ctx->info.comandos.c_rearme && ctx->info.estados.e_defeito) {
                ctx->info.estados.e_defeito = false;
                ctx->info.comandos.c_rearme = false;
                std::cout << "Caminhao " << ctx->info.id_caminhao << ": Sistema REARMADO" << std::endl;
            }
            
            if (!ctx->info.estados.e_automatico && !ctx->info.estados.e_defeito) {
                if (ctx->info.comandos.c_acelera) {
                    ctx->info.atuadores.o_aceleracao = 50;
                    ctx->info.comandos.c_acelera = false;
                }
                
                if (ctx->info.comandos.c_direita) {
                    ctx->info.atuadores.o_direcao -= 5;
                    ctx->info.comandos.c_direita = false;
                }
                
                if (ctx->info.comandos.c_esquerda) {
                    ctx->info.atuadores.o_direcao += 5;
                    ctx->info.comandos.c_esquerda = false;
                }
            }
            
            if (ctx->eventos.ev_falha_eletrica || ctx->eventos.ev_falha_hidraulica || 
                ctx->eventos.ev_sobretemperatura) {
                ctx->info.estados.e_defeito = true;
                ctx->info.atuadores.o_aceleracao = 0;
                std::cout << "Caminhao " << ctx->info.id_caminhao << ": FALHA DETECTADA - Parando veiculo" << std::endl;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
=======
// caminhao/logica_comando.cpp
#include "caminhao/logica_comando.hpp"
#include <iostream>
#include <chrono>
#include <thread>

void tarefa_logica_comando(CaminhaoContext* ctx) {
    while (ctx->rodando) {
        
        bool falha_ativa = false;
        
        // --- 1. Reage a Eventos de Falha (Leitura Protegida) ---
        {
            std::lock_guard<std::mutex> lock_eventos(ctx->mtx_eventos);
            if (ctx->eventos.ev_sobretemperatura || ctx->eventos.ev_falha_eletrica) {
                falha_ativa = true;
            }
        }

        // --- 2. Atualização dos Estados (SEÇÃO CRÍTICA) ---
        std::lock_guard<std::mutex> lock_info(ctx->mtx_info);

        if (falha_ativa) {
            // Se há falha, o estado de defeito é TRUE e o modo automático é desativado
            ctx->info.estados.e_defeito = true;
            ctx->info.estados.e_automatico = false;
            std::cerr << "\t\t[T2: LÓGICA] MODO DEFEITO ATIVADO!" << std::endl;
        } 
        else if (ctx->info.comandos.c_automatico) {
            // Se não há falha e o comando automático está ativo, entra em modo AUTOMÁTICO
            ctx->info.estados.e_defeito = false;
            ctx->info.estados.e_automatico = true;
        } 
        else if (ctx->info.comandos.c_man) {
            // Comando para modo manual
            ctx->info.estados.e_defeito = false;
            ctx->info.estados.e_automatico = false;
        }
        
        // Período de execução da tarefa (200ms)
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
>>>>>>> Stashed changes
    }
}