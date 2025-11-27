#ifndef CAMINHAO_TRATAMENTO_SENSORES_HPP // 
#define CAMINHAO_TRATAMENTO_SENSORES_HPP

#include "caminhao/caminhao.hpp"

void tarefa_tratamento_sensores(CaminhaoContext* ctx); // Tarefa cíclica de Tratamento de Sensores, vai ler os sensores via MQTT e atualizar ctx->info.sensores

#endif