#ifndef MQTT_MQTT_CLIENT_HPP
#define MQTT_MQTT_CLIENT_HPP

#include <mqtt/async_client.h>
#include <string>
#include <iostream>
#include <functional>

class MQTTClient {
private:
    mqtt::async_client* cliente;
    std::string id;
    bool conectado;

public:
    MQTTClient() : cliente(nullptr), conectado(false) {}
    ~MQTTClient() { desconectar(); }
    
    bool conectar(const std::string& broker, int port, const std::string& client_id) {
        try {
            std::string endereco = "tcp://" + broker + ":" + std::to_string(port);
            cliente = new mqtt::async_client(endereco, client_id);
            
            auto connOpts = mqtt::connect_options_builder()
                .clean_session(true)
                .finalize();
            
            cliente->connect(connOpts)->wait();
            conectado = true;
            std::cout << "[MQTT] " << client_id << " conectado" << std::endl;
            return true;
        } catch (const mqtt::exception& exc) {
            std::cerr << "[MQTT] Erro: " << exc.what() << std::endl;
            return false;
        }
    }
    
    void publicar(const std::string& topic, const std::string& mensagem) {
        if (!conectado || !cliente) return;
        
        try {
            auto msg = mqtt::make_message(topic, mensagem);
            msg->set_qos(1);
            cliente->publish(msg)->wait();
        } catch (const mqtt::exception& exc) {
            std::cerr << "[MQTT] Erro publicando: " << exc.what() << std::endl;
        }
    }
    
    void subscrever(const std::string& topic, std::function<void(const std::string&)> callback) {
        if (!conectado || !cliente) return;
        
        try {
            cliente->subscribe(topic, 1)->wait();
            
            cliente->set_message_callback([callback](mqtt::const_message_ptr msg) {
                callback(msg->get_payload_str());
            });
            
        } catch (const mqtt::exception& exc) {
            std::cerr << "[MQTT] Erro inscrevendo: " << exc.what() << std::endl;
        }
    }
    
    void desconectar() {
        if (cliente && conectado) {
            cliente->disconnect()->wait();
            delete cliente;
            cliente = nullptr;
            conectado = false;
        }
    }
};

#endif