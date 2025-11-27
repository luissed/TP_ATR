<<<<<<< Updated upstream
#include "caminhao/tratamento_sensores.hpp"
#include "caminhao/contexto.hpp"
#include "core/buffer.hpp"
#include "mqtt/mqtt_client.hpp"
#include <vector>
#include <numeric>
#include <chrono>
#include <random>
#include <iostream>

extern BufferCircular<InfoCaminhao> buffer_sensores;

class FiltroMediaMovel {
private:
    std::vector<int> historico;
    size_t ordem;
    size_t indice;

public:
    FiltroMediaMovel(size_t ord) : ordem(ord), indice(0) {
        historico.resize(ordem, 0);
    }

    int filtrar(int novo_valor) {
        historico[indice] = novo_valor;
        indice = (indice + 1) % ordem;
        
        int soma = std::accumulate(historico.begin(), historico.end(), 0);
        return soma / static_cast<int>(ordem);
    }
};

void tarefa_tratamento_sensores(CaminhaoContext* ctx) {
    MQTTClient mqtt_client;
    FiltroMediaMovel filtro_x(5), filtro_y(5), filtro_angulo(5), filtro_temp(3);
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis_pos(-5, 5);
    std::uniform_int_distribution<> dis_temp(80, 110);
    std::uniform_int_distribution<> dis_angulo(-10, 10);
    
    int pos_base_x = ctx->info.id_caminhao * 100;
    int pos_base_y = ctx->info.id_caminhao * 50;
    int angulo_base = 0;
    
    if (!mqtt_client.conectar("localhost", 1883, 
        "caminhao_sensores_" + std::to_string(ctx->info.id_caminhao))) {
        return;
    }

    while (ctx->rodando) {
        SensoresCaminhao sensores_brutos;
        sensores_brutos.i_posicao_x = pos_base_x + dis_pos(gen);
        sensores_brutos.i_posicao_y = pos_base_y + dis_pos(gen);
        sensores_brutos.i_angulo_x = angulo_base + dis_angulo(gen);
        sensores_brutos.i_temperatura = dis_temp(gen);
        sensores_brutos.i_falha_eletrica = false;
        sensores_brutos.i_falha_hidraulica = false;
        
        ctx->info.sensores.i_posicao_x = filtro_x.filtrar(sensores_brutos.i_posicao_x);
        ctx->info.sensores.i_posicao_y = filtro_y.filtrar(sensores_brutos.i_posicao_y);
        ctx->info.sensores.i_angulo_x = filtro_angulo.filtrar(sensores_brutos.i_angulo_x);
        ctx->info.sensores.i_temperatura = filtro_temp.filtrar(sensores_brutos.i_temperatura);
        ctx->info.sensores.i_falha_eletrica = sensores_brutos.i_falha_eletrica;
        ctx->info.sensores.i_falha_hidraulica = sensores_brutos.i_falha_hidraulica;
        
        mqtt_client.publicar("caminhao/sensores/" + std::to_string(ctx->info.id_caminhao), 
                            ctx->info.sensores);
        buffer_sensores.escrever(ctx->info);
        
        pos_base_x += ctx->info.atuadores.o_aceleracao / 10;
        angulo_base = ctx->info.atuadores.o_direcao;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    mqtt_client.desconectar();
=======
// caminhao/tratamento_sensores.cpp
#include "caminhao/tratamento_sensores.hpp"
#include <iostream>
#include <chrono>
#include <thread>

// Simula a leitura e filtragem dos sensores (PRODUTOR)
void tarefa_tratamento_sensores(CaminhaoContext* ctx) {
    while (ctx->rodando) {
        // --- 1. Leitura Simulada e Processamento (FORA da Seção Crítica) ---
        SensoresCaminhao nova_leitura = { 
            /* simulação de leitura com ruído */
            .i_posicao_x = rand() % 100, 
            .i_posicao_y = rand() % 100,
            .i_angulo_x = rand() % 360,
            .i_temperatura = 90, // Simulação de temperatura
            .i_falha_eletrica = false,
            .i_falha_hidraulica = false
        };
        
        InfoCaminhao nova_info = ctx->info;
        nova_info.sensores = nova_leitura;

        // --- 2. Inserção no Buffer (SEÇÃO CRÍTICA) ---
        std::unique_lock<std::mutex> lock(ctx->mtx_buffer);

        // Espera de Condição (Evita Busy Waiting): Dorme se o buffer estiver cheio
        // Predicado: Buffer cheio (count == SIZE)?
        ctx->cv_buffer_vazio.wait(lock, [ctx] {
            return ctx->buffer.count < BufferCircular::SIZE; 
        });

        // Insere o novo dado no buffer circular
        ctx->buffer.dados[ctx->buffer.in] = nova_info;
        ctx->buffer.in = (ctx->buffer.in + 1) % BufferCircular::SIZE;
        ctx->buffer.count++;
        
        std::cout << "[T1: SENSORES] Produziu amostra. Itens no buffer: " << ctx->buffer.count << std::endl;

        // Sinaliza que há um slot cheio (acorda consumidores)
        ctx->cv_buffer_cheio.notify_all(); 

        lock.unlock(); // Libera o Mutex

        // Período de execução da tarefa (50ms - Hard Real-Time)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
>>>>>>> Stashed changes
}