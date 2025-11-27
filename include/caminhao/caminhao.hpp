#ifndef CAMINHAO_CAMINHAO_HPP
#define CAMINHAO_CAMINHAO_HPP

#include <thread>

struct CaminhaoContext;
struct CaminhaoRuntime;

void tarefa_tratamento_sensores(CaminhaoContext* ctx);
void tarefa_logica_comando(CaminhaoContext* ctx);
void tarefa_monitoramento_falhas(CaminhaoContext* ctx);
void tarefa_controle_navegacao(CaminhaoContext* ctx);
void tarefa_coletor_dados(CaminhaoContext* ctx);
void tarefa_interface_local(CaminhaoContext* ctx);

CaminhaoRuntime criar_caminhao(int id);
void parar_caminhao(CaminhaoRuntime& caminhao);
void join_caminhao(CaminhaoRuntime& caminhao);

#endif