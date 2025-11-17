#ifndef CORE_TIPOS_HPP
#define CORE_TIPOS_HPP

struct Sensores {
    int i_posicao_x;
    int i_posicao_y;
    int i_angulo_x;
    int i_temperatura;
    bool i_falha_eletrica;
    bool i_falha_hidraulica;
};

struct Atuadores {
    int o_aceleracao;
    int o_direcao;
};

struct Estados {
    bool e_defeito;
    bool e_automatico;
};

struct Comandos {
    bool c_automatico;
    bool c_man;
    bool c_rearme;
    bool c_acelera;
    bool c_direita;
    bool c_esquerda;
};

struct EstadoCaminhao {
    int id_caminhao;
    Sensores sensores;
    Atuadores atuadores;
    Estados estados;
    Comandos comandos;
};

#endif