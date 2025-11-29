#include "../../include/caminhao/controle_navegacao.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

ControleNavegacao::ControleNavegacao(CaminhaoContext& contexto, Buffer& buf) 
    : ctx(contexto), buffer(buf) {}

void ControleNavegacao::executar() {
    // 1. Verifica Modo de Operação
    // Se não estiver em automático ou se houver defeito, para o caminhão (segurança)
    if (!ctx.info.estados.e_automatico || ctx.info.estados.e_defeito) {
        ctx.info.atuadores.o_aceleracao = 0;
        // Mantém a direção atual ou zera
        return; 
    }

    // 2. Leitura de Variáveis
    int x_atual = ctx.info.sensores.i_posicao_x;
    int y_atual = ctx.info.sensores.i_posicao_y;
    int angulo_atual = ctx.info.sensores.i_angulo_x;

    int x_dest = ctx.info.navegacao.x_setpoint;
    int y_dest = ctx.info.navegacao.y_setpoint;

    // 3. Cálculos de Erro
    double dx = x_dest - x_atual;
    double dy = y_dest - y_atual;
double distancia = std::sqrt(static_cast<double>(dx*dx + dy*dy));
    // 4. Lógica de "Chegou no Destino"
    if (distancia < DISTANCIA_ALVO) {
        ctx.info.atuadores.o_aceleracao = 0;
        
        // Só avisa que chegou se ainda não tiver avisado (borda de subida)
        if (!ctx.info.navegacao.chegou) {
            ctx.info.navegacao.chegou = true;
            // Atualiza o buffer global para o Planejamento saber que pode mandar o próximo
            buffer.atualizarSetpoint(ctx.info.id_caminhao, x_dest, y_dest); 
            // Nota: Precisamos de um método no Buffer para sinalizar 'chegou', 
            // ou o Planejamento monitora a posição vs setpoint.
            // Por simplicidade, assumimos que o buffer reflete o estado local.
        }
        return;
    } 
    
    // Se saiu do raio de tolerância, reseta flag
    ctx.info.navegacao.chegou = false;

    // 5. Cálculo de Direção (Heading)
    // atan2 retorna radianos entre -PI e PI. Converter para graus.
    double angulo_desejado_rad = std::atan2(dy, dx);
    int angulo_desejado_graus = static_cast<int>(angulo_desejado_rad * 180.0 / M_PI);

    // Erro angular (normalizado para -180 a 180)
    int erro_ang = angulo_desejado_graus - angulo_atual;
    while (erro_ang > 180) erro_ang -= 360;
    while (erro_ang < -180) erro_ang += 360;

    // 6. Aplicação do Controle (Atuadores)
    
    // Direção: Proporcional ao erro angular
    int comando_dir = static_cast<int>(erro_ang * KP_DIRECAO);
    // Trava entre -100% e 100% (ou graus, depende da especificação do servo)
    // Assumindo input direto em graus reltivos ou força de esterçamento:
    comando_dir = std::max(-180, std::min(180, comando_dir));

    // Aceleração: Se o erro angular for muito grande, desacelera para fazer a curva
    int comando_acel = static_cast<int>(100 * KP_VELOCIDADE); // Aceleração base
    if (std::abs(erro_ang) > 45) {
        comando_acel /= 4; // Reduz a 25% se a curva for fechada
    } else if (std::abs(erro_ang) > 20) {
        comando_acel /= 2; // Reduz a 50%
    }
    
    // Trava em 100%
    comando_acel = std::max(0, std::min(100, comando_acel));

    // 7. Escreve nos Atuadores Locais
    ctx.info.atuadores.o_aceleracao = comando_acel;
    ctx.info.atuadores.o_direcao = comando_dir;

    // 8. Opcional: Atualizar Buffer Global imediatamente (ou deixar pro Coletor)
    buffer.atualizarAtuadores(ctx.info.id_caminhao, comando_acel, comando_dir);
}