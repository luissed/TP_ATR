#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <cstddef>
#include "Tipos.hpp"

class FilaEventos {
public:
    FilaEventos() = default;

void postar(const Evento& evento);

    Evento esperarProximo();

    bool tentarRetirar(Evento& out);

    std::size_t tamanho() const;

private:
    mutable std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<Evento> fila_;
};
