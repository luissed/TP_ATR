// main_gui.cpp
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>

#include "SimulacaoMina.hpp"
#include "Tipos.hpp"

struct CaminhaoDrawInfo {
    int   id;
    bool  temDados;
    float xTela;
    float yTela;
};

int main() {
    std::cout << "GUI Gestao da Mina\n";

    // começa sem caminhões
    SimulacaoMina mina(0, 200);

    // cria 2 caminhões para teste
    int id1 = mina.criarNovoCaminhao(); // id = 1
    int id2 = mina.criarNovoCaminhao(); // id = 2

    // inicia threads internas de todos os caminhões
    mina.iniciar();

    // Configuração do mapa
    const int   WINDOW_WIDTH  = 800;
    const int   WINDOW_HEIGHT = 600;
    const float SCALE         = 4.0f; // 1 metro = 4 pixels
    const float ORIGEM_X      = WINDOW_WIDTH  / 2.0f; // (0,0) no centro
    const float ORIGEM_Y      = WINDOW_HEIGHT / 2.0f;

    sf::RenderWindow window(
        sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
        "Gestao da Mina - Mapa"
    );
    window.setFramerateLimit(60);

    // começa com o caminhão 1 selecionado
    int idSelecionado = id1;

    while (window.isOpen()) {
        // tratamento de eventos (mouse, fechar janela, etc)
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::MouseButtonPressed &&
                     event.mouseButton.button == sf::Mouse::Left) {

                sf::Vector2i pixel(event.mouseButton.x, event.mouseButton.y);

                // construir lista de caminhões com posição em tela
                std::vector<CaminhaoDrawInfo> infos;
                infos.reserve(mina.quantidadeCaminhoes());

                for (std::size_t i = 0; i < mina.quantidadeCaminhoes(); ++i) {
                    Caminhao& c = mina.getCaminhao(i);

                    RegistroBuffer reg{};
                    bool ok = c.lerUltimoRegistro(reg);

                    CaminhaoDrawInfo info{};
                    info.id       = c.getId();
                    info.temDados = ok;

                    if (ok) {
                        info.xTela = ORIGEM_X + static_cast<float>(reg.sensores.i_posicao_x) * SCALE;
                        info.yTela = ORIGEM_Y - static_cast<float>(reg.sensores.i_posicao_y) * SCALE;
                    } else {
                        info.xTela = ORIGEM_X;
                        info.yTela = ORIGEM_Y;
                    }

                    infos.push_back(info);
                }

                // ver se clicou em cima de algum caminhão
                const float LIMITE_CLICK = 12.0f; // raio de seleção em pixels
                float melhorDist = 1e9f;
                int   idClicado  = -1;

                for (const auto& info : infos) {
                    if (!info.temDados) {
                        continue;
                    }

                    float dx = static_cast<float>(pixel.x) - info.xTela;
                    float dy = static_cast<float>(pixel.y) - info.yTela;
                    float dist = std::sqrt(dx*dx + dy*dy);

                    if (dist < melhorDist) {
                        melhorDist = dist;
                        idClicado  = info.id;
                    }
                }

                if (idClicado != -1 && melhorDist <= LIMITE_CLICK) {
                    // clicou em cima de um caminhão, então muda seleção
                    idSelecionado = idClicado;
                    std::cout << "[GUI] Caminhao selecionado: " << idSelecionado << "\n";
                } else if (idSelecionado != -1) {
                    // não clicou em caminhão, novo destino para o caminhão selecionado
                    double worldX = (static_cast<double>(pixel.x) - ORIGEM_X) / SCALE;
                    double worldY = (ORIGEM_Y - static_cast<double>(pixel.y)) / SCALE;

                    Caminhao& cSel = mina.getCaminhaoPorId(idSelecionado);

                    RegistroBuffer reg{};
                    if (cSel.lerUltimoRegistro(reg)) {
                        int x0 = reg.sensores.i_posicao_x;
                        int y0 = reg.sensores.i_posicao_y;

                        int xDest = static_cast<int>(std::lround(worldX));
                        int yDest = static_cast<int>(std::lround(worldY));

                        cSel.definirRota(x0, y0, xDest, yDest);
                        cSel.comandarAutomatico();

                        std::cout << "[GUI] Nova rota para caminhao "
                                  << idSelecionado << ": ("
                                  << x0 << "," << y0 << ") -> ("
                                  << xDest << "," << yDest << ")\n";
                    }
                }
            }
        }

        // Desenho
        window.clear(sf::Color(30, 30, 30));

        // desenha eixos X e Y no centro
        {
            sf::Vertex eixosHoriz[2] = {
                sf::Vertex(sf::Vector2f(0.f, ORIGEM_Y), sf::Color(80,80,80)),
                sf::Vertex(sf::Vector2f(static_cast<float>(WINDOW_WIDTH), ORIGEM_Y),
                           sf::Color(80,80,80))
            };
            sf::Vertex eixosVert[2] = {
                sf::Vertex(sf::Vector2f(ORIGEM_X, 0.f), sf::Color(80,80,80)),
                sf::Vertex(sf::Vector2f(ORIGEM_X, static_cast<float>(WINDOW_HEIGHT)),
                           sf::Color(80,80,80))
            };
            window.draw(eixosHoriz, 2, sf::Lines);
            window.draw(eixosVert,  2, sf::Lines);
        }

        // desenha todos os caminhões
        for (std::size_t i = 0; i < mina.quantidadeCaminhoes(); ++i) {
            Caminhao& c = mina.getCaminhao(i);

            RegistroBuffer reg{};
            if (!c.lerUltimoRegistro(reg)) {
                continue; // ainda sem dados
            }

            float xTela = ORIGEM_X + static_cast<float>(reg.sensores.i_posicao_x) * SCALE;
            float yTela = ORIGEM_Y - static_cast<float>(reg.sensores.i_posicao_y) * SCALE;

            bool selecionado = (c.getId() == idSelecionado);

            sf::CircleShape circulo(6.f);
            circulo.setOrigin(6.f, 6.f);
            circulo.setPosition(xTela, yTela);
            circulo.setFillColor(selecionado ? sf::Color::Yellow : sf::Color::Cyan);

            window.draw(circulo);
        }

        window.display();
    }

    // fecha simulação
    mina.parar();
    std::cout << "GUI Gestao da Mina encerrada\n";
    return 0;
}
