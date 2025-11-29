#include "../../include/caminhao/coletor_dados.hpp"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <thread>

ColetorDados::ColetorDados(Buffer& b) : buffer(b), ativo(false) {
    // Muda a extensão para .csv
    arquivo_log.open("log_caminhoes.csv", std::ios::app);
    
    if (!arquivo_log.is_open()) {
        std::cerr << "[ERRO] Nao foi possivel criar arquivo de log!\n";
    } else {
        // ESCREVE O CABEÇALHO DO EXCEL (Só se o arquivo estiver vazio)
        // O seekp(0, end) verifica o tamanho. Se for 0, escreve cabeçalho.
        arquivo_log.seekp(0, std::ios::end);
        if (arquivo_log.tellp() == 0) {
            arquivo_log << "TIMESTAMP;ID_CAMINHAO;MODO_OPERACAO;POSICAO_X;POSICAO_Y;MENSAGEM_EVENTO\n";
        }
    }
}

ColetorDados::~ColetorDados() {
    if (arquivo_log.is_open()) {
        arquivo_log.close();
    }
}

std::string ColetorDados::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    // Formato compatível com Excel
    ss << std::put_time(std::localtime(&now_c), "%d/%m/%Y %H:%M:%S");
    return ss.str();
}

void ColetorDados::log(int id, const std::string& evento) {
    std::lock_guard<std::mutex> lock(mtx_arquivo);
    if (arquivo_log.is_open()) {
        InfoCaminhao info;
        // Tenta pegar info do buffer (pode falhar se o caminhão ainda não foi criado)
        // Se falhar, preenchemos com zeros para não quebrar o CSV
        int x = 0, y = 0;
        std::string modo = "DESCONHECIDO";

        if(buffer.getInfoCaminhao(id, info)) {
            x = info.sensores.i_posicao_x;
            y = info.sensores.i_posicao_y;
            modo = (info.estados.e_automatico ? "AUTOMATICO" : "MANUAL");
        }

        // ESCRITA FORMATADA PARA EXCEL (Separado por ponto-e-vírgula ;)
        arquivo_log << getTimestamp() << ";"
                    << id << ";"
                    << modo << ";"
                    << x << ";"
                    << y << ";"
                    << evento << "\n";
                    
        arquivo_log.flush(); 
    }
}

void ColetorDados::parar() {
    ativo = false;
}

void ColetorDados::loop() {
    ativo = true;
    std::cout << "[COLETOR] Gravando dados em 'log_caminhoes.csv'...\n";

    while (ativo) {
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "[COLETOR] Encerrando.\n";
}
