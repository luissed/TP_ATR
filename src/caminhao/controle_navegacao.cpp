<<<<<<< Updated upstream
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
=======
// caminhao/controle_navegacao.cpp
#include "caminhao/controle_navegacao.hpp"
#include <iostream>
#include <chrono>
#include <thread>

// Tarefa cíclica de Controle de Navegação (CONSUMIDOR)
void tarefa_controle_navegacao(CaminhaoContext* ctx) {
    while (ctx->rodando) {
        InfoCaminhao info_consumida;

        // --- 1. Consumo do Buffer (SEÇÃO CRÍTICA) ---
        std::unique_lock<std::mutex> lock_buffer(ctx->mtx_buffer);

        // Espera de Condição (Evita Busy Waiting): Dorme se o buffer estiver vazio
        // Predicado: Buffer vazio (count == 0)?
        ctx->cv_buffer_cheio.wait(lock_buffer, [ctx] {
            return ctx->buffer.count > 0;
        });

        // Consome o dado do buffer circular
        info_consumida = ctx->buffer.dados[ctx->buffer.out];
        ctx->buffer.out = (ctx->buffer.out + 1) % BufferCircular::SIZE;
        ctx->buffer.count--;

        std::cout << "\t[T3: CONTROLE] Consumiu amostra x: " << info_consumida.sensores.i_posicao_x << std::endl;
        
        // Sinaliza que há um slot vazio (acorda o produtor)
        ctx->cv_buffer_vazio.notify_one();

        lock_buffer.unlock(); // Libera o Mutex do Buffer

        // --- 2. Lógica de Controle (Pode ser fora da seção crítica, idealmente) ---
        // Exemplo: Se o caminhão estiver no modo automático, calcula nova aceleração/direção.
        
        // Protege o acesso ao bloco de informações gerais (estados, atuadores)
        std::lock_guard<std::mutex> lock_info(ctx->mtx_info);
        
        if (ctx->info.estados.e_automatico) {
            // Lógica PID simulada
            ctx->info.atuadores.o_aceleracao = 50; 
            ctx->info.atuadores.o_direcao = 10;
        } else {
            // Em modo manual, o controle fica em 'off' (bumpless transfer)
            ctx->info.atuadores.o_aceleracao = 0; 
            ctx->info.atuadores.o_direcao = 0;
        }

        // Período de execução da tarefa (100ms - Hard Real-Time)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
>>>>>>> Stashed changes
}