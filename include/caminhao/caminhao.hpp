#ifndef CAMINHAO_CAMINHAO_HPP
#define CAMINHAO_CAMINHAO_HPP

#include <thread>
#include "../core/tipos.hpp"

struct CaminhaoContext { // contexto com tudo que as threads do caminhão compartilham
    InfoCaminhao info; // info atual do caminhão, informação dos sensores, atuadores, estados, comandos
    EventosFalhasCaminhao eventos;   // eventos de falha gerados pelo Monitoramento de Falhas
    bool rodando; // true  = threads devem continuar executando, false = threads devem encerrar o loop e terminar
};

struct CaminhaoRuntime {
    CaminhaoContext ctx; // contexto compartilhado entre todas as threads do caminhão (estado + controles)

    std::thread th_tratamento; // thread da tarefa cíclica de Tratamento de Sensores
    std::thread th_logica; // thread da tarefa cíclica Lógica de Comando
    std::thread th_monitoramento; // thread da tarefa cíclica de Monitoramento de Falhas
    std::thread th_controle; // thread da tarefa cíclica de Controle de Navegação
    std::thread th_coletor; // thread da tarefa de Coletor de Dados
    std::thread th_interface; // thread da tarefa de Interface Local
};

CaminhaoRuntime criar_caminhao(int id); // inicializa o CaminhaoContext, cria as threads th_tratamento, th_logica, th_monitoramento, th_controle, th_coletor e th_interface, cada uma rodando sua tarefa com &ctx, retorna um CaminhaoRuntime ligado
void parar_caminhao(CaminhaoRuntime& caminhao); // sinaliza para as threads do caminhão que elas devem encerrar
void join_caminhao(CaminhaoRuntime& caminhao);

#endif
