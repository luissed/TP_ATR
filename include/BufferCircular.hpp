#pragma once

#include <vector>
#include <mutex>
#include <cstddef>   // std::size_t
#include "Tipos.hpp"

// Buffer circular monitorado para compartilhar dados dentro de UM caminhão.
// Várias tarefas podem escrever e ler, mas o acesso é protegido por mutex.
class BufferCircular {
public:
    explicit BufferCircular(std::size_t capacidade);

    // Insere um novo registro no buffer.
    // Se estiver cheio, sobrescreve o mais antigo (comportamento típico de buffer circular).
    void inserir(const RegistroBuffer& registro);

    // Tenta ler o registro mais recente.
    // Retorna true se conseguiu, false se o buffer estiver vazio.
    bool tentarLerMaisRecente(RegistroBuffer& out) const;

    // Retorna uma cópia de todos os registros atualmente no buffer, em ordem do mais antigo para o mais recente.
    std::vector<RegistroBuffer> snapshot() const;

    std::size_t tamanho() const;
    std::size_t capacidade() const;

private:
    mutable std::mutex mtx_;
    std::vector<RegistroBuffer> dados_;
    std::size_t capacidade_;
    std::size_t inicio_;      // índice do elemento mais antigo
    std::size_t quantidade_;  // quantidade de elementos válidos no buffer
};
