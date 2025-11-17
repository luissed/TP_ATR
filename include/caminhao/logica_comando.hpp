#ifndef CAMINHAO_LOGICA_COMANDO_HPP
#define CAMINHAO_LOGICA_COMANDO_HPP

#include "caminhao/caminhao.hpp"

void tarefa_logica_comando(CaminhaoContext* ctx); // Lógica de Comando, lê ctx.info.sensores e ctx.info.comandos, lê ctx.eventos para reagir a falhas, atualiza ctx.info.estados, atualiza ctx.info.atuadores com os comandos para os atuadores

#endif