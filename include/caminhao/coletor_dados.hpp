#pragma once
#include "../core/buffer.hpp"
#include <fstream>
#include <string>
#include <mutex>

class ColetorDados {
private:
    Buffer& buffer;
    std::ofstream arquivo_log;
    std::mutex mtx_arquivo;
    bool ativo;

    // Função para pegar hora atual formatada
    std::string getTimestamp();

public:
    ColetorDados(Buffer& b);
    ~ColetorDados();
    
    // Registra um evento no arquivo
    void log(int id_caminhao, const std::string& evento);
    
    // Loop que fica monitorando e salvando estados periodicamente
    void loop(); 
    void parar();
};