#ifndef CORE_CONFIG_HPP
#define CORE_CONFIG_HPP

#include <string>

struct ConfigMQTT {
    std::string broker_url = "localhost";
    int broker_port = 1883;
    std::string topic_sensores = "caminhao/sensores/";
    std::string topic_atuadores = "caminhao/atuadores/";
    std::string topic_comandos = "caminhao/comandos/";
    std::string topic_estados = "caminhao/estados/";
};

struct ConfigBuffer {
    size_t tamanho_buffer = 200;
    int ordem_filtro = 5;
};

#endif