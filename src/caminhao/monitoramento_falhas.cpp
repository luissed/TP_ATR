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
}