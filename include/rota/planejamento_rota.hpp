#ifndef PLANEJAMENTO_ROTA_HPP
#define PLANEJAMENTO_ROTA_HPP

#include "../core/RegistroBuffer.hpp"
#include "../core/Buffer.hpp"
#include "../core/tipos.hpp"
#include <cmath>

class PlanejamentoRota {
public:
    PlanejamentoRota(BufferCircular& buffer, int id_caminhao);

    void loop();  // thread principal do roteador

private:
    BufferCircular& buffer_;
    int id_;

    float distancia(float x1, float y1, float x2, float y2);
};

#endif
