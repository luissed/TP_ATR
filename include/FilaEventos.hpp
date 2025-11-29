#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <cstddef>
#include "Tipos.hpp"

// fila de eventos thread safe usada pra troca de eventos entre tarefas
// uma tarefa posta eventos aqui e outras podem esperar ou tentar ler sem travar
class FilaEventos {
public:
    FilaEventos() = default;

    // posta um novo evento na fila e acorda quem estiver esperando
    void postar(const Evento& evento);

    // bloqueia ate existir pelo menos um evento na fila e retorna o primeiro
    Evento esperarProximo();

    // tenta retirar um evento sem bloquear
    // retorna true se conseguiu pegar algum evento
    bool tentarRetirar(Evento& out);

    // retorna o tamanho atual da fila de eventos
    std::size_t tamanho() const;

private:
    mutable std::mutex mtx_;
    std::condition_variable cv_;
    std::queue<Evento> fila_;
};