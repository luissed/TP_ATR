#include "caminhao/interface_local.hpp"
#include "caminhao/contexto.hpp"
#include <iostream>
#include <chrono>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

int _kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

char _getch() {
    struct termios oldt, newt;
    char ch;
    
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    ch = getchar();
    
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

void limpar_tela() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void tarefa_interface_local(CaminhaoContext* ctx) {
    while (ctx->rodando) {
        limpar_tela();
        
        std::cout << "=== CAMINHAO AUTONOMO " << ctx->info.id_caminhao << " ===" << std::endl;
        std::cout << "Estado: " << (ctx->info.estados.e_defeito ? "DEFEITO" : 
                                  (ctx->info.estados.e_automatico ? "AUTOMATICO" : "MANUAL")) << std::endl;
        std::cout << "Posicao: (" << ctx->info.sensores.i_posicao_x << ", " 
                  << ctx->info.sensores.i_posicao_y << ")" << std::endl;
        std::cout << "Angulo: " << ctx->info.sensores.i_angulo_x << " graus" << std::endl;
        std::cout << "Temperatura: " << ctx->info.sensores.i_temperatura << " C" << std::endl;
        std::cout << "Aceleracao: " << ctx->info.atuadores.o_aceleracao << "%" << std::endl;
        std::cout << "Direcao: " << ctx->info.atuadores.o_direcao << " graus" << std::endl;
        
        std::cout << "\nFalhas: ";
        if (ctx->eventos.ev_falha_eletrica) std::cout << "[ELETRICA] ";
        if (ctx->eventos.ev_falha_hidraulica) std::cout << "[HIDRAULICA] ";
        if (ctx->eventos.ev_sobretemperatura) std::cout << "[SOBRETEMP] ";
        if (!ctx->eventos.ev_falha_eletrica && !ctx->eventos.ev_falha_hidraulica && !ctx->eventos.ev_sobretemperatura) {
            std::cout << "NENHUMA";
        }
        std::cout << std::endl;
        
        std::cout << "\nComandos: [1] Automatico [2] Manual [3] Rearme" << std::endl;
        std::cout << "          [W] Acelerar [A] Esquerda [D] Direita [S] Parar [Q] Sair" << std::endl;
        
        if (_kbhit()) {
            char tecla = _getch();
            tecla = toupper(tecla);
            
            ctx->info.comandos.c_acelera = false;
            ctx->info.comandos.c_esquerda = false;
            ctx->info.comandos.c_direita = false;
            ctx->info.comandos.c_automatico = false;
            ctx->info.comandos.c_man = false;
            ctx->info.comandos.c_rearme = false;
            
            switch (tecla) {
                case '1': 
                    ctx->info.comandos.c_automatico = true; 
                    std::cout << " >> Modo AUTOMATICO ativado" << std::endl;
                    break;
                case '2': 
                    ctx->info.comandos.c_man = true; 
                    std::cout << " >> Modo MANUAL ativado" << std::endl;
                    break;
                case '3': 
                    ctx->info.comandos.c_rearme = true; 
                    std::cout << " >> SISTEMA REARMADO" << std::endl;
                    break;
                case 'W': 
                    ctx->info.comandos.c_acelera = true; 
                    std::cout << " >> ACELERANDO" << std::endl;
                    break;
                case 'S': 
                    ctx->info.atuadores.o_aceleracao = 0;
                    std::cout << " >> PARANDO" << std::endl;
                    break;
                case 'D': 
                    ctx->info.comandos.c_direita = true;
                    std::cout << " >> DIREITA" << std::endl;
                    break;
                case 'A': 
                    ctx->info.comandos.c_esquerda = true;
                    std::cout << " >> ESQUERDA" << std::endl;
                    break;
                case 'Q': 
                    ctx->rodando = false; 
                    std::cout << " >> ENCERRANDO..." << std::endl;
                    break;
                default:
                    break;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}