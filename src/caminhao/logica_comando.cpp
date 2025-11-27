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
    }
}