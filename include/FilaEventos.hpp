#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <cstddef>
#include "Tipos.hpp"

// Fila de eventos thread-safe.
// Permite que uma tarefa poste eventos e outras esperem por eles.
class FilaEventos {
public:
    FilaEventos() = default;

    // Posta (insere) um novo evento na fila e acorda quem estiver esperando.
    void postar(const Evento& evento);

    // Bloqueia até haver pelo menos um evento, e então retorna o primeiro.
    Evento esperarProximo();

    // Tenta retirar um evento sem bloquear.
    // Retorna true se conseguiu pegar algum evento.
    bool tentarRetirar(Evento& out);

    std::size_t tamanho() const;

private:
    mutable std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<Evento> fila_;
};
