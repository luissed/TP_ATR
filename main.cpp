#include <iostream>

#include "SimulacaoMina.hpp"

int main() {
    std::cout << "Interface de Simulacao da Mina\n";

    // Backend da mina:
    //
    // - capacidadeBufferPadrao = 200;
    // - numCaminhoes inicial = 0 -> vamos criar via criarNovoCaminhao().
    SimulacaoMina mina(0, 200);

    // SIMULAÇÃO DA MINA: criação de 1 caminhão via "interface de simulação"
    int idCaminhao1 = mina.criarNovoCaminhao(); // id = 1

    // GESTÃO DA MINA: define a rota do caminhão 1: (0,0) -> (100,50)
    // (isso é mais pra testar AUTO antes do MANUAL)
    mina.definirRotaCaminhao(idCaminhao1, 0, 0, 100, 50);

    // Inicia as tarefas internas de todos os caminhões
    mina.iniciar();

    // Pega uma referência para o caminhão 1 (para mandar comandos)
    Caminhao& cam1 = mina.getCaminhaoPorId(idCaminhao1);

    // -------------------------------------------------------------------------
    // FASE 1: AUTO sem falhas (só pra ver o controlador funcionando)
    // -------------------------------------------------------------------------
    std::cout << "\n>>> [Interface] Fase 1: caminhão em modo automático (sem falhas) por 5s\n";
    cam1.comandarAutomatico();
    mina.rodarPorSegundos(5);

    // -------------------------------------------------------------------------
    // FASE 2: MODO MANUAL - anda reto (acelerando) por 5 s
    // -------------------------------------------------------------------------
    std::cout << "\n>>> [Interface] Fase 2: caminhão em modo manual, acelerando reto por 5s\n";
    cam1.comandarManual();          // troca para manual
    cam1.setComandoAcelerar(true);  // segurando "acelera"
    cam1.setComandoDireita(false);
    cam1.setComandoEsquerda(false);
    mina.rodarPorSegundos(5);

    // -------------------------------------------------------------------------
    // FASE 3: MODO MANUAL - continua acelerando e vira para a direita
    // -------------------------------------------------------------------------
    std::cout << "\n>>> [Interface] Fase 3: manual, acelerando e virando para a direita por 5s\n";
    cam1.setComandoAcelerar(true);
    cam1.setComandoDireita(true);   // como se o operador segurasse a tecla de virar à direita
    cam1.setComandoEsquerda(false);
    mina.rodarPorSegundos(5);

    // -------------------------------------------------------------------------
    // FASE 4: MODO MANUAL - solta aceleração, direção neutra
    // -------------------------------------------------------------------------
    std::cout << "\n>>> [Interface] Fase 4: manual, soltando acelerador (sem girar o volante) por 5s\n";
    cam1.setComandoAcelerar(false);
    cam1.setComandoDireita(false);
    cam1.setComandoEsquerda(false);
    mina.rodarPorSegundos(5);

    // Encerra simulação
    mina.parar();

    std::cout << "Simulacao da Mina encerrada\n";
    return 0;
}
