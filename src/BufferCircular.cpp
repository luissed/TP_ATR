// src/BufferCircular.cpp
#include "BufferCircular.hpp"

BufferCircular::BufferCircular(std::size_t capacidade)
    : dados_(capacidade),
      capacidade_(capacidade),
      inicio_(0),
      quantidade_(0) {}

void BufferCircular::inserir(const RegistroBuffer& registro) {
    std::lock_guard<std::mutex> lock(mtx_);

    if (capacidade_ == 0) {
        return; // capacidade zero entao nao faz nada
    }

    // indice onde vamos escrever eh inicio mais quantidade modulo capacidade
    std::size_t idxEscrita = (inicio_ + quantidade_) % capacidade_;
    dados_[idxEscrita] = registro;

    if (quantidade_ < capacidade_) {
        ++quantidade_;
    } else {
        // se o buffer estiver cheio avancamos o indice inicio e jogamos fora o registro mais antigo
        inicio_ = (inicio_ + 1) % capacidade_;
    }
}

bool BufferCircular::tentarLerMaisRecente(RegistroBuffer& out) const {
    std::lock_guard<std::mutex> lock(mtx_);

    if (quantidade_ == 0 || capacidade_ == 0) {
        return false;
    }

    // indice do registro mais recente eh inicio mais quantidade menos um modulo capacidade
    std::size_t idxMaisRecente = (inicio_ + quantidade_ - 1) % capacidade_;
    out = dados_[idxMaisRecente];
    return true;
}

std::vector<RegistroBuffer> BufferCircular::snapshot() const {
    std::lock_guard<std::mutex> lock(mtx_);

    std::vector<RegistroBuffer> copia;
    copia.reserve(quantidade_);

    for (std::size_t i = 0; i < quantidade_; ++i) {
        std::size_t idx = (inicio_ + i) % capacidade_;
        copia.push_back(dados_[idx]);
    }

    return copia;
}

std::size_t BufferCircular::tamanho() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return quantidade_;
}

std::size_t BufferCircular::capacidade() const {
    return capacidade_;
}
