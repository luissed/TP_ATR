#pragma once

#include <vector>
#include <memory>
#include <cstddef>
#include "Caminhao.hpp"

// classe que representa a simulacao e a gestao da mina
// funciona como backend das tarefas de cima
// cuida de criar caminhoes iniciar e parar as tarefas internas e injetar falhas
// tambem oferece um mapa simples da posicao dos caminhoes e permite mudar rota e setpoint
// a ideia eh que uma interface grafica futura use so estes metodos publicos para controlar tudo
class SimulacaoMina {
public:
    // construtor pode opcionalmente criar numCaminhoes logo no inicio
    // usa um buffer com tamanho capacidadeBufferPadrao
    // se numCaminhoes for zero os caminhoes podem ser criados depois com criarNovoCaminhao
    SimulacaoMina(int numCaminhoes = 0, std::size_t capacidadeBufferPadrao = 200);

    ~SimulacaoMina();

    // controle global de execucao da simulacao da mina

    // inicia as tarefas de todos os caminhoes que existem
    void iniciar();

    // pede para todos os caminhoes pararem e faz join nas threads
    void parar();

    // parte da simulacao da mina que cria caminhoes e injeta falhas

    // cria um novo caminhao na mina
    // se capacidadeBuffer for zero usa o valor padrao interno
    // retorna um id logico do caminhao compativel com getId
    // se a simulacao ja estiver rodando o novo caminhao entra com as tarefas internas ja iniciadas
    int criarNovoCaminhao(std::size_t capacidadeBuffer = 0);

    // acessa caminhao pelo id logico de um ate n
    // lanca std::out_of_range se nao encontrar
    Caminhao& getCaminhaoPorId(int id);
    const Caminhao& getCaminhaoPorId(int id) const;

    // metodos de atalho para a interface de simulacao
    // servem para injetar falhas em um caminhao especifico
    void injetarFalhaTemperatura(int idCaminhao);
    void injetarFalhaEletrica(int idCaminhao);
    void injetarFalhaHidraulica(int idCaminhao);

    // parte de gestao da mina ligada ao mapa e a alteracao de rotas

    // define uma rota com ponto inicial e final para um caminhao especifico
    // funciona como a ordem que a gestao da mina envia para a tarefa de planejamento de rota
    void definirRotaCaminhao(int idCaminhao,
                             int x_inicial, int y_inicial,
                             int x_destino, int y_destino);

    // imprime em texto um mapa simples com um snapshot do ultimo estado de todos os caminhoes
    // util para teste enquanto a interface grafica nao existe
    void imprimirMapaTexto() const;

    // executa um loop de mapa em tempo quase real por um certo numero de segundos chamando imprimirMapaTexto
    // funciona como um stub da tarefa de gestao da mina
    void rodarPorSegundos(int segundos);

    // acesso direto por indice de zero ate n menos um util para testes antigos e para o main atual
    Caminhao& getCaminhao(std::size_t indice);
    const Caminhao& getCaminhao(std::size_t indice) const;

    std::size_t quantidadeCaminhoes() const;

private:
    std::vector<std::unique_ptr<Caminhao>> caminhoes_;
    std::size_t capacidadeBufferPadrao_;
    bool rodando_;
};