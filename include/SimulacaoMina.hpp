#pragma once

#include <vector>
#include <memory>
#include <cstddef>
#include <mutex>
#include <atomic>
#include <thread>
#include "Caminhao.hpp"
#include "MqttInterface.hpp" // Necessário para o membro mqtt_

// Classe de alto nível que representa a simulação/gestão da mina.
class SimulacaoMina {
public:
    SimulacaoMina(int numCaminhoes = 0, std::size_t capacidadeBufferPadrao = 200);

    ~SimulacaoMina();

    // -------------------------------------------------------------------------
    // Controle global de execução
    // -------------------------------------------------------------------------
    void iniciar();
    void parar();

    // -------------------------------------------------------------------------
    // Simulação da Mina
    // -------------------------------------------------------------------------
    int criarNovoCaminhao(std::size_t capacidadeBuffer = 0);

    Caminhao& getCaminhaoPorId(int id);
    const Caminhao& getCaminhaoPorId(int id) const;

    void injetarFalhaTemperatura(int idCaminhao);
    void injetarFalhaEletrica(int idCaminhao);
    void injetarFalhaHidraulica(int idCaminhao);

    // -------------------------------------------------------------------------
    // Gestão da Mina
    // -------------------------------------------------------------------------
    void definirRotaCaminhao(int idCaminhao,
                             int x_inicial, int y_inicial,
                             int x_destino, int y_destino);

    void imprimirMapaTexto() const;
    void rodarPorSegundos(int segundos);

    // -------------------------------------------------------------------------
    // Acesso bruto
    // -------------------------------------------------------------------------
    Caminhao& getCaminhao(std::size_t indice);
    const Caminhao& getCaminhao(std::size_t indice) const;

    std::size_t quantidadeCaminhoes() const;

private:
    // Métodos auxiliares (privados) exigidos pelo .cpp
    void processarMensagemCentral(const std::string& topico, const std::string& payload);
    void tarefaMonitoramentoSeguranca();

    std::vector<std::unique_ptr<Caminhao>> caminhoes_;
    std::size_t capacidadeBufferPadrao_;
    
    // Variáveis de controle de concorrência e estado
    std::atomic<bool> rodando_;           // Alterado para atomic (usa compare_exchange_strong)
    mutable std::mutex mtxCaminhoes_;     // Mutable para permitir lock em métodos const
    std::thread thSeguranca_;             // Thread de monitoramento
    
    // Interface de comunicação
    std::unique_ptr<MqttInterface> mqtt_;
};