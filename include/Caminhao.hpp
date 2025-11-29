#pragma once

#include <thread>
#include <atomic>
#include <mutex>
#include <cstddef>
#include <fstream>
#include <string>
#include <random>

#include "Tipos.hpp"
#include "BufferCircular.hpp"
#include "FilaEventos.hpp"

// forward pra declarar amizade entre as classes
class SimulacaoMina;

// classe que representa um caminhao da mina
class Caminhao {
    // simulacaomina pode chamar simularPassoFisico direto
    friend class SimulacaoMina;

public:
    Caminhao(int id, std::size_t capacidadeBuffer = 100);
    ~Caminhao();

    // inicia todas as tarefas, threads internas
    void iniciar();

    // pede para todas as tarefas encerrarem e da join
    void parar();

    int getId() const;

    // leitura simples para outras camadas, simulacaomina e afins
    EstadoCaminhao lerEstadoLogico() const;
    bool lerUltimoRegistro(RegistroBuffer& out) const;

    // interface local simulada, comandos do operador para modo e rearme
    void comandarAutomatico();
    void comandarManual();
    void comandarRearme();

    // backend do modo manual, comandos continuos no nivel de sinal
    // interface grafica futura tipo pygame vai chamar isso
    void setComandoAcelerar(bool ativo);
    void setComandoDireita(bool ativo);
    void setComandoEsquerda(bool ativo);

    // funcoes de teste pra injetar falhas na simulacao fisica via sensores
    // tarefa de monitoramento de falhas converte isso em eventos
    void injetarFalhaTemperaturaAlta();
    void injetarFalhaEletrica();
    void injetarFalhaHidraulica();

    // definir rota de x_inicial e y_inicial para x_destino e y_destino
    // chamar antes de iniciar a simulacao ou entre cenarios
    void definirRota(int x_inicial, int y_inicial, int x_destino, int y_destino);

private:
    // tarefas internas, cada uma roda em uma thread

    // aqui nao tem mais dinamica nem ruido, so le sensores brutos
    // filtra e alimenta o buffer
    void tarefaTratamentoSensores();

    void tarefaLogicaComando();       // decide e_defeito, e_automatico e estado logico
    void tarefaMonitoramentoFalhas(); // monitora temperatura e falhas e posta eventos
    void tarefaControleNavegacao();   // controla atuadores em auto ou manual sem dinamica pesada
    void tarefaPlanejamentoRota();    // define setpoints em funcao da rota
    void tarefaColetorDados();        // le do buffer e registra no terminal e em arquivo csv

    // simulacao fisica usada so por simulacaomina
    //
    // atualiza a dinamica fisica, posicao velocidade e temperatura a partir dos atuadores
    // soma ruido de media nula e gera sensores brutos ja com falhas aplicadas
    void simularPassoFisico(double dt, std::mt19937& rng);

    // identificacao
    int id_;

    // infraestrutura interna
    BufferCircular buffer_;
    FilaEventos filaEventos_;

    // comandos do operador
    mutable std::mutex mtxComandos_;
    ComandosCaminhao comandos_;

    // estado logico global do caminhao, maquina de estados
    mutable std::mutex mtxEstadoLogico_;
    EstadoCaminhao estadoLogico_;
    double tempoNoEstado_s_; // tempo acumulado no estado atual

    // estados
    mutable std::mutex mtxEstados_;
    EstadosCaminhao estados_;

    // estado fisico real
    mutable std::mutex mtxFisico_;
    double fis_pos_x_;   // posicao x em metros
    double fis_pos_y_;   // posicao y em metros
    double fis_vel_;     // velocidade escalar em m por s
    double fis_ang_deg_; // direcao da frente em graus, zero para leste
    double fis_temp_C_;  // temperatura do motor em graus celsius

    // sensores brutos gerados pela simulacaomina a partir da fisica com ruido
    mutable std::mutex mtxSensoresBrutos_;
    SensoresCaminhao sensoresBrutos_;

    // flags de falha fisica ou simulada, usadas pra forcar sensores
    std::atomic<bool> fis_forcarFalhaTemp_;
    std::atomic<bool> fis_forcarFalhaElec_;
    std::atomic<bool> fis_forcarFalhaHid_;

    // atuadores atuais
    mutable std::mutex mtxAtuadores_;
    AtuadoresCaminhao atuadores_;

    // setpoints, planejamento de rota
    mutable std::mutex mtxSetpoints_;
    SetpointsCaminhao setpoints_;

    // rota atual, origem e destino definidos externamente pela gestao da mina ou main
    mutable std::mutex mtxRota_;
    bool rota_definida_;
    int rota_origem_x_;
    int rota_origem_y_;
    int rota_destino_x_;
    int rota_destino_y_;

    // log em arquivo
    std::ofstream arquivoLog_;
    mutable std::mutex mtxLog_;

    // controle de threads internas
    std::atomic<bool> rodando_;
    std::thread thTratamentoSensores_;
    std::thread thLogicaComando_;
    std::thread thMonitoramentoFalhas_;
    std::thread thControleNavegacao_;
    std::thread thPlanejamentoRota_;
    std::thread thColetorDados_;
};