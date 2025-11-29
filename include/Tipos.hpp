#pragma once

#include <string>

// ----------------------- Tabela 1: Sensores e atuadores -----------------------

// Sensores de posição, orientação, temperatura e falhas.
// Nomes exatamente iguais aos definidos pelo professor.
struct SensoresCaminhao {
    int  i_posicao_x;        // posição no eixo x (m)
    int  i_posicao_y;        // posição no eixo y (m)
    int  i_angulo_x;         // direção da frente, em graus (0 = leste)
    int  i_temperatura;      // temperatura do motor (°C)
    bool i_falha_eletrica;   // true = falha elétrica presente
    bool i_falha_hidraulica; // true = falha hidráulica presente
};

// Atuadores (saídas) - nomes exatamente como na Tabela 1.
struct AtuadoresCaminhao {
    int o_aceleracao; // -100 .. 100 %
    int o_direcao;    // -180 .. 180 graus (ângulo absoluto)
};

// ----------------------- Tabela 2: Estados e comandos ------------------------

// Estados do caminhão (gerados pela Lógica de Comando).
struct EstadosCaminhao {
    bool e_defeito;    // 1:defeito, 0:sem defeito
    bool e_automatico; // 1:automático, 0:manual
};

// Comandos do operador (Interface Local).
struct ComandosCaminhao {
    bool c_automatico; // comando para modo automático
    bool c_man;        // comando para modo manual
    bool c_rearme;     // comando de rearme
    bool c_acelera;    // comando para acelerar (modo manual)
    bool c_direita;    // comando para girar para a direita
    bool c_esquerda;   // comando para girar para a esquerda
};

// ----------------------- Estado lógico de alto nível -------------------------

// Máquina de estados de alto nível do caminhão (genérica, sem “mina/usina”).
enum class EstadoCaminhao {
    Parado,
    EmMovimento,
    EmFalha
};

// Setpoints definidos pelo Planejamento de Rota.
struct SetpointsCaminhao {
    int sp_posicao_x; // posição alvo em x
    int sp_posicao_y; // posição alvo em y
    int sp_angulo_x;  // direção alvo em graus
};

// ----------------------- Registro no Buffer Circular -------------------------

// Snapshot completo do caminhão que é compartilhado entre as tarefas internas.
struct RegistroBuffer {
    double tempoSimulacao_s; // tempo da simulação em segundos
    int    id_caminhao;      // identificação do caminhão

    EstadoCaminhao   estado;    // estado lógico de alto nível
    SensoresCaminhao sensores;  // sensores TRATADOS (saída do Tratamento de Sensores)
    AtuadoresCaminhao atuadores;// atuadores atuais
    EstadosCaminhao  estados;   // estados Tabela 2 (e_defeito, e_automatico)
    ComandosCaminhao comandos;  // comandos Tabela 2
    SetpointsCaminhao setpoints;// setpoints atuais
};

// ----------------------------- Eventos ---------------------------------------

enum class TipoEvento {
    FalhaTemperaturaAlta,
    FalhaEletrica,
    FalhaHidraulica,
    ComandoRearme,
    Outro
};

struct Evento {
    TipoEvento tipo;
    std::string descricao;    // ex.: "Temperatura > 120 °C"
    double tempoSimulacao_s;  // quando o evento ocorreu
    int id_caminhao;          // caminhão associado
};

// Funções auxiliares para imprimir textos em logs.
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
