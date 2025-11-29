#pragma once

#include <vector>
#include <memory>
#include <cstddef>
#include "Caminhao.hpp"

class SimulacaoMina {
public:
    SimulacaoMina(int numCaminhoes, std::size_t capacidadeBuffer = 200);

    ~SimulacaoMina();

    void iniciar();

    void parar();

    void rodarPorSegundos(int segundos);

    Caminhao& getCaminhao(std::size_t indice);
    const Caminhao& getCaminhao(std::size_t indice) const;

    std::size_t quantidadeCaminhoes() const;

private:
    std::vector<std::unique_ptr<Caminhao>> caminhoes_;
};
