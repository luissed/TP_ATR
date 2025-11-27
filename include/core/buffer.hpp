#ifndef CORE_BUFFER_HPP
#define CORE_BUFFER_HPP

#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>

template<typename T>
class BufferCircular {
private:
    std::vector<T> buffer;
    size_t capacidade;
    size_t inicio;
    size_t fim;
    size_t contador;
    
    mutable std::mutex mtx;
    std::condition_variable cv_cheio;
    std::condition_variable cv_vazio;

public:
    BufferCircular(size_t cap = 200) : capacidade(cap), inicio(0), fim(0), contador(0) {
        buffer.resize(capacidade);
    }

    void escrever(const T& item) {
        std::unique_lock<std::mutex> lock(mtx);
        cv_cheio.wait(lock, [this]() { return contador < capacidade; });
        
        buffer[fim] = item;
        fim = (fim + 1) % capacidade;
        contador++;
        
        cv_vazio.notify_one();
    }

    bool ler(T& item) {
        std::unique_lock<std::mutex> lock(mtx);
        
        if (!cv_vazio.wait_for(lock, std::chrono::milliseconds(100), 
            [this]() { return contador > 0; })) {
            return false;
        }
        
        item = buffer[inicio];
        inicio = (inicio + 1) % capacidade;
        contador--;
        
        cv_cheio.notify_one();
        return true;
    }

    size_t tamanho() const {
        std::lock_guard<std::mutex> lock(mtx);
        return contador;
    }

    bool vazio() const {
        std::lock_guard<std::mutex> lock(mtx);
        return contador == 0;
    }

    bool cheio() const {
        std::lock_guard<std::mutex> lock(mtx);
        return contador == capacidade;
    }
};

#endif