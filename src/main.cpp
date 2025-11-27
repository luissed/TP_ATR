#include <iostream>
#include <thread>
#include <chrono>

#include "../include/caminhao/caminhao.hpp"
#include "../include/caminhao/tratamento_sensores.hpp"

int main() {
    CaminhaoContext ctx{};
    ctx.rodando = true;
    ctx.info.id_caminhao = 1;

    ctx.info.sensores = SensoresCaminhao{0, 0, 0, 0, false, false};

    std::thread th_tratamento(tarefa_tratamento_sensores, &ctx);

    std::this_thread::sleep_for(std::chrono::seconds(2));

    ctx.rodando = false;
    th_tratamento.join();

    std::cout << "Tarefa Tratamento de Sensores encerrada." << std::endl;
    return 0;
}
