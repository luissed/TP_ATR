#ifndef MQTT_MQTT_CLIENT_HPP
#define MQTT_MQTT_CLIENT_HPP

#include <string>
#include <iostream>
#include <type_traits>
#include "core/tipos.hpp"

class MQTTClient {
private:
    std::string id;
    bool conectado;

public:
    MQTTClient() : conectado(false) {}
    
    bool conectar(const std::string& broker, int port, const std::string& client_id) {
        id = client_id;
        conectado = true;
        std::cout << "[MQTT SIM] Conectado: " << client_id << " em " << broker << ":" << port << std::endl;
        return true;
    }
    
    template<typename T>
    void publicar(const std::string& topic, const T& data) {
        if (!conectado) return;
        
        std::cout << "[MQTT SIM] Publicando em " << topic;
        
        if constexpr (std::is_same_v<T, SensoresCaminhao>) {
            std::cout << " [Sensores: x=" << data.i_posicao_x << ", y=" << data.i_posicao_y 
                      << ", ang=" << data.i_angulo_x << "°, temp=" << data.i_temperatura << "°C]" << std::endl;
        } else if constexpr (std::is_same_v<T, EstadosCaminhao>) {
            std::cout << " [Estados: auto=" << data.e_automatico << ", defeito=" << data.e_defeito << "]" << std::endl;
        } else if constexpr (std::is_same_v<T, AtuadoresCaminhao>) {
            std::cout << " [Atuadores: acel=" << data.o_aceleracao << "%, dir=" << data.o_direcao << "°]" << std::endl;
        } else {
            std::cout << " [Dados serializados]" << std::endl;
        }
    }
    
    void desconectar() {
        conectado = false;
        std::cout << "[MQTT SIM] Desconectado: " << id << std::endl;
    }
    
    bool esta_conectado() const { return conectado; }
};

#endif