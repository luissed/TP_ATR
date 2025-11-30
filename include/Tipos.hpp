#pragma once

#include <string>

// tabela 1 sensores e atuadores

// sensores de posição orientação temperatura e falhas
// nomes iguais aos usados pelo professor
struct SensoresCaminhao {
    int i_posicao_x; // posição no eixo x em metros
    int i_posicao_y; // posição no eixo y em metros
    int i_angulo_x;  // direção da frente em graus com zero apontando para leste
    int i_temperatura; // temperatura do motor em graus celsius
    bool i_falha_eletrica;   // verdadeiro indica falha elétrica presente
    bool i_falha_hidraulica; // verdadeiro indica falha hidráulica presente
};

// atuadores de saída com a mesma nomenclatura da tabela do sistema
struct AtuadoresCaminhao {
    int o_aceleracao; // valor de aceleração em porcentagem de menos cem até cem
    int o_direcao;    // ângulo absoluto em graus de menos cento e oitenta até cento e oitenta
};

// tabela 2 estados e comandos

// estados do caminhão gerados pela lógica de comando
struct EstadosCaminhao {
    bool e_defeito;          // um indica defeito ativo zero indica sem defeito
    bool e_automatico;       // um indica modo automático zero indica modo manual
    bool e_bloqueio_rearme;  // um indica que é necessário REARME antes de aceitar AUTO
};

// comandos do operador na interface local
struct ComandosCaminhao {
    bool c_automatico; // comando para entrar em modo automático
    bool c_man;        // comando para entrar em modo manual
    bool c_rearme;     // comando de rearme de falha
    bool c_acelera;    // comando para acelerar em modo manual
    bool c_direita;    // comando para girar a direção para a direita
    bool c_esquerda;   // comando para girar a direção para a esquerda
};

// estado lógico de alto nível

// máquina de estados genérica do caminhão sem detalhes específicos de processo
enum class EstadoCaminhao {
    Parado,
    EmMovimento,
    EmFalha
};

// setpoints definidos pelo planejamento de rota
struct SetpointsCaminhao {
    int sp_posicao_x; // posição alvo no eixo x
    int sp_posicao_y; // posição alvo no eixo y
    int sp_angulo_x;  // direção alvo em graus
};

// registro principal usado no buffer circular

// snapshot do caminhão compartilhado entre as tarefas internas
struct RegistroBuffer {
    double tempoSimulacao_s; // tempo da simulação em segundos
    int id_caminhao;         // identificação lógica do caminhão

    EstadoCaminhao   estado;    // estado lógico de alto nível
    SensoresCaminhao sensores;  // sensores tratados vindos do tratamento de sensores
    AtuadoresCaminhao atuadores; // atuadores na situação atual
    EstadosCaminhao  estados;   // estados da tabela dois (e_defeito, e_automatico, e_bloqueio_rearme)
    ComandosCaminhao comandos;  // comandos da tabela dois
    SetpointsCaminhao setpoints;// setpoints em uso no instante
};

// eventos usados entre monitoramento de falhas lógica de comando e outros blocos

enum class TipoEvento {
    FalhaTemperaturaAlta,
    FalhaEletrica,
    FalhaHidraulica,
    ComandoRearme,
    Outro
};

struct Evento {
    TipoEvento   tipo;
    std::string  descricao;         // texto livre com um resumo do que aconteceu
    double       tempoSimulacao_s;  // tempo da simulação em que o evento foi gerado
    int          id_caminhao;       // identificador do caminhão associado ao evento
};

// funções auxiliares para montar texto de log a partir dos estados

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
