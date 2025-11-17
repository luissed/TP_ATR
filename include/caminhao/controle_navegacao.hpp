#ifndef CAMINHAO_CONTROLE_NAVEGACAO_HPP
#define CAMINHAO_CONTROLE_NAVEGACAO_HPP

#include "caminhao/caminhao.hpp"

void tarefa_controle_navegacao(CaminhaoContext* ctx); // Controle de Navegação, lê ctx.info.sensores, lê ctx.info.estados, lê ctx.eventos, escreve em ctx.info.atuadores

#endif
