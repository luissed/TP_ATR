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

// ... (Estrutura CaminhaoDrawInfo e função criarBotaoEstiloso permanecem iguais) ...
struct CaminhaoDrawInfo {
    int   id;
    bool  temDados;
    float xTela;
    float yTela;
};

// Funcao auxiliar visual para criar rects com borda (apenas estetica)
sf::RectangleShape criarBotaoEstiloso(sf::Vector2f tamanho, sf::Vector2f pos, sf::Color corBase) {
    sf::RectangleShape shape(tamanho);
    shape.setPosition(pos);
    shape.setFillColor(corBase);
    shape.setOutlineColor(sf::Color(255, 255, 255, 150));
    shape.setOutlineThickness(1.0f);
    return shape;
}


int main() {
    std::cout << "GUI Gestao da Mina\n";

    // comeca sem caminhoes
    SimulacaoMina mina(0, 200);

    // inicia threads internas se nao houver caminhoes nao faz nada por enquanto
    mina.iniciar();

    // configuracao do mapa
    const int   WINDOW_WIDTH  = 1920;
    const int   WINDOW_HEIGHT = 1080;
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

    // Estado de visibilidade do painel de informações
    bool painelVisivel = true;

    // --- ESTILIZACAO DO BOTAO NOVO ---
    sf::RectangleShape botaoNovo(sf::Vector2f(40.f, 30.f));
    botaoNovo.setPosition(20.f, 20.f);
    botaoNovo.setFillColor(sf::Color(46, 204, 113)); // Verde Emerald
    botaoNovo.setOutlineColor(sf::Color::White);
    botaoNovo.setOutlineThickness(2.f);
    sf::RectangleShape sombraBotaoNovo = botaoNovo;
    sombraBotaoNovo.setPosition(23.f, 23.f);
    sombraBotaoNovo.setFillColor(sf::Color(0,0,0,100));

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

    // Botão de Ocultar/Exibir Painel
    const float btnInfoWidth = 60.f;
    const float btnInfoHeight = 25.f;
    sf::RectangleShape botaoInfo(sf::Vector2f(btnInfoWidth, btnInfoHeight));
    botaoInfo.setPosition(WINDOW_WIDTH - btnInfoWidth - 10.f, 20.f); 
    botaoInfo.setFillColor(sf::Color(70, 80, 100)); 
    botaoInfo.setOutlineColor(sf::Color::White);
    botaoInfo.setOutlineThickness(1.f);

    // fonte para textos do painel lateral
    sf::Font fonte;
    bool fonteOk = false;
    // O caminho da fonte é específico do seu sistema. Mantenho o original:
    if (fonte.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        fonteOk = true;
    } else {
        std::cout << "[GUI] Nao foi possivel carregar fonte. Painel sera sem texto.\n";
    }

    auto criarTexto = [&](const std::string& s, float x, float y, int size = 14, sf::Color cor = sf::Color::White) {
        sf::Text txt;
        txt.setFont(fonte);
        txt.setString(s);
        txt.setCharacterSize(size);
        txt.setFillColor(cor);
        txt.setPosition(x, y);
        txt.setOutlineColor(sf::Color::Black);
        txt.setOutlineThickness(1.0f);
        return txt;
    };

    // painel lateral de telemetria e comandos
    const float painelWidth  = 260.f;
    const float painelHeight = 300.f; // Tamanho original mantido
    sf::RectangleShape painelInfo(sf::Vector2f(painelWidth, painelHeight));
    painelInfo.setPosition(WINDOW_WIDTH - painelWidth - 10.f,
                           botaoInfo.getPosition().y + botaoInfo.getSize().y + 10.f);
    painelInfo.setFillColor(sf::Color(30, 35, 45, 230));
    painelInfo.setOutlineColor(sf::Color(100, 100, 120));
    painelInfo.setOutlineThickness(1.f);

    float painelX = painelInfo.getPosition().x;
    float painelY = painelInfo.getPosition().y;

    // BOTOES DO PAINEL
    float linha1Y = painelY + 15.f;
    float colEsqX = painelX + 15.f;
    float colDirX = colEsqX + 90.f;

    sf::RectangleShape painelBotaoAuto    = criarBotaoEstiloso(sf::Vector2f(80.f, 24.f), sf::Vector2f(colEsqX, linha1Y), sf::Color::Black);
    sf::RectangleShape painelBotaoManual = criarBotaoEstiloso(sf::Vector2f(80.f, 24.f), sf::Vector2f(colDirX, linha1Y), sf::Color::Black);

    float linha2Y = linha1Y + 35.f;
    sf::RectangleShape painelBotaoRearme = criarBotaoEstiloso(sf::Vector2f(80.f, 24.f), sf::Vector2f(colEsqX, linha2Y), sf::Color::Black);

    float falhaX = colDirX;
    float falhaY = linha2Y;

    sf::RectangleShape painelBotaoFalhaTemp = criarBotaoEstiloso(sf::Vector2f(90.f, 22.f), sf::Vector2f(falhaX, falhaY), sf::Color::Black);
    sf::RectangleShape painelBotaoFalhaElec = criarBotaoEstiloso(sf::Vector2f(90.f, 22.f), sf::Vector2f(falhaX, falhaY + 26.f), sf::Color::Black);
    sf::RectangleShape painelBotaoFalhaHid  = criarBotaoEstiloso(sf::Vector2f(90.f, 22.f), sf::Vector2f(falhaX, falhaY + 52.f), sf::Color::Black);

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::MouseButtonPressed &&
                     event.mouseButton.button == sf::Mouse::Left) {

                sf::Vector2i pixel(event.mouseButton.x, event.mouseButton.y);
                sf::Vector2f pixelF(static_cast<float>(pixel.x),
                                    static_cast<float>(pixel.y));
                
                // Botão INFO (mostrar/ocultar painel)
                if (botaoInfo.getGlobalBounds().contains(pixelF)) {
                    painelVisivel = !painelVisivel;
                    std::cout << "[GUI] Botao INFO clicado. Painel Visivel=" << painelVisivel << "\n";
                    continue;
                }

                if (botaoNovo.getGlobalBounds().contains(pixelF)) {
                    int novoId = mina.criarNovoCaminhao();
                    std::cout << "[GUI] Botao + clicado. Novo caminhao id=" << novoId << "\n";
                    if (mina.quantidadeCaminhoes() == 1) {
                        idSelecionado = novoId;
                        painelVisivel = true;
                    }
                    continue;
                }

                if (idSelecionado != -1 && painelVisivel) {
                    if (painelBotaoAuto.getGlobalBounds().contains(pixelF)) {
                        mina.getCaminhaoPorId(idSelecionado).comandarAutomatico();
                        continue;
                    }
                    if (painelBotaoManual.getGlobalBounds().contains(pixelF)) {
                        mina.getCaminhaoPorId(idSelecionado).comandarManual();
                        continue;
                    }
                    if (painelBotaoRearme.getGlobalBounds().contains(pixelF)) {
                        mina.getCaminhaoPorId(idSelecionado).comandarRearme();
                        continue;
                    }
                    if (painelBotaoFalhaTemp.getGlobalBounds().contains(pixelF)) {
                        mina.injetarFalhaTemperatura(idSelecionado);
                        continue;
                    }
                    if (painelBotaoFalhaElec.getGlobalBounds().contains(pixelF)) {
                        mina.injetarFalhaEletrica(idSelecionado);
                        continue;
                    }
                    if (painelBotaoFalhaHid.getGlobalBounds().contains(pixelF)) {
                        mina.injetarFalhaHidraulica(idSelecionado);
                        continue;
                    }
                }

                // Clique no mapa (selecionar ou definir nova rota)
                std::vector<CaminhaoDrawInfo> infos;
                infos.reserve(mina.quantidadeCaminhoes());

                for (std::size_t i = 0; i < mina.quantidadeCaminhoes(); ++i) {
                    Caminhao& c = mina.getCaminhao(i);
                    RegistroBuffer reg{};
                    bool ok = c.lerUltimoRegistro(reg);
                    CaminhaoDrawInfo info{};
                    info.id      = c.getId();
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

                const float LIMITE_CLICK = 20.0f;
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
                    painelVisivel = true;
                } else if (idSelecionado != -1) {
                    double worldX = (static_cast<double>(pixel.x) - ORIGEM_X) / SCALE;
                    double worldY = (ORIGEM_Y - static_cast<double>(pixel.y)) / SCALE;
                    Caminhao& cSel = mina.getCaminhaoPorId(idSelecionado);
                    RegistroBuffer reg{};
                    if (cSel.lerUltimoRegistro(reg)) {
                        cSel.definirRota(reg.sensores.i_posicao_x, reg.sensores.i_posicao_y,
                                         static_cast<int>(std::lround(worldX)),
                                         static_cast<int>(std::lround(worldY)));
                        cSel.comandarAutomatico();
                        painelVisivel = true;
                    }
                }
            }
            else if (event.type == sf::Event::KeyPressed) {
                if (idSelecionado != -1) {
                    Caminhao& cSel = mina.getCaminhaoPorId(idSelecionado);
                    if (event.key.code == sf::Keyboard::W) cSel.setComandoAcelerar(true);
                    if (event.key.code == sf::Keyboard::A) { cSel.setComandoEsquerda(true); cSel.setComandoDireita(false); }
                    if (event.key.code == sf::Keyboard::D) { cSel.setComandoDireita(true); cSel.setComandoEsquerda(false); }
                }
            }
            else if (event.type == sf::Event::KeyReleased) {
                if (idSelecionado != -1) {
                    Caminhao& cSel = mina.getCaminhaoPorId(idSelecionado);
                    if (event.key.code == sf::Keyboard::W) cSel.setComandoAcelerar(false);
                    if (event.key.code == sf::Keyboard::A) cSel.setComandoEsquerda(false);
                    if (event.key.code == sf::Keyboard::D) cSel.setComandoDireita(false);
                }
            }
        }

        // --- DESENHO ---         
        window.clear(sf::Color(210, 200, 180)); 

        // GRID
        sf::Color gridColor(0, 0, 0, 20);
        for (int x = 0; x < WINDOW_WIDTH; x += 40) {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(static_cast<float>(x), 0.f), gridColor),
                sf::Vertex(sf::Vector2f(static_cast<float>(x), static_cast<float>(WINDOW_HEIGHT)), gridColor)
            };
            window.draw(line, 2, sf::Lines);
        }
        for (int y = 0; y < WINDOW_HEIGHT; y += 40) {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(0.f, static_cast<float>(y)), gridColor),
                sf::Vertex(sf::Vector2f(static_cast<float>(WINDOW_WIDTH), static_cast<float>(y)), gridColor)
            };
            window.draw(line, 2, sf::Lines);
        }

        // Eixos
        sf::Vertex eixoX[] = {
            sf::Vertex(sf::Vector2f(0.f, ORIGEM_Y), sf::Color(100, 100, 100, 150)),
            sf::Vertex(sf::Vector2f(static_cast<float>(WINDOW_WIDTH), ORIGEM_Y), sf::Color(100, 100, 100, 150))
        };
        sf::Vertex eixoY[] = {
            sf::Vertex(sf::Vector2f(ORIGEM_X, 0.f), sf::Color(100, 100, 100, 150)),
            sf::Vertex(sf::Vector2f(ORIGEM_X, static_cast<float>(WINDOW_HEIGHT)), sf::Color(100, 100, 100, 150))
        };
        window.draw(eixoX, 2, sf::Lines);
        window.draw(eixoY, 2, sf::Lines);

        // Cava da mina
        {
            float cavaX = ORIGEM_X - 150.f;
            float cavaY = ORIGEM_Y + 100.f;
            
            sf::CircleShape cava1(130.f);
            cava1.setFillColor(sf::Color(160, 130, 90));
            cava1.setOrigin(130.f, 130.f);
            cava1.setPosition(cavaX, cavaY);
            window.draw(cava1);

            sf::CircleShape cava2(100.f);
            cava2.setFillColor(sf::Color(130, 100, 70));
            cava2.setOrigin(100.f, 100.f);
            cava2.setPosition(cavaX, cavaY);
            window.draw(cava2);

            sf::CircleShape cava3(70.f);
            cava3.setFillColor(sf::Color(90, 60, 40));
            cava3.setOrigin(70.f, 70.f);
            cava3.setPosition(cavaX, cavaY);
            window.draw(cava3);

            if (fonteOk) {
                sf::Text label = criarTexto("AREA DE LAVRA", cavaX - 50, cavaY - 10, 12, sf::Color(255,255,255,150));
                window.draw(label);
            }
        }

        // Britador
        {
            float britX = ORIGEM_X + 200.f;
            float britY = ORIGEM_Y - 150.f;
            
            sf::RectangleShape base(sf::Vector2f(160.f, 120.f));
            base.setFillColor(sf::Color(120, 128, 130));
            base.setOutlineColor(sf::Color(60, 60, 60));
            base.setOutlineThickness(2.f);
            base.setOrigin(80.f, 60.f);
            base.setPosition(britX, britY);
            window.draw(base);

            sf::RectangleShape hopper(sf::Vector2f(60.f, 40.f));
            hopper.setFillColor(sf::Color(50, 50, 60));
            hopper.setOrigin(30.f, 20.f);
            hopper.setPosition(britX, britY);
            window.draw(hopper);

            sf::RectangleShape esteira(sf::Vector2f(100.f, 10.f));
            esteira.setFillColor(sf::Color(40, 40, 40));
            esteira.setOrigin(0.f, 5.f);
            esteira.setPosition(britX, britY);
            esteira.setRotation(-45.f);
            window.draw(esteira);

            if (fonteOk) {
                sf::Text label = criarTexto("BRITADOR PRIMARIO", britX - 60, britY + 40, 12, sf::Color::White);
                window.draw(label);
            }
        }

        // Botão Novo
        window.draw(sombraBotaoNovo);
        window.draw(botaoNovo);
        desenharMais(window);

        // Botao INFO
        sf::Color infoBtnColor = painelVisivel ? sf::Color(100, 100, 100) : sf::Color(70, 80, 100);
        botaoInfo.setFillColor(infoBtnColor);
        window.draw(botaoInfo);
        if (fonteOk) {
            sf::Text infoTxt = criarTexto(painelVisivel ? "OCULTAR" : "INFO", 0, 0, 12);
            sf::FloatRect bounds = botaoInfo.getLocalBounds();
            sf::FloatRect textBounds = infoTxt.getLocalBounds();
            infoTxt.setPosition(
                botaoInfo.getPosition().x + (bounds.width - textBounds.width) / 2.0f,
                botaoInfo.getPosition().y + (bounds.height - textBounds.height) / 2.0f - 2.0f
            );
            window.draw(infoTxt);
        }

        RegistroBuffer regSel{};
        bool temRegSel = false;

        // Desenha Caminhoes
        for (std::size_t i = 0; i < mina.quantidadeCaminhoes(); ++i) {
            Caminhao& c = mina.getCaminhao(i);

            RegistroBuffer reg{};
            if (!c.lerUltimoRegistro(reg)) continue;

            float xTela = ORIGEM_X + static_cast<float>(reg.sensores.i_posicao_x) * SCALE;
            float yTela = ORIGEM_Y - static_cast<float>(reg.sensores.i_posicao_y) * SCALE;

            bool selecionado = (c.getId() == idSelecionado);
            bool modoAuto    = reg.estados.e_automatico;
            
            float rotacaoVisual = -static_cast<float>(reg.sensores.i_angulo_x);

            sf::CircleShape sombra(14.f);
            sombra.setScale(1.5f, 0.8f);
            sombra.setFillColor(sf::Color(0,0,0,60));
            sombra.setOrigin(14.f, 14.f);
            sombra.setPosition(xTela + 4.f, yTela + 4.f);
            sombra.setRotation(rotacaoVisual);
            window.draw(sombra);

            sf::RectangleShape corpo(sf::Vector2f(32.f, 18.f));
            corpo.setOrigin(16.f, 9.f);
            corpo.setPosition(xTela, yTela);
            corpo.setRotation(rotacaoVisual);
            
            if (modoAuto) {
                corpo.setFillColor(sf::Color(255, 204, 0)); // Caterpillar Yellow
            } else {
                corpo.setFillColor(sf::Color(230, 80, 0));  // Manual: laranja
            }
            
            if (selecionado) {
                corpo.setOutlineColor(sf::Color::White);
                corpo.setOutlineThickness(2.f);
            } else {
                corpo.setOutlineThickness(0.f);
            }

            sf::RectangleShape cacamba(sf::Vector2f(20.f, 14.f));
            cacamba.setOrigin(10.f - 5.f, 7.f);
            cacamba.setPosition(xTela, yTela);
            cacamba.setRotation(rotacaoVisual);
            cacamba.setFillColor(sf::Color(0,0,0,30));
            
            sf::RectangleShape cabine(sf::Vector2f(8.f, 12.f));
            cabine.setOrigin(4.f - 10.f, 6.f);
            cabine.setPosition(xTela, yTela);
            cabine.setRotation(rotacaoVisual);
            cabine.setFillColor(sf::Color(50, 50, 50));

            sf::RectangleShape vidro(sf::Vector2f(4.f, 10.f));
            vidro.setOrigin(2.f - 11.f, 5.f);
            vidro.setPosition(xTela, yTela);
            vidro.setRotation(rotacaoVisual);
            vidro.setFillColor(sf::Color(100, 200, 255));

            auto desenharPneu = [&](float offX, float offY) {
                sf::RectangleShape p(sf::Vector2f(10.f, 4.f));
                p.setFillColor(sf::Color(20, 20, 20));
                p.setOrigin(5.f - offX, 2.f - offY);
                p.setPosition(xTela, yTela);
                p.setRotation(rotacaoVisual);
                window.draw(p);
            };
            desenharPneu(8.f, 10.f);
            desenharPneu(8.f, -10.f);
            desenharPneu(-8.f, 10.f);
            desenharPneu(-8.f, -10.f);

            window.draw(corpo);
            window.draw(cacamba);
            window.draw(cabine);
            window.draw(vidro);

            if (selecionado) {
                regSel  = reg;
                temRegSel = true;
                
                if (modoAuto) {
                     float spX = ORIGEM_X + static_cast<float>(reg.setpoints.sp_posicao_x) * SCALE;
                     float spY = ORIGEM_Y - static_cast<float>(reg.setpoints.sp_posicao_y) * SCALE;
                     
                     sf::Vertex linhaRota[] = {
                          sf::Vertex(sf::Vector2f(xTela, yTela), sf::Color(0, 255, 0, 100)),
                          sf::Vertex(sf::Vector2f(spX, spY),      sf::Color(0, 255, 0, 100))
                     };
                     window.draw(linhaRota, 2, sf::Lines);

                     sf::CircleShape alvo(3.f);
                     alvo.setFillColor(sf::Color::Green);
                     alvo.setPosition(spX-3, spY-3);
                     window.draw(alvo);
                }
            }
        }

        // Painel Lateral (HUD)
        if (painelVisivel && idSelecionado != -1 && temRegSel) {
            sf::Color corAtiva   = sf::Color(46, 204, 113);
            sf::Color corInativa = sf::Color(80, 80, 80);
            sf::Color corErro    = sf::Color(231, 76, 60);

            painelBotaoAuto.setFillColor(regSel.estados.e_automatico ? corAtiva : corInativa);
            painelBotaoManual.setFillColor(!regSel.estados.e_automatico ? sf::Color(230, 126, 34) : corInativa);

            // >>> AQUI ESTÁ A MUDANÇA IMPORTANTE <<<
            // Botão de REARME acende se:
            //  - houver defeito OU
            //  - houver bloqueio de rearme (manual->auto precisa de rearme)
            bool precisaRearme = regSel.estados.e_defeito || regSel.estados.e_bloqueio_rearme;

            if (precisaRearme) {
                 painelBotaoRearme.setFillColor(sf::Color(241, 196, 15)); // Amarelo
                 painelBotaoRearme.setOutlineColor(sf::Color::Red);
            } else {
                 painelBotaoRearme.setFillColor(corInativa);
                 painelBotaoRearme.setOutlineColor(sf::Color::White);
            }

            painelBotaoFalhaTemp.setFillColor(regSel.sensores.i_temperatura > 120 ? corErro : sf::Color(60, 40, 40));
            painelBotaoFalhaElec.setFillColor(regSel.sensores.i_falha_eletrica ? corErro : sf::Color(60, 40, 40));
            painelBotaoFalhaHid.setFillColor(regSel.sensores.i_falha_hidraulica ? corErro : sf::Color(60, 40, 40));

            window.draw(painelInfo);

            window.draw(painelBotaoAuto);
            window.draw(painelBotaoManual);
            window.draw(painelBotaoRearme);
            
            // --- INÍCIO DA MUDANÇA (Separação visual) ---
            // A linha separadora deve vir depois dos botões de falha
            sf::RectangleShape linhaSep(sf::Vector2f(painelWidth - 20.f, 1.f));
            linhaSep.setPosition(painelX + 10.f, falhaY + 52.f + 30.f); // Posição ajustada para ficar após o último botão de falha
            linhaSep.setFillColor(sf::Color(100, 100, 100));
            window.draw(linhaSep);
            // --- FIM DA MUDANÇA ---

            window.draw(painelBotaoFalhaTemp);
            window.draw(painelBotaoFalhaElec);
            window.draw(painelBotaoFalhaHid);

            if (fonteOk) {
                auto drawBtnText = [&](const std::string& txt, sf::RectangleShape& shape) {
                    sf::FloatRect bounds = shape.getLocalBounds();
                    sf::Text t = criarTexto(txt, 0, 0, 12);
                    sf::FloatRect textBounds = t.getLocalBounds();
                    t.setPosition(
                        shape.getPosition().x + (bounds.width - textBounds.width)/2.0f,
                        shape.getPosition().y + (bounds.height - textBounds.height)/2.0f - 2.0f
                    );
                    window.draw(t);
                };

                drawBtnText("AUTO",   painelBotaoAuto);
                drawBtnText("MANUAL", painelBotaoManual);
                drawBtnText("REARME", painelBotaoRearme);
                
                drawBtnText("F. TEMP", painelBotaoFalhaTemp);
                drawBtnText("F. ELEC", painelBotaoFalhaElec);
                drawBtnText("F. HIDR", painelBotaoFalhaHid);

                float xBase = painelX + 15.f;
                // --- INÍCIO DA MUDANÇA (Posição vertical dos dados) ---
                // Agora, yBase é calculado para começar logo após a linha separadora
                float yBase = linhaSep.getPosition().y + 10.f; 
                // --- FIM DA MUDANÇA ---
                float dy    = 18.f;
                int l       = 0;

                sf::Text titulo = criarTexto("CAMINHAO #" + std::to_string(idSelecionado),
                                             xBase, yBase, 16, sf::Color(100, 200, 255));
                window.draw(titulo);
                yBase += 25.f; // Ajuste para a próxima linha de dados começar 25 pixels abaixo do título

                auto desenharDado = [&](std::string label, std::string val, sf::Color corVal = sf::Color::White) {
                    window.draw(criarTexto(label, xBase, yBase + l*dy, 12, sf::Color(180, 180, 180)));
                    // --- MUDANÇA DE ALINHAMENTO DO VALOR ---
                    // Alinha o valor um pouco mais para a direita para evitar sobreposição
                    window.draw(criarTexto(val,   xBase + 100.f, yBase + l*dy, 12, corVal)); 
                    // --- FIM DA MUDANÇA ---
                    l++;
                };

                desenharDado("Estado:", estadoToString(regSel.estado));
                
                sf::Color corDefeito = regSel.estados.e_defeito ? sf::Color::Red : sf::Color::Green;
                desenharDado("Defeito:", regSel.estados.e_defeito ? "SIM" : "NAO", corDefeito);
                
                desenharDado("Pos (X,Y):",
                             std::to_string(regSel.sensores.i_posicao_x) + ", " +
                             std::to_string(regSel.sensores.i_posicao_y));
                desenharDado("Angulo:", std::to_string(regSel.sensores.i_angulo_x) + " deg");
                
                sf::Color corTemp = regSel.sensores.i_temperatura > 100 ? sf::Color(255, 100, 100) : sf::Color::White;
                desenharDado("Temp:", std::to_string(regSel.sensores.i_temperatura) + " C", corTemp);
                
                l++;
                
                // --- MUDANÇA DE POSICIONAMENTO DA BARRA DE ACELERAÇÃO ---
                // Ajusta a posição da barra para que ela fique mais alinhada com os outros dados
                window.draw(criarTexto("Acel:", xBase, yBase + l*dy + 3.f, 12)); 
                sf::RectangleShape barraFundo(sf::Vector2f(100.f, 6.f));
                barraFundo.setPosition(xBase + 100.f, yBase + l*dy + 6.f); // Ajuste de X para alinhamento
                barraFundo.setFillColor(sf::Color(50,50,50));
                window.draw(barraFundo);
                
                float pct = std::abs(regSel.atuadores.o_aceleracao) / 100.0f;
                if (pct > 1.0f) pct = 1.0f;
                sf::RectangleShape barra(sf::Vector2f(100.f * pct, 6.f));
                barra.setPosition(xBase + 100.f, yBase + l*dy + 6.f); // Ajuste de X para alinhamento
                barra.setFillColor(regSel.atuadores.o_aceleracao >= 0 ? sf::Color::Cyan : sf::Color::Magenta);
                window.draw(barra);
                // --- FIM DA MUDANÇA ---
            }
        }

        window.display();
    }

    mina.parar();
    std::cout << "GUI Gestao da Mina encerrada\n";
    return 0;
}