#ifndef MQTT_MQTT_CLIENT_REAL_HPP
#define MQTT_MQTT_CLIENT_REAL_HPP

#include <mqtt/async_client.h>
#include <string>
#include <iostream>
#include "core/tipos.hpp"

class MQTTClientReal {
private:
    mqtt::async_client* cliente;
    std::string id;
    bool conectado;

public:
    MQTTClientReal() : cliente(nullptr), conectado(false) {}
    
    bool conectar(const std::string& broker, int port, const std::string& client_id) {
        try {
            std::string endereco = "tcp://" + broker + ":" + std::to_string(port);
            cliente = new mqtt::async_client(endereco, client_id);
            
            auto connOpts = mqtt::connect_options_builder()
                .clean_session(true)
                .finalize();
            
            cliente->connect(connOpts)->wait();
            conectado = true;
            std::cout << "[MQTT REAL] Conectado: " << client_id << std::endl;
            return true;
        } catch (const mqtt::exception& exc) {
            std::cerr << "[MQTT REAL] Erro: " << exc.what() << std::endl;
            return false;
        }
    }
    
    template<typename T>
    void publicar(const std::string& topic, const T& data) {
        if (!conectado || !cliente) return;
        
        try {
            // Serializar dados para JSON (simplificado)
            std::string payload = serializar_dados(data);
            cliente->publish(topic, payload, 1, false)->wait();
        } catch (const mqtt::exception& exc) {
            std::cerr << "[MQTT REAL] Erro publicando: " << exc.what() << std::endl;
        }
    }
    
    void desconectar() {
        if (cliente && conectado) {
            cliente->disconnect()->wait();
            delete cliente;
            cliente = nullptr;
        }
        conectado = false;
    }
    
private:
    std::string serializar_dados(const SensoresCaminhao& sensores) {
        // Implementar serialização JSON real
        return "{\"pos_x\":" + std::to_string(sensores.i_posicao_x) + "}";
    }
    
    std::string serializar_dados(const EstadosCaminhao& estados) {
        return "{\"auto\":" + std::to_string(estados.e_automatico) + "}";
    }
};

#endif