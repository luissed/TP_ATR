#ifndef CORE_TIPOS_HPP
#define CORE_TIPOS_HPP

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

struct EventosFalhasCaminhao {
    bool ev_falha_eletrica;    // evento de falha elétrica
    bool ev_falha_hidraulica;  // evento de falha hidráulica
    bool ev_sobretemperatura;  // evento de temperatura acima do limite
};

struct InfoCaminhao {
    int id_caminhao;
    SensoresCaminhao sensores;
    AtuadoresCaminhao atuadores;
    EstadosCaminhao  estados;
    ComandosCaminhao comandos;
};

#endif