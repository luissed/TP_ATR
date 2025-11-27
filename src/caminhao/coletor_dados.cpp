#include "caminhao/coletor_dados.hpp"
#include "caminhao/contexto.hpp"
#include <fstream>
#include <chrono>
#include <iomanip>
#include <iostream>

void tarefa_coletor_dados(CaminhaoContext* ctx) {
    std::ofstream log_file;
    std::string filename = "caminhao_" + std::to_string(ctx->info.id_caminhao) + "_log.csv";
    
    log_file.open(filename, std::ios::out);
    log_file << "timestamp,id_caminhao,estado_defeito,estado_automatico,"
             << "pos_x,pos_y,angulo,temperatura,aceleracao,direcao,evento\n";
    log_file.close();
    
    int contador_log = 0;
    
    while (ctx->rodando) {
        if (contador_log % 10 == 0) {
            log_file.open(filename, std::ios::app);
            
            auto now = std::chrono::system_clock::now();
            auto timestamp = std::chrono::system_clock::to_time_t(now);
            
            log_file << std::put_time(std::localtime(&timestamp), "%Y-%m-%d %H:%M:%S") << ","
                     << ctx->info.id_caminhao << ","
                     << ctx->info.estados.e_defeito << ","
                     << ctx->info.estados.e_automatico << ","
                     << ctx->info.sensores.i_posicao_x << ","
                     << ctx->info.sensores.i_posicao_y << ","
                     << ctx->info.sensores.i_angulo_x << ","
                     << ctx->info.sensores.i_temperatura << ","
                     << ctx->info.atuadores.o_aceleracao << ","
                     << ctx->info.atuadores.o_direcao << ",";
            
            std::string evento = "NORMAL";
            if (ctx->eventos.ev_falha_eletrica) {
                evento = "FALHA_ELETRICA";
            } else if (ctx->eventos.ev_falha_hidraulica) {
                evento = "FALHA_HIDRAULICA";
            } else if (ctx->eventos.ev_sobretemperatura) {
                evento = "SOBRETEMPERATURA";
            }
            
            log_file << evento << "\n";
            log_file.close();
            
            if (contador_log % 100 == 0) {
                std::cout << "Caminhao " << ctx->info.id_caminhao << " [LOG]: Dados salvos - " 
                          << evento << std::endl;
            }
        }
        
        contador_log++;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    std::cout << "Coletor de dados finalizado para caminhao " << ctx->info.id_caminhao << std::endl;
    std::cout << "Log salvo em: " << filename << std::endl;
}