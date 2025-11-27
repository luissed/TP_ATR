#include "../include/caminhao/tratamento_sensores.hpp"

#include <chrono>
#include <thread>
#include <iostream>

// M últimas amostras
static constexpr int M_MEDIA_MOVEL = 5;

// simula tamanho do buffer circular
static constexpr int TAM_BUFFER_SIMULADO = 10;

void tarefa_tratamento_sensores(CaminhaoContext* ctx) {
    if (ctx == nullptr) {
        return;
    }

    using namespace std::chrono_literals;
    const auto PERIODO_TAREFA = 50ms;

    static int janela_x[M_MEDIA_MOVEL] = {0};
    static int janela_y[M_MEDIA_MOVEL] = {0};
    static int idx_janela = 0; // posição atual na janela [0..M-1]
    static int amostras_validas = 0; // quantas amostras já estão válidas (até M)
    static int idx_buffer_simulado = 0; // índice do buffer simulado

    // simula uma trajetória + ruído
    static int pos_x_bruta = 0;
    static int pos_y_bruta = 0;
    static int contador_ruido = 0;

    while (ctx->rodando) {
        pos_x_bruta += 10; // caminhão anda em x
        pos_y_bruta += 7; // caminhão anda em y

        contador_ruido++;
        int ruido_x = (contador_ruido % 5) - 2;
        int ruido_y = ((contador_ruido / 2) % 5) - 2;

        int pos_x_com_ruido = pos_x_bruta + ruido_x;
        int pos_y_com_ruido = pos_y_bruta + ruido_y;

        janela_x[idx_janela] = pos_x_com_ruido;
        janela_y[idx_janela] = pos_y_com_ruido;

        if (amostras_validas < M_MEDIA_MOVEL) {
            amostras_validas++;
        }

        idx_janela = (idx_janela + 1) % M_MEDIA_MOVEL;

        long soma_x = 0;
        long soma_y = 0;

        for (int i = 0; i < amostras_validas; ++i) {
            soma_x += janela_x[i];
            soma_y += janela_y[i];
        }

        int pos_x_filtrada = static_cast<int>(soma_x / amostras_validas);
        int pos_y_filtrada = static_cast<int>(soma_y / amostras_validas);

        SensoresCaminhao& sens = ctx->info.sensores;
        sens.i_posicao_x = pos_x_filtrada;
        sens.i_posicao_y = pos_y_filtrada;

        std::cout << "[TratamentoSensores] " << "buffer[" << idx_buffer_simulado << "] " << "bruto=(" << pos_x_com_ruido << "," << pos_y_com_ruido << ") " << "filtrado=(" << sens.i_posicao_x << "," << sens.i_posicao_y << ")" << std::endl;

        idx_buffer_simulado = (idx_buffer_simulado + 1) % TAM_BUFFER_SIMULADO;

        std::this_thread::sleep_for(PERIODO_TAREFA);
    }
}
