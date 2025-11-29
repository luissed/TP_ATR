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
#include "MqttInterface.hpp"
// Classe que representa um caminhão da mina
class Caminhao {
public:
    Caminhao(int id, std::size_t capacidadeBuffer = 100);
    ~Caminhao();

    // Inicia todas as tarefas (threads internas).
    void iniciar();

    // Pede para todas as tarefas encerrarem e dá join.
    void parar();

    int getId() const;

    // Leitura simples para outras camadas (SimulacaoMina, etc.)
    EstadoCaminhao lerEstadoLogico() const;
    bool lerUltimoRegistro(RegistroBuffer& out) const;

    // "Interface Local" simulada: comandos do operador (modo / rearme).
    void comandarAutomatico();
    void comandarManual();
    void comandarRearme();

    // Backend do modo MANUAL: comandos contínuos (nível de sinal).
    // A interface gráfica futura (pygame) vai chamar isso.
    void setComandoAcelerar(bool ativo);
    void setComandoDireita(bool ativo);
    void setComandoEsquerda(bool ativo);

    // Funções de TESTE para injetar falhas NA SIMULAÇÃO FÍSICA (sensores).
    // A tarefa de Monitoramento de Falhas é quem converte isso em eventos.
    void injetarFalhaTemperaturaAlta();
    void injetarFalhaEletrica();
    void injetarFalhaHidraulica();

    // Definir rota: de (x_inicial, y_inicial) para (x_destino, y_destino).
    // Deve ser chamado antes de iniciar a simulação OU entre cenários.
    void definirRota(int x_inicial, int y_inicial, int x_destino, int y_destino);

private:
void processarMensagemMqtt(const std::string& topico, const std::string& payload);
    std::unique_ptr<MqttInterface> mqtt_; // Ponteiro inteligente para a interface
    // ---------- Tarefas internas (cada uma roda em uma thread) ----------
    void tarefaTratamentoSensores();      // lê estado físico, filtra ruído e alimenta o buffer
    void tarefaLogicaComando();           // decide e_defeito, e_automatico e estado lógico
    void tarefaMonitoramentoFalhas();     // monitora temperatura/falhas e posta eventos
    void tarefaControleNavegacao();       // controla velocidade/ângulo (modo auto/manual) + simulação 2D
    void tarefaPlanejamentoRota();        // define setpoints em função da rota
    void tarefaColetorDados();            // lê do buffer e loga (terminal + arquivo CSV)

    // ---------- Identificação ----------
    int id_;

    // ---------- Infraestrutura interna ----------
    BufferCircular buffer_;
    FilaEventos    filaEventos_;

    // Comandos do operador
    mutable std::mutex mtxComandos_;
    ComandosCaminhao comandos_;

    // Estado lógico global do caminhão (máquina de estados)
    mutable std::mutex mtxEstadoLogico_;
    EstadoCaminhao estadoLogico_;
    double tempoNoEstado_s_; // tempo acumulado no estado atual

    // Estados
    mutable std::mutex mtxEstados_;
    EstadosCaminhao estados_;

    // Estado físico real
    mutable std::mutex mtxFisico_;
    double fis_pos_x_;   // posição x em metros
    double fis_pos_y_;   // posição y em metros
    double fis_vel_;     // velocidade escalar em m/s
    double fis_ang_deg_; // direção da frente, graus (0 = leste)
    double fis_temp_C_;  // temperatura do motor

    // Flags de falha física/simulada, usadas para forçar sensores
    std::atomic<bool> fis_forcarFalhaTemp_;
    std::atomic<bool> fis_forcarFalhaElec_;
    std::atomic<bool> fis_forcarFalhaHid_;

    // Atuadores atuais
    mutable std::mutex mtxAtuadores_;
    AtuadoresCaminhao atuadores_;

    // Setpoints (Planejamento de Rota)
    mutable std::mutex mtxSetpoints_;
    SetpointsCaminhao setpoints_;

    // Rota atual (origem/destino) definida externamente (main / gestão da mina)
    mutable std::mutex mtxRota_;
    bool rota_definida_;
    int  rota_origem_x_;
    int  rota_origem_y_;
    int  rota_destino_x_;
    int  rota_destino_y_;

    // ---------- Log em arquivo ----------
    std::ofstream       arquivoLog_;
    mutable std::mutex  mtxLog_;

    // ---------- Controle de threads ----------
    std::atomic<bool> rodando_;
    std::thread thTratamentoSensores_;
    std::thread thLogicaComando_;
    std::thread thMonitoramentoFalhas_;
    std::thread thControleNavegacao_;
    std::thread thPlanejamentoRota_;
    std::thread thColetorDados_;
};
