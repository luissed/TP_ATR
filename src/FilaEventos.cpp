#include "FilaEventos.hpp"

// posta um evento na fila e acorda uma thread que estiver esperando
void FilaEventos::postar(const Evento& evento) {
    {
        std::lock_guard<std::mutex> lock(mtx_);
        fila_.push(evento);
    }
    cv_.notify_one();
}

// espera ate existir pelo menos um evento na fila e devolve o primeiro
Evento FilaEventos::esperarProximo() {
    std::unique_lock<std::mutex> lock(mtx_);
    cv_.wait(lock, [this] { return !fila_.empty(); });
    Evento ev = fila_.front();
    fila_.pop();
    return ev;
}

// tenta pegar um evento sem bloquear, retorna false se a fila estiver vazia
bool FilaEventos::tentarRetirar(Evento& out) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (fila_.empty()) {
        return false;
    }
    out = fila_.front();
    fila_.pop();
    return true;
}

std::size_t FilaEventos::tamanho() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return fila_.size();
}