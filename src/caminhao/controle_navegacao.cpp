#include "controle_navegacao.hpp"
#include <cmath>
#include <thread>
#include <chrono>
#include <iostream>

ControleNavegacao::ControleNavegacao(InfoCaminhao* ctx, std::mutex* mtx)
    : ctx_(ctx),
      mtx_(mtx),
      ganho_vel_(0.5f),
      ganho_dir_(1.0f)
{
}

float ControleNavegacao::distancia(float x1, float y1, float x2, float y2) {
    return std::sqrt((x2 - x1)*(x2 - x1) + (y2 - y1)*(y2 - y1));
}

float ControleNavegacao::normalizarAngulo(float ang) {
    while (ang > 180) ang -= 360;
    while (ang < -180) ang += 360;
    return ang;
}

void ControleNavegacao::loop() {
    while (true) {

        {
            std::lock_guard<std::mutex> lock(*mtx_);

            float x  = ctx_->sensores.f_posicao_x;
            float y  = ctx_->sensores.f_posicao_y;
            float ang = ctx_->sensores.f_orientacao;

            float xs = ctx_->navegacao.x_setpoint;
            float ys = ctx_->navegacao.y_setpoint;

            float d = distancia(x, y, xs, ys);

            // Se já está perto o suficiente
            if (d < 1.0) {
                ctx_->atuadores.o_aceleracao = 0;
                ctx_->atuadores.o_direcao = 0;
            } else {
                float angSet = std::atan2(ys - y, xs - x) * 180.0 / M_PI;
                float erro = normalizarAngulo(angSet - ang);

                ctx_->atuadores.o_aceleracao = std::clamp((int)(ganho_vel_ * d), 0, 100);
                ctx_->atuadores.o_direcao    = std::clamp((int)(ganho_dir_ * erro), -45, 45);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 10 Hz
    }
}
