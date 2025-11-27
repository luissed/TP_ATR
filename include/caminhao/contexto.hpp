#ifndef CAMINHAO_CONTEXTO_HPP
#define CAMINHAO_CONTEXTO_HPP

#include <thread>
#include "core/tipos.hpp"

struct CaminhaoContext {
    InfoCaminhao info;
    EventosFalhasCaminhao eventos;
    bool rodando;
    
    CaminhaoContext(int id) : rodando(true) {
        info.id_caminhao = id;
        info.estados.e_defeito = false;
        info.estados.e_automatico = false;
        info.atuadores.o_aceleracao = 0;
        info.atuadores.o_direcao = 0;
        
        eventos.ev_falha_eletrica = false;
        eventos.ev_falha_hidraulica = false;
        eventos.ev_sobretemperatura = false;
    }
};

struct CaminhaoRuntime {
    CaminhaoContext ctx;
    std::thread th_tratamento;
    std::thread th_logica;
    std::thread th_monitoramento;
    std::thread th_controle;
    std::thread th_coletor;
    std::thread th_interface;
    
    CaminhaoRuntime(int id) : ctx(id) {}
};

#endif