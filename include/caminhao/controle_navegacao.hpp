#ifndef CAMINHAO_CONTROLE_NAVEGACAO_HPP
#define CAMINHAO_CONTROLE_NAVEGACAO_HPP

#include "caminhao/caminhao.hpp"

void tarefa_controle_navegacao(CaminhaoContext* ctx); // Controle de Navegação, lê ctx.info.sensores, lê ctx.info.estados, lê ctx.eventos, escreve em ctx.info.atuadores

#endif
#ifndef CAMINHAO_CONTROLE_NAVEGACAO_HPP
#define CAMINHAO_CONTROLE_NAVEGACAO_HPP

#include "caminhao.hpp"
#include "../core/buffer.hpp"

class ControleNavegacao {
private:
    CaminhaoContext& ctx; // Referência ao contexto do caminhão
    Buffer& buffer;       // Para comunicar o "chegou" ou atualizar atuadores no sistema central
    
    // Ganhos do controlador (Ajuste fino necessário depois)
    const float KP_VELOCIDADE = 1.5f;
    const float KP_DIRECAO = 0.8f;
    const int DISTANCIA_ALVO = 5; // Tolerância para considerar que chegou (metros/pixels)

public:
    ControleNavegacao(CaminhaoContext& contexto, Buffer& buf);
    
    // Método chamado ciclicamente pela thread
    void executar();
};

#endif