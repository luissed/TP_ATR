// include/BufferCircular.hpp
#pragma once

#include <vector>
#include <mutex>
#include <cstddef>   // tipo size_t da biblioteca padrao
#include "Tipos.hpp"

// buffer circular monitorado para compartilhar dados dentro de um caminhao
// varias tarefas podem escrever e ler, acesso protegido por mutex
class BufferCircular {
public:
    explicit BufferCircular(std::size_t capacidade);

    // insere um novo registro no buffer
    // se estiver cheio, sobrescreve o mais antigo, comportamento tipico de buffer circular
    void inserir(const RegistroBuffer& registro);

    // tenta ler o registro mais recente
    // retorna true quando consegue, retorna false quando o buffer esta vazio
    bool tentarLerMaisRecente(RegistroBuffer& out) const;

    // retorna uma copia de todos os registros atualmente no buffer
    // em ordem do mais antigo para o mais recente
    std::vector<RegistroBuffer> snapshot() const;

    std::size_t tamanho() const;
    std::size_t capacidade() const;

private:
    mutable std::mutex mtx_;
    std::vector<RegistroBuffer> dados_;
    std::size_t capacidade_;
    std::size_t inicio_; // indice do elemento mais antigo
    std::size_t quantidade_; // quantidade de elementos validos no buffer
};