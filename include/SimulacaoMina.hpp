#pragma once

#include <vector>
#include <memory>
#include <cstddef>
#include <mutex>
#include <atomic>
#include <thread>
#include "Caminhao.hpp"
#include "MqttInterface.hpp" 

class SimulacaoMina {
public:
    SimulacaoMina(int numCaminhoes = 0, std::size_t capacidadeBufferPadrao = 200);

    ~SimulacaoMina();

    void iniciar();
    void parar();

    int criarNovoCaminhao(std::size_t capacidadeBuffer = 0);

    Caminhao& getCaminhaoPorId(int id);
    const Caminhao& getCaminhaoPorId(int id) const;

    void injetarFalhaTemperatura(int idCaminhao);
    void injetarFalhaEletrica(int idCaminhao);
    void injetarFalhaHidraulica(int idCaminhao);

    void definirRotaCaminhao(int idCaminhao,
                             int x_inicial, int y_inicial,
                             int x_destino, int y_destino);

    void imprimirMapaTexto() const;
    void rodarPorSegundos(int segundos);

    Caminhao& getCaminhao(std::size_t indice);
    const Caminhao& getCaminhao(std::size_t indice) const;

    std::size_t quantidadeCaminhoes() const;

private:
    void processarMensagemCentral(const std::string& topico, const std::string& payload);
    void tarefaMonitoramentoSeguranca();

    std::vector<std::unique_ptr<Caminhao>> caminhoes_;
    std::size_t capacidadeBufferPadrao_;
    
    std::atomic<bool> rodando_; 
    mutable std::mutex mtxCaminhoes_; 
    std::thread thSeguranca_; 
    
    std::unique_ptr<MqttInterface> mqtt_;
};