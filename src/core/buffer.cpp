// core/buffer.cpp

#include "core/buffer.hpp"
#include <iostream>
#include <algorithm> // Para std::min

void inicializar_buffer(SharedBuffer* buffer) {
    buffer->in = 0;
    buffer->out = 0;
    buffer->count = 0;
    std::cout << "[BUFFER] Buffer Circular inicializado com " << SharedBuffer::SIZE << " posições." << std::endl;
}

// Tarefa Produtora
void produzir_item(SharedBuffer* buffer, const InfoCaminhao& info) {
    std::unique_lock<std::mutex> lock(buffer->mtx);

    // Espera Condicional: Bloqueia se o buffer estiver cheio (count == SIZE)
    buffer->cv_vazio.wait(lock, [buffer] {
        return buffer->count < SharedBuffer::SIZE;
    });

    // --- Seção Crítica (Produção) ---
    buffer->dados[buffer->in] = info;
    buffer->in = (buffer->in + 1) % SharedBuffer::SIZE;
    buffer->count++;

    // Sinaliza Consumidores que há um novo item
    buffer->cv_cheio.notify_all(); 

    // O lock é liberado automaticamente ao sair do escopo (RAII)
}

// Tarefa Consumidora
InfoCaminhao consumir_item(SharedBuffer* buffer) {
    std::unique_lock<std::mutex> lock(buffer->mtx);
    InfoCaminhao item;

    // Espera Condicional: Bloqueia se o buffer estiver vazio (count == 0)
    buffer->cv_cheio.wait(lock, [buffer] {
        return buffer->count > 0;
    });

    // --- Seção Crítica (Consumo) ---
    item = buffer->dados[buffer->out];
    buffer->out = (buffer->out + 1) % SharedBuffer::SIZE;
    buffer->count--;
    
    // Sinaliza Produtores que há um slot vazio
    buffer->cv_vazio.notify_all();

    return item; // Retorna o item consumido (liberando o lock antes)
}