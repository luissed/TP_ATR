#include "caminhao/controle_navegacao.hpp"
#include "caminhao/contexto.hpp"
#include "mqtt/mqtt_client.hpp"
#include <iostream>
#include <chrono>

void tarefa_controle_navegacao(CaminhaoContext* ctx) {
    MQTTClient mqtt_client;
    
    if (!mqtt_client.conectar("localhost", 1883, 
        "caminhao_controle_" + std::to_string(ctx->info.id_caminhao))) {
        std::cerr << "Erro ao conectar MQTT para controle" << std::endl;
        return;
    }

    int setpoint_velocidade = 30;
    int setpoint_direcao = 45;
    
    while (ctx->rodando) {
        mqtt_client.publicar("caminhao/estados/" + std::to_string(ctx->info.id_caminhao), 
                            ctx->info.estados);
        
        mqtt_client.publicar("caminhao/atuadores/" + std::to_string(ctx->info.id_caminhao), 
                            ctx->info.atuadores);
        
        if (ctx->info.estados.e_automatico && !ctx->info.estados.e_defeito) {
            int erro_velocidade = setpoint_velocidade - (ctx->info.sensores.i_posicao_x / 10);
            int erro_direcao = setpoint_direcao - ctx->info.sensores.i_angulo_x;
            
            ctx->info.atuadores.o_aceleracao = erro_velocidade * 2;
            ctx->info.atuadores.o_direcao = ctx->info.sensores.i_angulo_x + (erro_direcao * 0.3);
            
            if (ctx->info.atuadores.o_aceleracao > 100) ctx->info.atuadores.o_aceleracao = 100;
            if (ctx->info.atuadores.o_aceleracao < -100) ctx->info.atuadores.o_aceleracao = -100;
            if (ctx->info.atuadores.o_direcao > 180) ctx->info.atuadores.o_direcao = 180;
            if (ctx->info.atuadores.o_direcao < -180) ctx->info.atuadores.o_direcao = -180;
            
            static int contador = 0;
            if (contador++ % 50 == 0) {
                std::cout << "Caminhao " << ctx->info.id_caminhao << " [AUTO]: Vel=" 
                          << ctx->info.atuadores.o_aceleracao << "%, Dir=" 
                          << ctx->info.atuadores.o_direcao << "°" << std::endl;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    mqtt_client.desconectar();
    std::cout << "Controle de navegacao finalizado para caminhao " << ctx->info.id_caminhao << std::endl;
}