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

#endif