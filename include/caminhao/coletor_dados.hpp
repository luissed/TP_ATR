#ifndef CAMINHAO_COLETOR_DADOS_HPP
#define CAMINHAO_COLETOR_DADOS_HPP

#include "caminhao/caminhao.hpp"

void tarefa_coletor_dados(CaminhaoContext* ctx); // Coletor de Dados, lê ctx.info, lê ctx.eventos para registrar quando falhas ocorreram, grava essas informações

#endif