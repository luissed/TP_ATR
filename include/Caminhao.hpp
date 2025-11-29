#pragma once

#include <thread>
#include <atomic>
#include <mutex>
#include <cstddef>
#include <fstream>
#include <string>

#include "Tipos.hpp"
#include "BufferCircular.hpp"
#include "FilaEventos.hpp"

class Caminhao {
public:
    Caminhao(int id, std::size_t capacidadeBuffer = 100);
    ~Caminhao();

    void iniciar();

    void parar();

    int getId() const;

    EstadoCaminhao lerEstadoLogico() const;
    bool lerUltimoRegistro(RegistroBuffer& out) const;

    void comandarAutomatico();
    void comandarManual();
    void comandarRearme();

    void definirRota(int x_inicial, int y_inicial, int x_destino, int y_destino);

private:
    void tarefaTratamentoSensores();
    void tarefaLogicaComando();
    void tarefaMonitoramentoFalhas();
    void tarefaControleNavegacao();
    void tarefaPlanejamentoRota();
    void tarefaColetorDados();

    int id_;

    BufferCircular buffer_;
    FilaEventos filaEventos_;

    mutable std::mutex mtxComandos_;
    ComandosCaminhao comandos_;

    mutable std::mutex mtxEstadoLogico_;
    EstadoCaminhao estadoLogico_;
    double tempoNoEstado_s_;

    mutable std::mutex mtxEstados_;
    EstadosCaminhao estados_;

    mutable std::mutex mtxFisico_;
    double fis_pos_x_;
    double fis_pos_y_;
    double fis_vel_;
    double fis_ang_deg_;
    double fis_temp_C_;

    mutable std::mutex mtxAtuadores_;
    AtuadoresCaminhao atuadores_;

    mutable std::mutex mtxSetpoints_;
    SetpointsCaminhao setpoints_;

    mutable std::mutex mtxRota_;
    bool rota_definida_;
    int rota_origem_x_;
    int rota_origem_y_;
    int rota_destino_x_;
    int rota_destino_y_;

    std::ofstream arquivoLog_;
    mutable std::mutex mtxLog_;

    std::atomic<bool> rodando_;
    std::thread thTratamentoSensores_;
    std::thread thLogicaComando_;
    std::thread thMonitoramentoFalhas_;
    std::thread thControleNavegacao_;
    std::thread thPlanejamentoRota_;
    std::thread thColetorDados_;
};
