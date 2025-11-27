#ifndef PROCESSO_CAMINHAO_HPP
#define PROCESSO_CAMINHAO_HPP

#include <thread>
#include <atomic>
#include <vector>
#include "core/tipos.hpp"
#include "core/buffer.hpp"

class Caminhao {
private:
    int id;
    std::atomic<bool> rodando;
    
    // Buffers para comunicação INTERNA entre tarefas
    BufferCircular<InfoCaminhao> buffer_sensores_tratados;
    BufferCircular<EventosFalhasCaminhao> buffer_eventos_falhas;
    
    // Tarefas (threads) conforme diagrama
    std::thread tarefa_tratamento_sensores;
    std::thread tarefa_logica_comando;
    std::thread tarefa_monitoramento_falhas;
    std::thread tarefa_controle_navegacao;
    std::thread tarefa_coletor_dados;
    std::thread tarefa_interface_local;

public:
    Caminhao(int caminhao_id);
    ~Caminhao();
    
    void iniciar();
    void parar();
    void aguardar();

private:
    // Tarefas do diagrama
    void executar_tratamento_sensores();
    void executar_logica_comando();
    void executar_monitoramento_falhas();
    void executar_controle_navegacao();
    void executar_coletor_dados();
    void executar_interface_local();
};

class ProcessoCaminhao {
private:
    std::vector<std::unique_ptr<Caminhao>> caminhoes;
    std::atomic<bool> rodando;

public:
    ProcessoCaminhao(int num_caminhoes);
    void executar();
    void parar();
};

#endif