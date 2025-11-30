#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>

#include "SimulacaoMina.hpp"
#include "Tipos.hpp"

namespace {
    constexpr double PI = 3.14159265358979323846;
}

struct CaminhaoDrawInfo {
    int   id;
    bool  temDados;
    float xTela;
    float yTela;
};

int main() {
    std::cout << "GUI Gestao da Mina\n";

    // comeca sem caminhoes
    SimulacaoMina mina(0, 200);

    // inicia threads internas se nao houver caminhoes nao faz nada por enquanto
    mina.iniciar();

    // configuracao do mapa
    const int   WINDOW_WIDTH  = 800;
    const int   WINDOW_HEIGHT = 600;
    const float SCALE         = 4.0f; // 1 metro = 4 pixels
    const float ORIGEM_X      = WINDOW_WIDTH  / 2.0f; // zero zero no centro
    const float ORIGEM_Y      = WINDOW_HEIGHT / 2.0f;

    sf::RenderWindow window(
        sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT),
        "Gestao da Mina - Mapa"
    );
    window.setFramerateLimit(60);

    // nenhum caminhao selecionado no inicio
    int idSelecionado = -1;

    // botao para criar novos caminhoes no canto superior esquerdo
    sf::RectangleShape botaoNovo(sf::Vector2f(40.f, 30.f));
    botaoNovo.setPosition(10.f, 10.f);
    botaoNovo.setFillColor(sf::Color(120, 120, 120));
    botaoNovo.setOutlineColor(sf::Color::White);
    botaoNovo.setOutlineThickness(2.f);

    auto desenharMais = [&](sf::RenderWindow& win) {
        sf::Vector2f pos  = botaoNovo.getPosition();
        sf::Vector2f size = botaoNovo.getSize();
        float cx = pos.x + size.x / 2.0f;
        float cy = pos.y + size.y / 2.0f;
        float h  = 8.0f;

        sf::Vertex linhaH[2] = {
            sf::Vertex(sf::Vector2f(cx - h, cy), sf::Color::White),
            sf::Vertex(sf::Vector2f(cx + h, cy), sf::Color::White)
        };
        sf::Vertex linhaV[2] = {
            sf::Vertex(sf::Vector2f(cx, cy - h), sf::Color::White),
            sf::Vertex(sf::Vector2f(cx, cy + h), sf::Color::White)
        };
        win.draw(linhaH, 2, sf::Lines);
        win.draw(linhaV, 2, sf::Lines);
    };

    // fonte para textos do painel lateral
    sf::Font fonte;
    bool fonteOk = false;
    if (fonte.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        fonteOk = true;
    } else {
        std::cout << "[GUI] Nao foi possivel carregar fonte. "
                     "Painel sera sem texto.\n";
    }

    auto criarTexto = [&](const std::string& s, float x, float y) {
        sf::Text txt;
        txt.setFont(fonte);
        txt.setString(s);
        txt.setCharacterSize(14);
        txt.setFillColor(sf::Color::White);
        txt.setPosition(x, y);
        return txt;
    };

    // painel lateral de telemetria e comandos
    const float painelWidth  = 260.f;
    const float painelHeight = 280.f; // um pouco maior pra caber rearme e botoes de falha
    sf::RectangleShape painelInfo(sf::Vector2f(painelWidth, painelHeight));
    painelInfo.setPosition(WINDOW_WIDTH - painelWidth - 10.f, 50.f);
    painelInfo.setFillColor(sf::Color(40, 40, 40));
    painelInfo.setOutlineColor(sf::Color::White);
    painelInfo.setOutlineThickness(2.f);

    float painelX = painelInfo.getPosition().x;
    float painelY = painelInfo.getPosition().y;

    // botoes auto e manual dentro do painel na primeira linha
    sf::RectangleShape painelBotaoAuto(sf::Vector2f(80.f, 24.f));
    sf::RectangleShape painelBotaoManual(sf::Vector2f(80.f, 24.f));

    float linha1Y = painelY + 10.f;
    float colEsqX = painelX + 10.f;
    float colDirX = colEsqX + 90.f;

    painelBotaoAuto.setPosition(colEsqX, linha1Y);
    painelBotaoManual.setPosition(colDirX, linha1Y);

    painelBotaoAuto.setOutlineColor(sf::Color::White);
    painelBotaoAuto.setOutlineThickness(1.f);
    painelBotaoManual.setOutlineColor(sf::Color::White);
    painelBotaoManual.setOutlineThickness(1.f);

    // botao de rearme na segunda linha do painel
    sf::RectangleShape painelBotaoRearme(sf::Vector2f(80.f, 24.f));
    float linha2Y = linha1Y + 30.f;
    painelBotaoRearme.setPosition(colEsqX, linha2Y);
    painelBotaoRearme.setOutlineColor(sf::Color::White);
    painelBotaoRearme.setOutlineThickness(1.f);

    // botoes para forcar falhas na coluna da direita do painel
    sf::RectangleShape painelBotaoFalhaTemp(sf::Vector2f(90.f, 22.f));
    sf::RectangleShape painelBotaoFalhaElec(sf::Vector2f(90.f, 22.f));
    sf::RectangleShape painelBotaoFalhaHid(sf::Vector2f(90.f, 22.f));

    float falhaX = colDirX;
    float falhaY = linha2Y;

    painelBotaoFalhaTemp.setPosition(falhaX, falhaY);
    painelBotaoFalhaElec.setPosition(falhaX, falhaY + 24.f);
    painelBotaoFalhaHid.setPosition(falhaX, falhaY + 48.f);

    painelBotaoFalhaTemp.setOutlineColor(sf::Color::White);
    painelBotaoFalhaTemp.setOutlineThickness(1.f);
    painelBotaoFalhaElec.setOutlineColor(sf::Color::White);
    painelBotaoFalhaElec.setOutlineThickness(1.f);
    painelBotaoFalhaHid.setOutlineColor(sf::Color::White);
    painelBotaoFalhaHid.setOutlineThickness(1.f);

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            // trata clique de mouse com botao esquerdo
            else if (event.type == sf::Event::MouseButtonPressed &&
                     event.mouseButton.button == sf::Mouse::Left) {

                sf::Vector2i pixel(event.mouseButton.x, event.mouseButton.y);
                sf::Vector2f pixelF(static_cast<float>(pixel.x),
                                    static_cast<float>(pixel.y));

                // botao que cria caminhao novo
                if (botaoNovo.getGlobalBounds().contains(pixelF)) {
                    int novoId = mina.criarNovoCaminhao();
                    std::cout << "[GUI] Botao + clicado. Novo caminhao id="
                              << novoId << "\n";

                    if (mina.quantidadeCaminhoes() == 1) {
                        idSelecionado = novoId;
                        std::cout << "[GUI] Caminhao selecionado: "
                                  << idSelecionado << "\n";
                    }
                    continue;
                }

                // botoes do painel lateral auto manual rearme e falhas
                if (idSelecionado != -1) {
                    // auto
                    if (painelBotaoAuto.getGlobalBounds().contains(pixelF)) {
                        Caminhao& cSel = mina.getCaminhaoPorId(idSelecionado);
                        cSel.comandarAutomatico();
                        std::cout << "[GUI] (Painel) Modo AUTOMATICO para caminhao "
                                  << idSelecionado << "\n";
                        continue;
                    }
                    // manual
                    if (painelBotaoManual.getGlobalBounds().contains(pixelF)) {
                        Caminhao& cSel = mina.getCaminhaoPorId(idSelecionado);
                        cSel.comandarManual();
                        std::cout << "[GUI] (Painel) Modo MANUAL para caminhao "
                                  << idSelecionado << "\n";
                        continue;
                    }
                    // rearme
                    if (painelBotaoRearme.getGlobalBounds().contains(pixelF)) {
                        Caminhao& cSel = mina.getCaminhaoPorId(idSelecionado);
                        cSel.comandarRearme();
                        std::cout << "[GUI] (Painel) REARME enviado para caminhao "
                                  << idSelecionado << "\n";
                        continue;
                    }
                    // falha de temperatura
                    if (painelBotaoFalhaTemp.getGlobalBounds().contains(pixelF)) {
                        mina.injetarFalhaTemperatura(idSelecionado);
                        std::cout << "[GUI] (Painel) Falha de TEMPERATURA injetada no caminhao "
                                  << idSelecionado << "\n";
                        continue;
                    }
                    // falha eletrica
                    if (painelBotaoFalhaElec.getGlobalBounds().contains(pixelF)) {
                        mina.injetarFalhaEletrica(idSelecionado);
                        std::cout << "[GUI] (Painel) Falha ELETRICA injetada no caminhao "
                                  << idSelecionado << "\n";
                        continue;
                    }
                    // falha hidraulica
                    if (painelBotaoFalhaHid.getGlobalBounds().contains(pixelF)) {
                        mina.injetarFalhaHidraulica(idSelecionado);
                        std::cout << "[GUI] (Painel) Falha HIDRAULICA injetada no caminhao "
                                  << idSelecionado << "\n";
                        continue;
                    }
                }

                // clique em caminhao ou em algum ponto do mapa
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

                const float LIMITE_CLICK = 12.0f;
                float melhorDist = 1e9f;
                int   idClicado  = -1;

                for (const auto& info : infos) {
                    if (!info.temDados) continue;

                    float dx = static_cast<float>(pixel.x) - info.xTela;
                    float dy = static_cast<float>(pixel.y) - info.yTela;
                    float dist = std::sqrt(dx*dx + dy*dy);

                    if (dist < melhorDist) {
                        melhorDist = dist;
                        idClicado  = info.id;
                    }
                }

                if (idClicado != -1 && melhorDist <= LIMITE_CLICK) {
                    idSelecionado = idClicado;
                    std::cout << "[GUI] Caminhao selecionado: " << idSelecionado << "\n";
                } else if (idSelecionado != -1) {
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
            // controle manual com teclado usando wasd
            else if (event.type == sf::Event::KeyPressed) {
                if (idSelecionado != -1) {
                    Caminhao& cSel = mina.getCaminhaoPorId(idSelecionado);

                    if (event.key.code == sf::Keyboard::W) {
                        cSel.setComandoAcelerar(true);
                    }
                    if (event.key.code == sf::Keyboard::A) {
                        cSel.setComandoEsquerda(true);
                        cSel.setComandoDireita(false);
                    }
                    if (event.key.code == sf::Keyboard::D) {
                        cSel.setComandoDireita(true);
                        cSel.setComandoEsquerda(false);
                    }
                }
            }
            else if (event.type == sf::Event::KeyReleased) {
                if (idSelecionado != -1) {
                    Caminhao& cSel = mina.getCaminhaoPorId(idSelecionado);

                    if (event.key.code == sf::Keyboard::W) {
                        cSel.setComandoAcelerar(false);
                    }
                    if (event.key.code == sf::Keyboard::A) {
                        cSel.setComandoEsquerda(false);
                    }
                    if (event.key.code == sf::Keyboard::D) {
                        cSel.setComandoDireita(false);
                    }
                }
            }
        }

        // parte de desenho da tela
        window.clear(sf::Color(30, 30, 30));

        // desenha eixos do mapa
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

        // desenha botao de novo caminhao
        window.draw(botaoNovo);
        desenharMais(window);

        // guarda registro do caminhao selecionado para o painel
        RegistroBuffer regSel{};
        bool temRegSel = false;

        // desenha todos os caminhoes no mapa
        for (std::size_t i = 0; i < mina.quantidadeCaminhoes(); ++i) {
            Caminhao& c = mina.getCaminhao(i);

            RegistroBuffer reg{};
            if (!c.lerUltimoRegistro(reg)) {
                continue;
            }

            float xTela = ORIGEM_X + static_cast<float>(reg.sensores.i_posicao_x) * SCALE;
            float yTela = ORIGEM_Y - static_cast<float>(reg.sensores.i_posicao_y) * SCALE;

            bool selecionado = (c.getId() == idSelecionado);
            bool modoAuto    = reg.estados.e_automatico;

            if (selecionado) {
                regSel = reg;
                temRegSel = true;
            }

            sf::CircleShape circulo(6.f);
            circulo.setOrigin(6.f, 6.f);
            circulo.setPosition(xTela, yTela);

            if (modoAuto) {
                circulo.setFillColor(sf::Color::Cyan);          // cor para modo automatico
            } else {
                circulo.setFillColor(sf::Color(255, 165, 0));   // cor para modo manual
            }

            if (selecionado) {
                circulo.setOutlineColor(sf::Color::White);
                circulo.setOutlineThickness(2.f);
            } else {
                circulo.setOutlineThickness(0.f);
            }

            // desenha uma seta pequena indicando a direcao do caminhao
            double angRad = static_cast<double>(reg.sensores.i_angulo_x) * PI / 180.0;
            float dirX = std::cos(angRad);
            float dirY = std::sin(angRad);

            float comprimento = 20.0f; // tamanho aproximado da seta em pixels
            sf::Vertex linhaDir[2] = {
                sf::Vertex(sf::Vector2f(xTela, yTela), sf::Color::White),
                sf::Vertex(sf::Vector2f(
                    xTela + comprimento * dirX,
                    yTela - comprimento * dirY   // eixo y da tela cresce pra baixo por isso subtrai
                ), sf::Color::White)
            };

            window.draw(linhaDir, 2, sf::Lines);
            window.draw(circulo);
        }

        // painel lateral com telemetria e botoes de comando
        if (idSelecionado != -1 && temRegSel) {
            // cores base dos botoes do painel
            painelBotaoAuto.setFillColor(sf::Color(60, 120, 60));
            painelBotaoManual.setFillColor(sf::Color(120, 60, 60));
            painelBotaoRearme.setFillColor(sf::Color(90, 90, 90));

            painelBotaoFalhaTemp.setFillColor(sf::Color(90, 40, 40));
            painelBotaoFalhaElec.setFillColor(sf::Color(90, 40, 40));
            painelBotaoFalhaHid.setFillColor(sf::Color(90, 40, 40));

            // destaca o botao do modo atual
            if (regSel.estados.e_automatico) {
                painelBotaoAuto.setFillColor(sf::Color(0, 200, 0));
            } else {
                painelBotaoManual.setFillColor(sf::Color(200, 0, 0));
            }

            // se estiver em defeito destaca o botao de rearme
            if (regSel.estados.e_defeito) {
                painelBotaoRearme.setFillColor(sf::Color(200, 200, 0));
            }

            // destaca botoes de falha quando o sensor indica falha
            if (regSel.sensores.i_temperatura > 120) {
                painelBotaoFalhaTemp.setFillColor(sf::Color(200, 0, 0));
            }
            if (regSel.sensores.i_falha_eletrica) {
                painelBotaoFalhaElec.setFillColor(sf::Color(200, 0, 0));
            }
            if (regSel.sensores.i_falha_hidraulica) {
                painelBotaoFalhaHid.setFillColor(sf::Color(200, 0, 0));
            }

            window.draw(painelInfo);

            // botoes auto manual e rearme no painel
            window.draw(painelBotaoAuto);
            window.draw(painelBotaoManual);
            window.draw(painelBotaoRearme);

            // botoes para forcar falhas
            window.draw(painelBotaoFalhaTemp);
            window.draw(painelBotaoFalhaElec);
            window.draw(painelBotaoFalhaHid);

            if (fonteOk) {
                // textos dos botoes de modo
                window.draw(criarTexto(
                    "AUTO",
                    painelBotaoAuto.getPosition().x + 15.f,
                    painelBotaoAuto.getPosition().y + 3.f
                ));
                window.draw(criarTexto(
                    "MANUAL",
                    painelBotaoManual.getPosition().x + 8.f,
                    painelBotaoManual.getPosition().y + 3.f
                ));

                // texto do botao de rearme
                window.draw(criarTexto(
                    "REARME",
                    painelBotaoRearme.getPosition().x + 5.f,
                    painelBotaoRearme.getPosition().y + 3.f
                ));

                // textos dos botoes de falha
                window.draw(criarTexto(
                    "F. Temp",
                    painelBotaoFalhaTemp.getPosition().x + 8.f,
                    painelBotaoFalhaTemp.getPosition().y + 2.f
                ));
                window.draw(criarTexto(
                    "F. Elec",
                    painelBotaoFalhaElec.getPosition().x + 8.f,
                    painelBotaoFalhaElec.getPosition().y + 2.f
                ));
                window.draw(criarTexto(
                    "F. Hid",
                    painelBotaoFalhaHid.getPosition().x + 10.f,
                    painelBotaoFalhaHid.getPosition().y + 2.f
                ));

                // telemetria logo abaixo dos botoes do painel
                float x0 = painelX + 10.f;
                float y0 = linha2Y + 24.f + 50.f;
                float dy = 18.f;
                int linha = 0;

                window.draw(criarTexto(
                    "Caminhao ID: " + std::to_string(idSelecionado),
                    x0, y0 + dy * linha++
                ));

                window.draw(criarTexto(
                    "Estado logico: " + estadoToString(regSel.estado),
                    x0, y0 + dy * linha++
                ));

                window.draw(criarTexto(
                    "e_defeito: " + std::to_string(regSel.estados.e_defeito ? 1 : 0),
                    x0, y0 + dy * linha++
                ));

                window.draw(criarTexto(
                    "e_automatico: " + std::to_string(regSel.estados.e_automatico ? 1 : 0),
                    x0, y0 + dy * linha++
                ));

                window.draw(criarTexto(
                    "Posicao: x=" + std::to_string(regSel.sensores.i_posicao_x) +
                    " y=" + std::to_string(regSel.sensores.i_posicao_y),
                    x0, y0 + dy * linha++
                ));

                window.draw(criarTexto(
                    "Angulo: " + std::to_string(regSel.sensores.i_angulo_x) + " deg",
                    x0, y0 + dy * linha++
                ));

                window.draw(criarTexto(
                    "Temp motor: " + std::to_string(regSel.sensores.i_temperatura) + " C",
                    x0, y0 + dy * linha++
                ));

                // comando de aceleracao vai de menos cem ate cem por cento
                window.draw(criarTexto(
                    "Cmd acel: " + std::to_string(regSel.atuadores.o_aceleracao) + " %",
                    x0, y0 + dy * linha++
                ));

                window.draw(criarTexto(
                    "SP pos: x=" + std::to_string(regSel.setpoints.sp_posicao_x) +
                    " y=" + std::to_string(regSel.setpoints.sp_posicao_y),
                    x0, y0 + dy * linha++
                ));

                window.draw(criarTexto(
                    "SP angulo: " + std::to_string(regSel.setpoints.sp_angulo_x) + " deg",
                    x0, y0 + dy * linha++
                ));
            }
        }

        window.display();
    }

    // encerra a simulacao da mina antes de sair
    mina.parar();
    std::cout << "GUI Gestao da Mina encerrada\n";
    return 0;
}