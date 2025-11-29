// include/Tipos.hpp
#pragma once

#include <string>

struct SensoresCaminhao {
    int  i_posicao_x;
    int  i_posicao_y;
    int  i_angulo_x;
    int  i_temperatura;
    bool i_falha_eletrica;
    bool i_falha_hidraulica;
};

struct AtuadoresCaminhao {
    int o_aceleracao;
    int o_direcao;
};

struct EstadosCaminhao {
    bool e_defeito;
    bool e_automatico;
};

struct ComandosCaminhao {
    bool c_automatico;
    bool c_man;
    bool c_rearme;
    bool c_acelera;
    bool c_direita;
    bool c_esquerda;
};

enum class EstadoCaminhao {
    Parado,
    EmMovimento,
    EmFalha
};

struct SetpointsCaminhao {
    int sp_posicao_x;
    int sp_posicao_y;
    int sp_angulo_x;
};

struct RegistroBuffer {
    double tempoSimulacao_s;
    int    id_caminhao;

    EstadoCaminhao   estado;
    SensoresCaminhao sensores;
    AtuadoresCaminhao atuadores;
    EstadosCaminhao  estados;
    ComandosCaminhao comandos;
    SetpointsCaminhao setpoints;
};

enum class TipoEvento {
    FalhaTemperaturaAlta,
    FalhaEletrica,
    FalhaHidraulica,
    ComandoRearme,
    Outro
};

struct Evento {
    TipoEvento tipo;
    std::string descricao;
    double tempoSimulacao_s;
    int id_caminhao;
};

inline std::string estadoToString(EstadoCaminhao e) {
    switch (e) {
        case EstadoCaminhao::Parado:      return "Parado";
        case EstadoCaminhao::EmMovimento: return "Em movimento";
        case EstadoCaminhao::EmFalha:     return "EM FALHA";
        default:                          return "Desconhecido";
    }
}

inline std::string tipoEventoToString(TipoEvento t) {
    switch (t) {
        case TipoEvento::FalhaTemperaturaAlta: return "Falha: Temperatura Alta";
        case TipoEvento::FalhaEletrica:        return "Falha: Motor Elétrico";
        case TipoEvento::FalhaHidraulica:      return "Falha: Hidráulica";
        case TipoEvento::ComandoRearme:        return "Comando de Rearme";
        case TipoEvento::Outro:                return "Outro Evento";
        default:                               return "Evento Desconhecido";
    }
}
