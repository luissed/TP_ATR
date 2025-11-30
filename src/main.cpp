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

    // NOVO: Estado de visibilidade do painel de informações
    bool painelVisivel = true;

    // --- ESTILIZACAO DO BOTAO NOVO (Visual melhorado, mesma logica de hit) ---
    sf::RectangleShape botaoNovo(sf::Vector2f(40.f, 30.f));
    botaoNovo.setPosition(20.f, 20.f); // Leve ajuste de margem
    botaoNovo.setFillColor(sf::Color(46, 204, 113)); // Verde Emerald
    botaoNovo.setOutlineColor(sf::Color::White);
    botaoNovo.setOutlineThickness(2.f);
    // Sombra do botao (visual apenas)
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

    // NOVO: Botão de Ocultar/Exibir Painel
    const float btnInfoWidth = 60.f;
    const float btnInfoHeight = 25.f;
    sf::RectangleShape botaoInfo(sf::Vector2f(btnInfoWidth, btnInfoHeight));
    // Posição no canto superior direito
    botaoInfo.setPosition(WINDOW_WIDTH - btnInfoWidth - 10.f, 20.f); 
    // Cor azul claro/cinza para ser diferente
    botaoInfo.setFillColor(sf::Color(70, 80, 100)); 
    botaoInfo.setOutlineColor(sf::Color::White);
    botaoInfo.setOutlineThickness(1.f);

    // fonte para textos do painel lateral
    sf::Font fonte;
    bool fonteOk = false;
    // Mantendo o path original conforme solicitado na logica
    if (fonte.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
        fonteOk = true;
    } else {
        std::cout << "[GUI] Nao foi possivel carregar fonte. "
                      "Painel sera sem texto.\n";
    }

    auto criarTexto = [&](const std::string& s, float x, float y, int size = 14, sf::Color cor = sf::Color::White) {
        sf::Text txt;
        txt.setFont(fonte);
        txt.setString(s);
        txt.setCharacterSize(size);
        txt.setFillColor(cor);
        txt.setPosition(x, y);
        // Pequena sombra no texto para leitura
        txt.setOutlineColor(sf::Color::Black);
        txt.setOutlineThickness(1.0f);
        return txt;
    };

    // painel lateral de telemetria e comandos
    const float painelWidth  = 260.f;
    const float painelHeight = 300.f; // Ajustado levemente
    
    // Fundo do painel (Visual Glass)
    sf::RectangleShape painelInfo(sf::Vector2f(painelWidth, painelHeight));
    // Ajuste a posição inicial, o painel começa abaixo do novo botão 'INFO'
    painelInfo.setPosition(WINDOW_WIDTH - painelWidth - 10.f, botaoInfo.getPosition().y + botaoInfo.getSize().y + 10.f);
    painelInfo.setFillColor(sf::Color(30, 35, 45, 230)); // Escuro semitransparente
    painelInfo.setOutlineColor(sf::Color(100, 100, 120));
    painelInfo.setOutlineThickness(1.f);

    float painelX = painelInfo.getPosition().x;
    float painelY = painelInfo.getPosition().y;

    // --- BOTOES DO PAINEL ---
    // Posicionamento mantido para logica de clique, mas visual refinado
    float linha1Y = painelY + 15.f;
    float colEsqX = painelX + 15.f;
    float colDirX = colEsqX + 90.f;

    sf::RectangleShape painelBotaoAuto = criarBotaoEstiloso(sf::Vector2f(80.f, 24.f), sf::Vector2f(colEsqX, linha1Y), sf::Color::Black);
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
            // --- LOGICA DE EVENTOS INTACTA ---
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            else if (event.type == sf::Event::MouseButtonPressed &&
                     event.mouseButton.button == sf::Mouse::Left) {

                sf::Vector2i pixel(event.mouseButton.x, event.mouseButton.y);
                sf::Vector2f pixelF(static_cast<float>(pixel.x),
                                    static_cast<float>(pixel.y));
                
                // NOVO: Lógica do botão INFO
                if (botaoInfo.getGlobalBounds().contains(pixelF)) {
                    painelVisivel = !painelVisivel;
                    std::cout << "[GUI] Botao INFO clicado. Painel Visivel=" << painelVisivel << "\n";
                    continue; // Pula outras verificações de clique
                }

                if (botaoNovo.getGlobalBounds().contains(pixelF)) {
                    int novoId = mina.criarNovoCaminhao();
                    std::cout << "[GUI] Botao + clicado. Novo caminhao id=" << novoId << "\n";
                    if (mina.quantidadeCaminhoes() == 1) {
                        idSelecionado = novoId;
                        painelVisivel = true; // Força o painel a aparecer ao criar
                    }
                    continue;
                }

                if (idSelecionado != -1 && painelVisivel) { // Adicionada a condição painelVisivel
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

                // Logica de clique no mapa (Selecionar ou Mover)
                std::vector<CaminhaoDrawInfo> infos;
                infos.reserve(mina.quantidadeCaminhoes());

                for (std::size_t i = 0; i < mina.quantidadeCaminhoes(); ++i) {
                    Caminhao& c = mina.getCaminhao(i);
                    RegistroBuffer reg{};
                    bool ok = c.lerUltimoRegistro(reg);
                    CaminhaoDrawInfo info{};
                    info.id = c.getId();
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

                const float LIMITE_CLICK = 20.0f; // Aumentei levemente a margem de click
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
                    painelVisivel = true; // Força o painel a aparecer ao selecionar
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
                        painelVisivel = true; // Força o painel a aparecer ao dar um comando
                    }
                }
            }
            else if (event.type == sf::Event::KeyPressed) {
                if (idSelecionado != -1 && !painelVisivel) { // Permite controle mesmo com painel oculto
                    Caminhao& cSel = mina.getCaminhaoPorId(idSelecionado);
                    if (event.key.code == sf::Keyboard::W) cSel.setComandoAcelerar(true);
                    if (event.key.code == sf::Keyboard::A) { cSel.setComandoEsquerda(true); cSel.setComandoDireita(false); }
                    if (event.key.code == sf::Keyboard::D) { cSel.setComandoDireita(true); cSel.setComandoEsquerda(false); }
                }
            }
            else if (event.type == sf::Event::KeyReleased) {
                if (idSelecionado != -1 && !painelVisivel) { // Permite controle mesmo com painel oculto
                    Caminhao& cSel = mina.getCaminhaoPorId(idSelecionado);
                    if (event.key.code == sf::Keyboard::W) cSel.setComandoAcelerar(false);
                    if (event.key.code == sf::Keyboard::A) cSel.setComandoEsquerda(false);
                    if (event.key.code == sf::Keyboard::D) cSel.setComandoDireita(false);
                }
            }
        }

        // --- INICIO DO DESENHO (DESIGN MELHORADO) ---
        
        // 1. Limpa com cor de solo (Bege Areia Industrial)
        window.clear(sf::Color(210, 200, 180)); 

        // 2. Desenha um GRID (Grade) estilo CAD para referencia de distancia
        // Linhas verticais e horizontais fracas
        sf::Color gridColor(0, 0, 0, 20);
        for(int x = 0; x < WINDOW_WIDTH; x += 40) { // a cada 10 metros (40px)
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f((float)x, 0.f), gridColor),
                sf::Vertex(sf::Vector2f((float)x, (float)WINDOW_HEIGHT), gridColor)
            };
            window.draw(line, 2, sf::Lines);
        }
        for(int y = 0; y < WINDOW_HEIGHT; y += 40) {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(0.f, (float)y), gridColor),
                sf::Vertex(sf::Vector2f((float)WINDOW_WIDTH, (float)y), gridColor)
            };
            window.draw(line, 2, sf::Lines);
        }

        // 3. Eixos X e Y mais fortes (estilo Blueprint)
        sf::Vertex eixoX[] = {
            sf::Vertex(sf::Vector2f(0.f, ORIGEM_Y), sf::Color(100, 100, 100, 150)),
            sf::Vertex(sf::Vector2f((float)WINDOW_WIDTH, ORIGEM_Y), sf::Color(100, 100, 100, 150))
        };
        sf::Vertex eixoY[] = {
            sf::Vertex(sf::Vector2f(ORIGEM_X, 0.f), sf::Color(100, 100, 100, 150)),
            sf::Vertex(sf::Vector2f(ORIGEM_X, (float)WINDOW_HEIGHT), sf::Color(100, 100, 100, 150))
        };
        window.draw(eixoX, 2, sf::Lines);
        window.draw(eixoY, 2, sf::Lines);


        // 4. Elementos da Mina (Cenario)
        // Zona de Mineracao (Cava) - Feita com degraus concentricos
        {
            float cavaX = ORIGEM_X - 150.f;
            float cavaY = ORIGEM_Y + 100.f;
            
            // Degrau superior
            sf::CircleShape cava1(130.f);
            cava1.setFillColor(sf::Color(160, 130, 90)); // Terra clara
            cava1.setOrigin(130.f, 130.f);
            cava1.setPosition(cavaX, cavaY);
            window.draw(cava1);

            // Degrau medio
            sf::CircleShape cava2(100.f);
            cava2.setFillColor(sf::Color(130, 100, 70)); // Terra media
            cava2.setOrigin(100.f, 100.f);
            cava2.setPosition(cavaX, cavaY);
            window.draw(cava2);

            // Fundo
            sf::CircleShape cava3(70.f);
            cava3.setFillColor(sf::Color(90, 60, 40)); // Terra escura/Buraco
            cava3.setOrigin(70.f, 70.f);
            cava3.setPosition(cavaX, cavaY);
            window.draw(cava3);

            if (fonteOk) {
                sf::Text label = criarTexto("AREA DE LAVRA", cavaX - 50, cavaY - 10, 12, sf::Color(255,255,255,150));
                window.draw(label);
            }
        }

        // Zona de Britagem/Despejo - Estrutura industrial
        {
            float britX = ORIGEM_X + 200.f;
            float britY = ORIGEM_Y - 150.f;
            
            // Base de concreto
            sf::RectangleShape base(sf::Vector2f(160.f, 120.f));
            base.setFillColor(sf::Color(120, 128, 130)); // Concreto
            base.setOutlineColor(sf::Color(60, 60, 60));
            base.setOutlineThickness(2.f);
            base.setOrigin(80.f, 60.f);
            base.setPosition(britX, britY);
            window.draw(base);

            // Hopper (Funil)
            sf::RectangleShape hopper(sf::Vector2f(60.f, 40.f));
            hopper.setFillColor(sf::Color(50, 50, 60)); // Metal escuro
            hopper.setOrigin(30.f, 20.f);
            hopper.setPosition(britX, britY);
            window.draw(hopper);

            // Esteira transportadora (linha saindo)
            sf::RectangleShape esteira(sf::Vector2f(100.f, 10.f));
            esteira.setFillColor(sf::Color(40, 40, 40));
            esteira.setOrigin(0.f, 5.f);
            esteira.setPosition(britX, britY);
            esteira.setRotation(-45.f); // Saindo na diagonal
            window.draw(esteira);

            if (fonteOk) {
                sf::Text label = criarTexto("BRITADOR PRIMARIO", britX - 60, britY + 40, 12, sf::Color::White);
                window.draw(label);
            }
        }

        // Botao Novo
        window.draw(sombraBotaoNovo); // desenha sombra antes
        window.draw(botaoNovo);
        desenharMais(window);

        // NOVO: Desenha o botão INFO e seu texto
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

        // 5. Desenha Caminhoes (Design Industrial: Amarelo CAT)
        for (std::size_t i = 0; i < mina.quantidadeCaminhoes(); ++i) {
            Caminhao& c = mina.getCaminhao(i);

            RegistroBuffer reg{};
            if (!c.lerUltimoRegistro(reg)) continue;

            float xTela = ORIGEM_X + static_cast<float>(reg.sensores.i_posicao_x) * SCALE;
            float yTela = ORIGEM_Y - static_cast<float>(reg.sensores.i_posicao_y) * SCALE;

            bool selecionado = (c.getId() == idSelecionado);
            bool modoAuto    = reg.estados.e_automatico;
            
            // Angulo (visual)
            float rotacaoVisual = -static_cast<float>(reg.sensores.i_angulo_x);

            // Sombra (oval preta transparente embaixo)
            sf::CircleShape sombra(14.f);
            sombra.setScale(1.5f, 0.8f);
            sombra.setFillColor(sf::Color(0,0,0,60));
            sombra.setOrigin(14.f, 14.f);
            sombra.setPosition(xTela + 4.f, yTela + 4.f); // leve offset
            sombra.setRotation(rotacaoVisual);
            window.draw(sombra);

            // --- CARROCERIA (Chassi) ---
            sf::RectangleShape corpo(sf::Vector2f(32.f, 18.f));
            corpo.setOrigin(16.f, 9.f);
            corpo.setPosition(xTela, yTela);
            corpo.setRotation(rotacaoVisual);
            
            // Cor baseada no modo
            if (modoAuto) {
                corpo.setFillColor(sf::Color(255, 204, 0)); // Caterpillar Yellow
            } else {
                corpo.setFillColor(sf::Color(230, 80, 0)); // Safety Orange (Manual)
            }
            
            // Destaque de selecao
            if (selecionado) {
                corpo.setOutlineColor(sf::Color::White);
                corpo.setOutlineThickness(2.f);
            } else {
                corpo.setOutlineThickness(0.f);
            }

            // --- DETALHES DO CAMINHAO ---
            
            // 1. Caçamba (Area de carga) - Parte traseira um pouco mais escura/reborda
            sf::RectangleShape cacamba(sf::Vector2f(20.f, 14.f));
            cacamba.setOrigin(10.f - 5.f, 7.f); // Deslocada para tras (esquerda visual no eixo local)
            cacamba.setPosition(xTela, yTela);
            cacamba.setRotation(rotacaoVisual);
            cacamba.setFillColor(sf::Color(0,0,0,30)); // Overlay escuro
            
            // 2. Cabine - Pequeno quadrado ciano/azul na frente
            sf::RectangleShape cabine(sf::Vector2f(8.f, 12.f));
            // A frente do caminhao é o +X (direita na rotacao 0), entao cabine vai pra +X
            cabine.setOrigin(4.f - 10.f, 6.f); // Deslocado p/ frente
            cabine.setPosition(xTela, yTela);
            cabine.setRotation(rotacaoVisual);
            cabine.setFillColor(sf::Color(50, 50, 50)); // Estrutura da cabine

            sf::RectangleShape vidro(sf::Vector2f(4.f, 10.f));
            vidro.setOrigin(2.f - 11.f, 5.f);
            vidro.setPosition(xTela, yTela);
            vidro.setRotation(rotacaoVisual);
            vidro.setFillColor(sf::Color(100, 200, 255)); // Vidro

            // 3. Pneus (Grandes, de mina)
            auto desenharPneu = [&](float offX, float offY) {
                sf::RectangleShape p(sf::Vector2f(10.f, 4.f));
                p.setFillColor(sf::Color(20, 20, 20));
                p.setOrigin(5.f - offX, 2.f - offY);
                p.setPosition(xTela, yTela);
                p.setRotation(rotacaoVisual);
                window.draw(p);
            };
            desenharPneu(8.f, 10.f);  // Frente Dir
            desenharPneu(8.f, -10.f); // Frente Esq
            desenharPneu(-8.f, 10.f); // Tras Dir
            desenharPneu(-8.f, -10.f);// Tras Esq

            // Desenha a geometria do veiculo
            window.draw(corpo);
            window.draw(cacamba);
            window.draw(cabine);
            window.draw(vidro);

            // Salva dados se for o selecionado
            if (selecionado) {
                regSel = reg;
                temRegSel = true;
                
                // Indicador de alvo (linha pontilhada ate o setpoint se existir)
                if (modoAuto) {
                     float spX = ORIGEM_X + static_cast<float>(reg.setpoints.sp_posicao_x) * SCALE;
                     float spY = ORIGEM_Y - static_cast<float>(reg.setpoints.sp_posicao_y) * SCALE;
                     
                     sf::Vertex linhaRota[] = {
                          sf::Vertex(sf::Vector2f(xTela, yTela), sf::Color(0, 255, 0, 100)),
                          sf::Vertex(sf::Vector2f(spX, spY), sf::Color(0, 255, 0, 100))
                     };
                     window.draw(linhaRota, 2, sf::Lines);

                     // X no destino
                     sf::CircleShape alvo(3.f);
                     alvo.setFillColor(sf::Color::Green);
                     alvo.setPosition(spX-3, spY-3);
                     window.draw(alvo);
                }
            }
        }

        // 6. Painel Lateral (HUD) - DESENHA SOMENTE SE VISÍVEL
        if (painelVisivel && idSelecionado != -1 && temRegSel) {
            
            // Cores de estado para botoes (Feedback visual)
            sf::Color corAtiva   = sf::Color(46, 204, 113); // Verde
            sf::Color corInativa = sf::Color(80, 80, 80);   // Cinza escuro
            sf::Color corErro    = sf::Color(231, 76, 60);  // Vermelho

            // Configura cores baseado no estado
            painelBotaoAuto.setFillColor(regSel.estados.e_automatico ? corAtiva : corInativa);
            painelBotaoManual.setFillColor(!regSel.estados.e_automatico ? sf::Color(230, 126, 34) : corInativa); // Laranja se manual
            
            // Rearme pisca ou fica amarelo se tiver defeito
            if (regSel.estados.e_defeito) {
                 painelBotaoRearme.setFillColor(sf::Color(241, 196, 15)); // Amarelo
                 painelBotaoRearme.setOutlineColor(sf::Color::Red);
            } else {
                 painelBotaoRearme.setFillColor(corInativa);
                 painelBotaoRearme.setOutlineColor(sf::Color::White);
            }

            // Falhas ativas
            painelBotaoFalhaTemp.setFillColor(regSel.sensores.i_temperatura > 120 ? corErro : sf::Color(60, 40, 40));
            painelBotaoFalhaElec.setFillColor(regSel.sensores.i_falha_eletrica ? corErro : sf::Color(60, 40, 40));
            painelBotaoFalhaHid.setFillColor(regSel.sensores.i_falha_hidraulica ? corErro : sf::Color(60, 40, 40));

            window.draw(painelInfo);

            // Botoes
            window.draw(painelBotaoAuto);
            window.draw(painelBotaoManual);
            window.draw(painelBotaoRearme);
            
            // Separador visual
            sf::RectangleShape linhaSep(sf::Vector2f(painelWidth - 20.f, 1.f));
            linhaSep.setPosition(painelX + 10.f, linha2Y + 40.f);
            linhaSep.setFillColor(sf::Color(100, 100, 100));
            window.draw(linhaSep);

            // Botoes de falha
            window.draw(painelBotaoFalhaTemp);
            window.draw(painelBotaoFalhaElec);
            window.draw(painelBotaoFalhaHid);

            if (fonteOk) {
                // Labels Botoes
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

                drawBtnText("AUTO", painelBotaoAuto);
                drawBtnText("MANUAL", painelBotaoManual);
                drawBtnText("REARME", painelBotaoRearme);
                
                drawBtnText("F. TEMP", painelBotaoFalhaTemp);
                drawBtnText("F. ELEC", painelBotaoFalhaElec);
                drawBtnText("F. HIDR", painelBotaoFalhaHid);

                // Telemetria
                float xBase = painelX + 15.f;
                float yBase = linha2Y + 50.f;
                float dy    = 18.f;
                int l = 0;

                // Titulo
                sf::Text titulo = criarTexto("CAMINHAO #" + std::to_string(idSelecionado), xBase, yBase, 16, sf::Color(100, 200, 255));
                window.draw(titulo);
                yBase += 25.f;

                auto desenharDado = [&](std::string label, std::string val, sf::Color corVal = sf::Color::White) {
                    window.draw(criarTexto(label, xBase, yBase + l*dy, 12, sf::Color(180, 180, 180)));
                    window.draw(criarTexto(val, xBase + 80.f, yBase + l*dy, 12, corVal));
                    l++;
                };

                desenharDado("Estado:", estadoToString(regSel.estado));
                
                sf::Color corDefeito = regSel.estados.e_defeito ? sf::Color::Red : sf::Color::Green;
                desenharDado("Defeito:", regSel.estados.e_defeito ? "SIM" : "NAO", corDefeito);
                
                desenharDado("Pos (X,Y):", std::to_string(regSel.sensores.i_posicao_x) + ", " + std::to_string(regSel.sensores.i_posicao_y));
                desenharDado("Angulo:", std::to_string(regSel.sensores.i_angulo_x) + " deg");
                
                sf::Color corTemp = regSel.sensores.i_temperatura > 100 ? sf::Color(255, 100, 100) : sf::Color::White;
                desenharDado("Temp:", std::to_string(regSel.sensores.i_temperatura) + " C", corTemp);
                
                // Barra de aceleracao
                l++;
                window.draw(criarTexto("Acel:", xBase, yBase + l*dy, 12));
                sf::RectangleShape barraFundo(sf::Vector2f(100.f, 6.f));
                barraFundo.setPosition(xBase + 40.f, yBase + l*dy + 6.f);
                barraFundo.setFillColor(sf::Color(50,50,50));
                window.draw(barraFundo);
                
                float pct = std::abs(regSel.atuadores.o_aceleracao) / 100.0f;
                if(pct > 1.0f) pct = 1.0f;
                sf::RectangleShape barra(sf::Vector2f(100.f * pct, 6.f));
                barra.setPosition(xBase + 40.f, yBase + l*dy + 6.f);
                barra.setFillColor(regSel.atuadores.o_aceleracao >= 0 ? sf::Color::Cyan : sf::Color::Magenta);
                window.draw(barra);
            }
        }

        window.display();
    }

    // encerra a simulacao da mina antes de sair
    mina.parar();
    std::cout << "GUI Gestao da Mina encerrada\n";
    return 0;
}