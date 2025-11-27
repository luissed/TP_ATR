#ifndef CORE_TIPOS_HPP
#define CORE_TIPOS_HPP

#include <mutex>
#include <condition_variable>

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
    bool ev_falha_eletrica;
    bool ev_falha_hidraulica;
    bool ev_sobretemperatura;
};

struct InfoCaminhao {
    int id_caminhao;
    SensoresCaminhao sensores;
    AtuadoresCaminhao atuadores;
    EstadosCaminhao  estados;
    ComandosCaminhao comandos;
};

// --- Buffer Circular (Estrutura de Dados do Produtor/Consumidor) ---
// Define as posições e ponteiros lógicos. A lógica de sincronização reside no Contexto.
struct BufferCircular {
    enum { SIZE = 200 };
    InfoCaminhao dados[SIZE]; // Armazena a informação do caminhão em um momento t
    int in;  // Posição de escrita (produtor)
    int out; // Posição de leitura (consumidor)
    int count; // Número de itens no buffer (para lógica de checagem)
};

// --- Contexto Compartilhado (CaminhaoContext) ---

// O contexto é o ponto de agregação de TODOS os recursos compartilhados, incluindo as primitivas
struct CaminhaoContext { 
    
    // Mutexes para exclusão mútua (proteção contra Race Condition)
    std::mutex mtx_info;        // Protege dados InfoCaminhao (atuadores/comandos)
    std::mutex mtx_eventos;     // Protege EventosFalhasCaminhao
    std::mutex mtx_buffer;      // Protege o BufferCircular (para leitura/escrita)

    // Variáveis de Condição (Coordenação entre Produtor/Consumidor)
    std::condition_variable cv_buffer_vazio;  // Produtor espera por um slot vazio
    std::condition_variable cv_buffer_cheio;  // Consumidor(es) esperam por um slot cheio
    
    // O Buffer Circular agora vive aqui, como o recurso compartilhado central
    BufferCircular buffer;

    InfoCaminhao info; 
    EventosFalhasCaminhao eventos;
    
    bool rodando; 
};

#endif