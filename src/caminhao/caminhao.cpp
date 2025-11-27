#include "../include/caminhao/caminhao.hpp"
#include "mqtt/mqtt_client.hpp"
#include <iostream>
#include <chrono>
#include <random>
#include <sstream>

Caminhao::Caminhao(int caminhao_id) : id(caminhao_id), rodando(true) {
    std::cout << "Criando Caminhão " << id << std::endl;
}

Caminhao::~Caminhao() {
    parar();
}

void Caminhao::iniciar() {
    std::cout << "Iniciando tarefas do Caminhão " << id << std::endl;
    
    tarefa_tratamento_sensores = std::thread(&Caminhao::executar_tratamento_sensores, this);
    tarefa_logica_comando = std::thread(&Caminhao::executar_logica_comando, this);
    tarefa_monitoramento_falhas = std::thread(&Caminhao::executar_monitoramento_falhas, this);
    tarefa_controle_navegacao = std::thread(&Caminhao::executar_controle_navegacao, this);
    tarefa_coletor_dados = std::thread(&Caminhao::executar_coletor_dados, this);
    tarefa_interface_local = std::thread(&Caminhao::executar_interface_local, this);
}

void Caminhao::parar() {
    rodando = false;
}

void Caminhao::aguardar() {
    if (tarefa_tratamento_sensores.joinable()) tarefa_tratamento_sensores.join();
    if (tarefa_logica_comando.joinable()) tarefa_logica_comando.join();
    if (tarefa_monitoramento_falhas.joinable()) tarefa_monitoramento_falhas.join();
    if (tarefa_controle_navegacao.joinable()) tarefa_controle_navegacao.join();
    if (tarefa_coletor_dados.joinable()) tarefa_coletor_dados.join();
    if (tarefa_interface_local.joinable()) tarefa_interface_local.join();
}

// TAREFA: Tratamento Sensores (com filtro média móvel)
void Caminhao::executar_tratamento_sensores() {
    MQTTClient mqtt;
    mqtt.conectar("localhost", 1883, "caminhao_" + std::to_string(id) + "_sensores");
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis_pos(0, 1000);
    std::uniform_int_distribution<> dis_temp(80, 120);
    
    // Filtro de média móvel (ordem M = 5)
    std::vector<int> historico_x, historico_y, historico_angulo;
    const int ordem_filtro = 5;
    
    while (rodando) {
        InfoCaminhao info;
        info.id_caminhao = id;
        
        // Gerar dados brutos com ruído
        int pos_x_bruto = dis_pos(gen);
        int pos_y_bruto = dis_pos(gen);
        int angulo_bruto = dis_pos(gen) % 360;
        int temp_bruto = dis_temp(gen);
        
        // Aplicar filtro de média móvel
        historico_x.push_back(pos_x_bruto);
        historico_y.push_back(pos_y_bruto);
        historico_angulo.push_back(angulo_bruto);
        
        if (historico_x.size() > ordem_filtro) {
            historico_x.erase(historico_x.begin());
            historico_y.erase(historico_y.begin());
            historico_angulo.erase(historico_angulo.begin());
        }
        
        info.sensores.i_posicao_x = std::accumulate(historico_x.begin(), historico_x.end(), 0) / historico_x.size();
        info.sensores.i_posicao_y = std::accumulate(historico_y.begin(), historico_y.end(), 0) / historico_y.size();
        info.sensores.i_angulo_x = std::accumulate(historico_angulo.begin(), historico_angulo.end(), 0) / historico_angulo.size();
        info.sensores.i_temperatura = temp_bruto;
        info.sensores.i_falha_eletrica = false;
        info.sensores.i_falha_hidraulica = false;
        
        // PUBLICAR para outros PROCESSOS via MQTT
        std::stringstream sensores_msg;
        sensores_msg << "SENSORES:" << id << "," 
                     << info.sensores.i_posicao_x << ","
                     << info.sensores.i_posicao_y << ","
                     << info.sensores.i_angulo_x << ","
                     << info.sensores.i_temperatura;
        
        mqtt.publicar("caminhao/sensores", sensores_msg.str());
        
        // Escrever no BUFFER INTERNO para outras tarefas
        buffer_sensores_tratados.escrever(info);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 10Hz
    }
}

// TAREFA: Lógica de Comando
void Caminhao::executar_logica_comando() {
    MQTTClient mqtt;
    mqtt.conectar("localhost", 1883, "caminhao_" + std::to_string(id) + "_comandos");
    
    // SUBSCREVER para receber comandos de outros PROCESSOS
    mqtt.subscrever("caminhao/comandos", [this](const std::string& mensagem) {
        std::cout << "Caminhão " << id << " recebeu comando: " << mensagem << std::endl;
        
        // Processar comando MQTT
        if (mensagem.find("AUTOMATICO") != std::string::npos) {
            // Ativar modo automático
        }
    });
    
    InfoCaminhao info;
    
    while (rodando) {
        // Ler do BUFFER INTERNO (dados dos sensores tratados)
        if (buffer_sensores_tratados.ler(info)) {
            // Processar lógica de comando baseada nos sensores
            
            // PUBLICAR estado para outros PROCESSOS
            std::stringstream estado_msg;
            estado_msg << "ESTADOS:" << id << ","
                       << info.estados.e_automatico << ","
                       << info.estados.e_defeito;
            
            mqtt.publicar("caminhao/estados", estado_msg.str());
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50)); // 20Hz
    }
}

// Implementações das outras tarefas...
void Caminhao::executar_monitoramento_falhas() {
    MQTTClient mqtt;
    mqtt.conectar("localhost", 1883, "caminhao_" + std::to_string(id) + "_falhas");
    
    while (rodando) {
        // Monitorar temperatura
        // Gerar eventos de falha
        // PUBLICAR falhas via MQTT
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 5Hz
    }
}

void Caminhao::executar_controle_navegacao() {
    while (rodando) {
        // Controle de navegação
        // Ler do buffer interno
        // Calcular atuadores
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 10Hz
    }
}

void Caminhao::executar_coletor_dados() {
    while (rodando) {
        // Coletar dados para log
        // Escrever em arquivo
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 2Hz
    }
}

void Caminhao::executar_interface_local() {
    while (rodando) {
        // Interface local com operador
        // Mostrar estados
        // Receber comandos locais
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 10Hz
    }
}

// ProcessoCaminhao (gerencia N caminhões)
ProcessoCaminhao::ProcessoCaminhao(int num_caminhoes) : rodando(true) {
    for (int i = 0; i < num_caminhoes; i++) {
        caminhoes.push_back(std::make_unique<Caminhao>(i));
    }
}

void ProcessoCaminhao::executar() {
    std::cout << "Iniciando " << caminhoes.size() << " caminhões" << std::endl;
    
    for (auto& caminhao : caminhoes) {
        caminhao->iniciar();
    }
    
    // Manter processo rodando
    while (rodando) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void ProcessoCaminhao::parar() {
    rodando = false;
    for (auto& caminhao : caminhoes) {
        caminhao->parar();
        caminhao->aguardar();
    }
}