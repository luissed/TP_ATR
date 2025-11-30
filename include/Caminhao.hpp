// include/Caminhao.hpp
#pragma once

#include <thread>
#include <atomic>
#include <mutex>
#include <cstddef>
#include <fstream>
#include <string>
#include <random>
#include <memory> 

#include "Tipos.hpp"
#include "BufferCircular.hpp"
#include "FilaEventos.hpp"
#include "MqttInterface.hpp" // Necessário para comunicação

class Caminhao {
    friend class SimulacaoMina;

public:
    Caminhao(int id, std::size_t capacidadeBuffer = 100);
    ~Caminhao();

    void iniciar();
    void parar();
    int getId() const;

    EstadoCaminhao lerEstadoLogico() const;
    bool lerUltimoRegistro(RegistroBuffer& out) const;

    // Comandos
    void comandarAutomatico();
    void comandarManual();
    void comandarRearme();
    void setComandoAcelerar(bool ativo);
    void setComandoDireita(bool ativo);
    void setComandoEsquerda(bool ativo);
    
    // NOVO: Segurança (Sistema Anticolisão chama isso)
    void setReducaoSeguranca(bool ativar);

    // Injeção de Falhas
    void injetarFalhaTemperaturaAlta();
    void injetarFalhaEletrica();
    void injetarFalhaHidraulica();

    void definirRota(int x_inicial, int y_inicial, int x_destino, int y_destino);

private:
    // MQTT
    void processarMensagemMqtt(const std::string& topico, const std::string& payload);
    std::unique_ptr<MqttInterface> mqtt_;

    // Tarefas
    void comandarParadaEmergencia();
    void tarefaTratamentoSensores();
    void tarefaLogicaComando();
    void tarefaMonitoramentoFalhas();
    void tarefaControleNavegacao();
    void tarefaPlanejamentoRota();
    void tarefaColetorDados();

    // Identificação e Infra
    int id_;
    BufferCircular buffer_;
    FilaEventos filaEventos_;

    // Estados Internos (Protegidos por Mutex)
    mutable std::mutex mtxComandos_;
    ComandosCaminhao comandos_;

    mutable std::mutex mtxEstadoLogico_;
    EstadoCaminhao estadoLogico_;
    double tempoNoEstado_s_;

    mutable std::mutex mtxEstados_;
    EstadosCaminhao estados_;

    mutable std::mutex mtxFisico_;
    double fis_pos_x_, fis_pos_y_, fis_vel_, fis_ang_deg_, fis_temp_C_;

    std::atomic<bool> fis_forcarFalhaTemp_;
    std::atomic<bool> fis_forcarFalhaElec_;
    std::atomic<bool> fis_forcarFalhaHid_;
    
    // NOVO: Flag para o sistema anticolisão reduzir a velocidade
    std::atomic<bool> em_reducao_seguranca_{false};

    mutable std::mutex mtxAtuadores_;
    AtuadoresCaminhao atuadores_;

    mutable std::mutex mtxSetpoints_;
    SetpointsCaminhao setpoints_;

    mutable std::mutex mtxRota_;
    bool rota_definida_;
    int rota_origem_x_, rota_origem_y_, rota_destino_x_, rota_destino_y_;

    // Log
    std::ofstream arquivoLog_;
    mutable std::mutex mtxLog_;

    // Threads
    std::atomic<bool> rodando_;
    std::thread thTratamentoSensores_;
    std::thread thLogicaComando_;
    std::thread thMonitoramentoFalhas_;
    std::thread thControleNavegacao_;
    std::thread thPlanejamentoRota_;
    std::thread thColetorDados_;
};