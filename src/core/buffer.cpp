#ifndef CORE_BUFFER_HPP
#define CORE_BUFFER_HPP

#include "tipos.hpp"
// Se o mqtt_client.hpp estiver em include/mqtt/, use:
#include "../mqtt/mqtt_client.hpp" 
#include <map>
#include <mutex>
#include <vector>
#include <string>

class Buffer {
private:
    std::mutex mtx;
    std::map<int, InfoCaminhao> frota; 
    MqttClient* mqtt_client; // O ponteiro deve estar aqui

    void onMqttMessage(std::string topico, std::string payload);
    int extrairIdDoTopico(const std::string& topico);

public:
    Buffer();
    ~Buffer();

    void iniciarComunicacao();

    // Métodos de Acesso
    void setInfoCaminhao(int id, const InfoCaminhao& info);
    bool getInfoCaminhao(int id, InfoCaminhao& info_out);
    std::vector<InfoCaminhao> getFrotaCompleta();

    // Métodos de Atualização
    void atualizarPosicao(int id, int x, int y, int angulo);
    void atualizarAtuadores(int id, int acel, int dir);
    void atualizarSetpoint(int id, int x, int y);
    void setModoAutomatico(int id, bool automatico);
    void registrarFalha(int id, bool eletrica, bool hidraulica, bool temperatura);
};

#endif#include "../../include/core/buffer.hpp"
#include <iostream>
#include <sstream>
#include <string>

// Construtor
Buffer::Buffer() : mqtt_client(nullptr) {
    mqtt_client = new MqttClient();
}

// Destrutor
Buffer::~Buffer() {
    if (mqtt_client) {
        delete mqtt_client;
    }
}

void Buffer::iniciarComunicacao() {
    if(mqtt_client) {
        mqtt_client->conectar("tcp://localhost:1883", "sistema_buffer_central");
        
        // Lambda para callback
        mqtt_client->setCallback([this](std::string t, std::string p) {
            this->onMqttMessage(t, p);
        });
        
        mqtt_client->subscribe("minas/caminhao/+/cmd");
    }
    std::cout << "[Buffer] Comunicacao Iniciada.\n";
}

void Buffer::onMqttMessage(std::string topico, std::string payload) {
    int id = extrairIdDoTopico(topico);
    if (id == -1) return;

    std::lock_guard<std::mutex> lock(mtx);
    if (frota.find(id) == frota.end()) frota[id].id_caminhao = id;

    if (payload == "CMD:AUTO") {
        frota[id].estados.e_automatico = true;
    } else if (payload == "CMD:MANUAL") {
        frota[id].estados.e_automatico = false;
    }
    // Lógica simplificada para evitar erros de string
}

int Buffer::extrairIdDoTopico(const std::string& topico) {
    try {
        size_t p1 = topico.find("caminhao/");
        if (p1 == std::string::npos) return -1;
        p1 += 9;
        size_t p2 = topico.find('/', p1);
        std::string s = topico.substr(p1, p2 - p1);
        return std::stoi(s);
    } catch (...) { return -1; }
}

// --- Implementação dos Getters/Setters ---
void Buffer::setInfoCaminhao(int id, const InfoCaminhao& info) {
    std::lock_guard<std::mutex> lock(mtx);
    frota[id] = info;
}

bool Buffer::getInfoCaminhao(int id, InfoCaminhao& info_out) {
    std::lock_guard<std::mutex> lock(mtx);
    if (frota.count(id)) {
        info_out = frota[id];
        return true;
    }
    return false;
}

std::vector<InfoCaminhao> Buffer::getFrotaCompleta() {
    std::lock_guard<std::mutex> lock(mtx);
    std::vector<InfoCaminhao> v;
    for(auto& pair : frota) v.push_back(pair.second);
    return v;
}

void Buffer::atualizarPosicao(int id, int x, int y, int angulo) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        frota[id].id_caminhao = id;
        frota[id].sensores.i_posicao_x = x;
        frota[id].sensores.i_posicao_y = y;
        frota[id].sensores.i_angulo_x = angulo;
    }
    // Opcional: Publicar MQTT aqui se necessário
}

void Buffer::atualizarAtuadores(int id, int acel, int dir) {
    std::lock_guard<std::mutex> lock(mtx);
    frota[id].atuadores.o_aceleracao = acel;
    frota[id].atuadores.o_direcao = dir;
}

void Buffer::atualizarSetpoint(int id, int x, int y) {
    std::lock_guard<std::mutex> lock(mtx);
    frota[id].navegacao.x_setpoint = x;
    frota[id].navegacao.y_setpoint = y;
    frota[id].navegacao.chegou = false;
}

void Buffer::setModoAutomatico(int id, bool automatico) {
    std::lock_guard<std::mutex> lock(mtx);
    frota[id].estados.e_automatico = automatico;
}

void Buffer::registrarFalha(int id, bool eletrica, bool hidraulica, bool temperatura) {
    std::lock_guard<std::mutex> lock(mtx);
    frota[id].sensores.i_falha_eletrica = eletrica;
    frota[id].sensores.i_falha_hidraulica = hidraulica;
    frota[id].estados.e_defeito = (eletrica || hidraulica || temperatura);
}