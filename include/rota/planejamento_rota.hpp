#ifndef ROTA_PLANEJAMENTO_ROTA_HPP
#define ROTA_PLANEJAMENTO_ROTA_HPP

#include "../core/buffer.hpp"
#include "../core/tipos.hpp"
#include <map>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>

// Estrutura simples para um ponto no mapa
struct Coordenada {
    int x;
    int y;
};

class PlanejamentoRota {
private:
    Buffer& buffer; // Referência ao buffer compartilhado
    std::atomic<bool> ativo; // Controle da thread
    std::thread th_planejamento; // Thread interna

    // Mapa de filas: ID do Caminhão -> Fila de pontos a visitar
    std::map<int, std::queue<Coordenada>> filas_de_rotas;
    std::mutex mtx_rotas; // Proteção para adicionar rotas enquanto a thread lê

    void loop(); // Loop principal da thread

public:
    // Construtor recebe o Buffer por referência
    PlanejamentoRota(Buffer& b);
    ~PlanejamentoRota();

    // Inicia e para a thread de planejamento
    void iniciar();
    void parar();

    // Método para a "Gestão da Mina" adicionar pontos para um caminhão
    void adicionarDestino(int id_caminhao, int x, int y);
    
    // Limpa a rota de um caminhão específico (parada de emergência ou nova ordem)
    void limparRota(int id_caminhao);
};

#endif