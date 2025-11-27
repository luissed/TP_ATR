<<<<<<< Updated upstream
#include "caminhao/caminhao.hpp"
#include "caminhao/contexto.hpp"
#include "core/buffer.hpp"
=======
// caminhao/caminhao.cpp
#include "caminhao/caminhao.hpp"
>>>>>>> Stashed changes
#include "caminhao/tratamento_sensores.hpp"
#include "caminhao/logica_comando.hpp"
#include "caminhao/monitoramento_falhas.hpp"
#include "caminhao/controle_navegacao.hpp"
#include "caminhao/coletor_dados.hpp"
#include "caminhao/interface_local.hpp"
<<<<<<< Updated upstream
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
=======

#include <iostream>
#include <memory> // Para std::make_unique

CaminhaoRuntime criar_caminhao(int id) {
    CaminhaoRuntime cr;
    
    // Inicializa o Contexto
    cr.ctx.info.id_caminhao = id;
    cr.ctx.rodando = true;
    cr.ctx.buffer.in = 0;
    cr.ctx.buffer.out = 0;
    cr.ctx.buffer.count = 0;
    
    // As estruturas InfoCaminhao e Eventos são copiadas para o contexto,
    // mas o acesso a elas será via ponteiro.
    CaminhaoContext* ctx_ptr = &cr.ctx; 

    // Cria as threads de cada tarefa, passando o ponteiro para o contexto compartilhado
    // Usamos std::ref(cr.ctx) se as threads manipulassem o objeto diretamente, 
    // mas a passagem por ponteiro (ctx_ptr) é mais explícita aqui.

    cr.th_tratamento = std::thread(tarefa_tratamento_sensores, ctx_ptr);
    cr.th_logica = std::thread(tarefa_logica_comando, ctx_ptr);
    cr.th_monitoramento = std::thread(tarefa_monitoramento_falhas, ctx_ptr);
    cr.th_controle = std::thread(tarefa_controle_navegacao, ctx_ptr);
    cr.th_coletor = std::thread(tarefa_coletor_dados, ctx_ptr);
    cr.th_interface = std::thread(tarefa_interface_local, ctx_ptr);
    
    std::cout << "Maestro: Caminhão " << id << " inicializado com " 
              << std::thread::hardware_concurrency() << " cores lógicos." << std::endl;

    return cr;
>>>>>>> Stashed changes
}

void parar_caminhao(CaminhaoRuntime& caminhao) {
    caminhao.ctx.rodando = false;
<<<<<<< Updated upstream
=======
    // (Em sistemas mais complexos, seria necessário notificar Condition Variables para 'acordar' threads adormecidas)
>>>>>>> Stashed changes
}

void join_caminhao(CaminhaoRuntime& caminhao) {
    if (caminhao.th_tratamento.joinable()) caminhao.th_tratamento.join();
    if (caminhao.th_logica.joinable()) caminhao.th_logica.join();
    if (caminhao.th_monitoramento.joinable()) caminhao.th_monitoramento.join();
    if (caminhao.th_controle.joinable()) caminhao.th_controle.join();
    if (caminhao.th_coletor.joinable()) caminhao.th_coletor.join();
    if (caminhao.th_interface.joinable()) caminhao.th_interface.join();
<<<<<<< Updated upstream
=======
    
    std::cout << "Maestro: Caminhão " << caminhao.ctx.info.id_caminhao << " encerrado com sucesso." << std::endl;
}

// ----------------------------------------------------------------
// Exemplo de main (Você deve adaptar a lógica de parada)
int main() {
    CaminhaoRuntime c1 = criar_caminhao(1);

    // Simula a execução do sistema por 10 segundos
    std::this_thread::sleep_for(std::chrono::seconds(10)); 

    parar_caminhao(c1);
    join_caminhao(c1);

    return 0;
>>>>>>> Stashed changes
}