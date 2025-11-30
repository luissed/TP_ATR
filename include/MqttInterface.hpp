// include/MqttInterface.hpp
#pragma once

#include <mqtt/async_client.h>
#include <string>
#include <iostream>
#include <functional>
#include <thread>
#include <chrono>

// Endereço do broker (localhost pois você está rodando local)
const std::string SERVER_ADDRESS = "tcp://localhost:1883";
const int QOS = 1;

class MqttInterface {
public:
    // Tipo para a função que será chamada quando chegar mensagem
    using MessageCallback = std::function<void(const std::string&, const std::string&)>;

    MqttInterface(const std::string& idCliente, MessageCallback callback = nullptr)
        : client_(SERVER_ADDRESS, idCliente), callback_(callback) 
    {
        // Configurações de conexão
        connOpts_.set_keep_alive_interval(20);
        connOpts_.set_clean_session(true);

        // Define o callback da biblioteca Paho
        client_.set_message_callback([this](mqtt::const_message_ptr msg) {
            if (callback_) {
                // Chama nossa função passando Tópico e Payload (conteúdo)
                callback_(msg->get_topic(), msg->to_string());
            }
        });
    }

    ~MqttInterface() {
        desconectar();
    }

    void conectar() {
        try {
            std::cout << "[MQTT] Conectando... (" << client_.get_client_id() << ")" << std::endl;
            client_.connect(connOpts_)->wait();
            std::cout << "[MQTT] Conectado!" << std::endl;
        }
        catch (const mqtt::exception& exc) {
            std::cerr << "[MQTT] Erro ao conectar: " << exc.what() << std::endl;
        }
    }

    void desconectar() {
        if (client_.is_connected()) {
            client_.disconnect()->wait();
        }
    }

    void publicar(const std::string& topico, const std::string& payload) {
        if (!client_.is_connected()) return;
        try {
            client_.publish(topico, payload, QOS, false);
        }
        catch (const mqtt::exception& exc) {
            std::cerr << "[MQTT] Erro ao publicar: " << exc.what() << std::endl;
        }
    }

    void assinar(const std::string& topico) {
        if (!client_.is_connected()) return;
        try {
            client_.subscribe(topico, QOS)->wait();
            std::cout << "[MQTT] Assinado no topico: " << topico << std::endl;
        }
        catch (const mqtt::exception& exc) {
            std::cerr << "[MQTT] Erro ao assinar: " << exc.what() << std::endl;
        }
    }

private:
    mqtt::async_client client_;
    mqtt::connect_options connOpts_;
    MessageCallback callback_;
};