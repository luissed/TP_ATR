#include "caminhao/caminhao.hpp"
#include "caminhao/contexto.hpp"
#include "core/buffer.hpp"
#include "caminhao/tratamento_sensores.hpp"
#include "caminhao/logica_comando.hpp"
#include "caminhao/monitoramento_falhas.hpp"
#include "caminhao/controle_navegacao.hpp"
#include "caminhao/coletor_dados.hpp"
#include "caminhao/interface_local.hpp"
#include <iostream>

BufferCircular<InfoCaminhao> buffer_sensores;

CaminhaoRuntime criar_caminhao(int id) {
    CaminhaoRuntime runtime(id);
    
    std::cout << "Iniciando caminhao " << id << "..." << std::endl;
    
    runtime.th_tratamento = std::thread(tarefa_tratamento_sensores, &runtime.ctx);
    runtime.th_logica = std::thread(tarefa_logica_comando, &runtime.ctx);
    runtime.th_monitoramento = std::thread(tarefa_monitoramento_falhas, &runtime.ctx);
    runtime.th_controle = std::thread(tarefa_controle_navegacao, &runtime.ctx);
    runtime.th_coletor = std::thread(tarefa_coletor_dados, &runtime.ctx);
    runtime.th_interface = std::thread(tarefa_interface_local, &runtime.ctx);
    
    return runtime;
}

void parar_caminhao(CaminhaoRuntime& caminhao) {
    caminhao.ctx.rodando = false;
}

void join_caminhao(CaminhaoRuntime& caminhao) {
    if (caminhao.th_tratamento.joinable()) caminhao.th_tratamento.join();
    if (caminhao.th_logica.joinable()) caminhao.th_logica.join();
    if (caminhao.th_monitoramento.joinable()) caminhao.th_monitoramento.join();
    if (caminhao.th_controle.joinable()) caminhao.th_controle.join();
    if (caminhao.th_coletor.joinable()) caminhao.th_coletor.join();
    if (caminhao.th_interface.joinable()) caminhao.th_interface.join();
}