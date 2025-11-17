#ifndef CAMINHAO_MONITORAMENTO_FALHAS_HPP
#define CAMINHAO_MONITORAMENTO_FALHAS_HPP

#include "caminhao/caminhao.hpp"

void tarefa_monitoramento_falhas(CaminhaoContext* ctx); // Monitoramento de Falhas, lê ctx.info.sensores, detecta condições de falha, escreve eventos em ctx.eventos

#endif
