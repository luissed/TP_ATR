#pragma once

#include <vector>
#include <memory>
#include <cstddef>
#include "Caminhao.hpp"

// Classe de alto nível que representa a simulação/gestão da mina.
//
// Ela é o BACKEND das tarefas:
//
// - Simulação da Mina:
//     * cria caminhões novos;
//     * inicia/para todas as tarefas internas dos caminhões;
//     * permite injetar falhas em qualquer caminhão (para a interface de simulação).
//
// - Gestão da Mina:
//     * fornece um "mapa" (por enquanto textual) da posição de todos os caminhões;
//     * permite alterar o setpoint/rota de qualquer caminhão.
//
// Mais pra frente, uma interface gráfica (ou outra camada) vai chamar
// EXCLUSIVAMENTE estes métodos públicos aqui para controlar o sistema.
class SimulacaoMina {
public:
    // Construtor: opcionalmente já cria 'numCaminhoes' caminhões com
    // buffer de tamanho 'capacidadeBufferPadrao'.
    // Se numCaminhoes == 0, você pode criar caminhões depois com criarNovoCaminhao().
    SimulacaoMina(int numCaminhoes = 0, std::size_t capacidadeBufferPadrao = 200);

    ~SimulacaoMina();

    // -------------------------------------------------------------------------
    // Controle global de execução (Simulação da Mina)
    // -------------------------------------------------------------------------

    // Inicia todas as tarefas de todos os caminhões existentes.
    void iniciar();

    // Pede para todos os caminhões pararem e dá join nas threads.
    void parar();

    // -------------------------------------------------------------------------
    // Simulação da Mina: criação de caminhões e injeção de falhas
    // -------------------------------------------------------------------------

    // Cria um novo caminhão na mina.
    //
    // - capacidadeBuffer == 0 -> usa capacidadeBufferPadrao interna.
    // - Retorna o ID lógico do caminhão (1,2,3,...), igual a Caminhao::getId().
    //
    // Se a simulação já estiver rodando (iniciar() já foi chamado),
    // o caminhão novo já tem suas tarefas internas iniciadas automaticamente.
    int criarNovoCaminhao(std::size_t capacidadeBuffer = 0);

    // Acessa caminhão por ID lógico (1..N).
    // Lança std::out_of_range se não encontrar.
    Caminhao& getCaminhaoPorId(int id);
    const Caminhao& getCaminhaoPorId(int id) const;

    // Métodos de conveniência para a INTERFACE DE SIMULAÇÃO:
    // injetam falhas em um caminhão específico.
    void injetarFalhaTemperatura(int idCaminhao);
    void injetarFalhaEletrica(int idCaminhao);
    void injetarFalhaHidraulica(int idCaminhao);

    // -------------------------------------------------------------------------
    // Gestão da Mina: mapa e alteração de rotas
    // -------------------------------------------------------------------------

    // Define uma rota (ponto inicial e final) para um caminhão específico.
    // Esta é a "ordem" que a tarefa de Gestão da Mina envia para Planejamento de Rota.
    void definirRotaCaminhao(int idCaminhao,
                             int x_inicial, int y_inicial,
                             int x_destino, int y_destino);

    // Imprime, em texto, um "mapa" com um snapshot do último estado
    // de todos os caminhões (para testes enquanto a GUI não existe).
    void imprimirMapaTexto() const;

    // Loop de "mapa em tempo real" por 'segundos', chamando imprimirMapaTexto()
    // a cada segundo. Isto é um stub da tarefa de Gestão da Mina.
    void rodarPorSegundos(int segundos);

    // -------------------------------------------------------------------------
    // Acesso bruto por índice (0..N-1) - útil para testes antigos/main atual
    // -------------------------------------------------------------------------

    Caminhao& getCaminhao(std::size_t indice);
    const Caminhao& getCaminhao(std::size_t indice) const;

    std::size_t quantidadeCaminhoes() const;

private:
    std::vector<std::unique_ptr<Caminhao>> caminhoes_;
    std::size_t capacidadeBufferPadrao_;
    bool rodando_;
};
