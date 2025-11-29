#pragma once

#include <vector>
#include <mutex>
#include <cstddef>
#include "Tipos.hpp"

class BufferCircular {
public:
    explicit BufferCircular(std::size_t capacidade);

    void inserir(const RegistroBuffer& registro);

    bool tentarLerMaisRecente(RegistroBuffer& out) const;

    std::vector<RegistroBuffer> snapshot() const;

    std::size_t tamanho() const;
    std::size_t capacidade() const;

private:
    mutable std::mutex mtx_;
    std::vector<RegistroBuffer> dados_;
    std::size_t capacidade_;
    std::size_t inicio_;
    std::size_t quantidade_;
};
