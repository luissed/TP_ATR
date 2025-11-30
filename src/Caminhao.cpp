#include "Caminhao.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <random>
#include <deque>
#include <cstdio> 
#include <mutex>
#include <algorithm> 

using namespace std::chrono_literals;

namespace {
    constexpr double PI = 3.14159265358979323846;
}

// -----------------------------------------------------------------------------
// Construtor / Destrutor / Controle de Threads (INALTERADO)
// -----------------------------------------------------------------------------

Caminhao::Caminhao(int id, std::size_t capacidadeBuffer)
    : id_(id),
      buffer_(capacidadeBuffer),
      filaEventos_(),
      comandos_{},
      estadoLogico_(EstadoCaminhao::Parado),
      tempoNoEstado_s_(0.0),
      estados_{false, true}, // e_defeito = false, e_automatico = true
      fis_pos_x_(0.0),
      fis_pos_y_(0.0),
      fis_vel_(0.0),
      fis_ang_deg_(0.0),
      fis_temp_C_(40.0),
      fis_forcarFalhaTemp_(false),
      fis_forcarFalhaElec_(false),
      fis_forcarFalhaHid_(false),
      em_reducao_seguranca_(false), // Inicia sem reducao
      atuadores_{0, 0},
      setpoints_{0, 0, 0},
      rota_definida_(false),
      rota_origem_x_(0),
      rota_origem_y_(0),
      rota_destino_x_(0),
      rota_destino_y_(0),
      rodando_(false)
{
    std::string nomeArquivo = "caminhao_" + std::to_string(id_) + ".csv";
    arquivoLog_.open(nomeArquivo, std::ios::out);
    
    // Cabecalho do CSV
    if (arquivoLog_.is_open()) {
        arquivoLog_ << "tempo_s;id_caminhao;estado;e_defeito;e_automatico;"
                    << "i_posicao_x;i_posicao_y;i_angulo_x;i_temperatura;"
                    << "o_aceleracao;o_direcao;evento\n";
        arquivoLog_.flush();
        std::cout << "[Caminhao " << id_ << "] Log em arquivo: " << nomeArquivo << "\n";
    }
}

Caminhao::~Caminhao() {
    parar();
}

void Caminhao::iniciar() {
    if (rodando_) return;

    // --- 1. INICIO MQTT ---
    std::string clientId = "caminhao_" + std::to_string(id_);
    mqtt_ = std::make_unique<MqttInterface>(clientId, 
        [this](const std::string& topico, const std::string& payload) {
            this->processarMensagemMqtt(topico, payload);
        }
    );

    mqtt_->conectar();
    mqtt_->assinar("mina/caminhao/" + std::to_string(id_) + "/cmd");

    rodando_ = true;

    // --- 2. Inicia Threads ---
    thTratamentoSensores_  = std::thread(&Caminhao::tarefaTratamentoSensores,  this);
    thLogicaComando_       = std::thread(&Caminhao::tarefaLogicaComando,       this);
    thMonitoramentoFalhas_ = std::thread(&Caminhao::tarefaMonitoramentoFalhas, this);
    thControleNavegacao_   = std::thread(&Caminhao::tarefaControleNavegacao,   this);
    thPlanejamentoRota_    = std::thread(&Caminhao::tarefaPlanejamentoRota,    this);
    thColetorDados_        = std::thread(&Caminhao::tarefaColetorDados,        this);

    std::cout << "[Caminhao " << id_ << "] Tarefas iniciadas e MQTT conectado.\n";
}

void Caminhao::parar() {
    rodando_ = false;

    if (thTratamentoSensores_.joinable())  thTratamentoSensores_.join();
    if (thLogicaComando_.joinable())       thLogicaComando_.join();
    if (thMonitoramentoFalhas_.joinable()) thMonitoramentoFalhas_.join();
    if (thControleNavegacao_.joinable())   thControleNavegacao_.join();
    if (thPlanejamentoRota_.joinable())    thPlanejamentoRota_.join();
    if (thColetorDados_.joinable())        thColetorDados_.join();
    
    if (mqtt_) mqtt_->desconectar();
}

int Caminhao::getId() const { return id_; }

EstadoCaminhao Caminhao::lerEstadoLogico() const {
    std::lock_guard<std::mutex> lock(mtxEstadoLogico_);
    return estadoLogico_;
}

bool Caminhao::lerUltimoRegistro(RegistroBuffer& out) const {
    return buffer_.tentarLerMaisRecente(out);
}

// -----------------------------------------------------------------------------
// Comandos e Seguranca
// -----------------------------------------------------------------------------

void Caminhao::setReducaoSeguranca(bool ativar) {
    em_reducao_seguranca_ = ativar;
    
    // AJUSTE PARA SISTEMA DE COLISAO:
    // Se o sinal de reducao de seguranca for ativado (risco de colisao), 
    // forcamos o caminhao para o modo de Parada de Emergencia/Manual.
    if (ativar) {
        comandarParadaEmergencia();
    }
}

void Caminhao::comandarAutomatico() {
    std::lock_guard<std::mutex> lock(mtxComandos_);
    comandos_.c_automatico = true;
    comandos_.c_man        = false;
    std::cout << "[Caminhao " << id_ << "] Comando do operador: modo AUTOMATICO.\n";
}

void Caminhao::comandarManual() {
    std::lock_guard<std::mutex> lock(mtxComandos_);
    comandos_.c_man        = true;
    comandos_.c_automatico = false;
    std::cout << "[Caminhao " << id_ << "] Comando do operador: modo MANUAL.\n";
}

void Caminhao::comandarRearme() {
    { std::lock_guard<std::mutex> l(mtxComandos_); comandos_.c_rearme = true; }
    { std::lock_guard<std::mutex> l(mtxEstados_); estados_.e_defeito = false; }
    fis_forcarFalhaTemp_ = false;
    fis_forcarFalhaElec_ = false;
    fis_forcarFalhaHid_  = false;
    std::cout << "[Caminhao " << id_ << "] Comando do operador: REARME.\n";
}

void Caminhao::comandarParadaEmergencia() {
    std::lock_guard<std::mutex> lock(mtxComandos_);
    
    // Passa para Manual
    comandos_.c_man = true;
    comandos_.c_automatico = false;
    
    // Zera qualquer intencao de movimento anterior
    comandos_.c_acelera = false;
    comandos_.c_direita = false;
    comandos_.c_esquerda = false;

    // Mensagem de ALERTA clara na tela
    std::cerr << "[Caminhao " << id_ << "] !!! PARADA DE EMERGENCIA (ANTI-COLISAO) !!!\n";
}

void Caminhao::setComandoAcelerar(bool ativo) { std::lock_guard<std::mutex> l(mtxComandos_); comandos_.c_acelera = ativo; }
void Caminhao::setComandoDireita(bool ativo) { std::lock_guard<std::mutex> l(mtxComandos_); comandos_.c_direita = ativo; }
void Caminhao::setComandoEsquerda(bool ativo) { std::lock_guard<std::mutex> l(mtxComandos_); comandos_.c_esquerda = ativo; }

void Caminhao::injetarFalhaTemperaturaAlta() { fis_forcarFalhaTemp_ = true; std::cout << "[Caminhao " << id_ << "] [TESTE] Falha Temperatura.\n"; }
void Caminhao::injetarFalhaEletrica() { fis_forcarFalhaElec_ = true; std::cout << "[Caminhao " << id_ << "] [TESTE] Falha Eletrica.\n"; }
void Caminhao::injetarFalhaHidraulica() { fis_forcarFalhaHid_ = true; std::cout << "[Caminhao " << id_ << "] [TESTE] Falha Hidraulica.\n"; }

void Caminhao::definirRota(int x1, int y1, int x2, int y2) {
    {
        std::lock_guard<std::mutex> lf(mtxFisico_);
        // Teleporta para inicio (para facilitar testes e evitar colisao no spawn)
        fis_pos_x_ = (double)x1; fis_pos_y_ = (double)y1; fis_vel_ = 0;
        
        // Calcula angulo inicial apontando para o destino
        double dx = (double)(x2 - x1);
        double dy = (double)(y2 - y1);
        if (dx != 0 || dy != 0) {
            fis_ang_deg_ = atan2(dy, dx) * 180.0/PI;
        }
    }
    {
        std::lock_guard<std::mutex> lr(mtxRota_);
        rota_origem_x_ = x1; rota_origem_y_ = y1;
        rota_destino_x_ = x2; rota_destino_y_ = y2;
        rota_definida_ = true;
    }
    
    // Reseta estado para Parado ao definir nova rota
    {
        std::lock_guard<std::mutex> le(mtxEstadoLogico_);
        estadoLogico_ = EstadoCaminhao::Parado;
    }
    
    std::cout << "[Caminhao " << id_ << "] Rota definida (" << x1 << "," << y1 << ") -> (" << x2 << "," << y2 << ")\n";
}

// -----------------------------------------------------------------------------
// Tarefas
// -----------------------------------------------------------------------------

void Caminhao::tarefaTratamentoSensores() {
    auto inicio = std::chrono::steady_clock::now();
    const int M = 10;
    std::deque<double> hist_x, hist_y, hist_ang, hist_temp;
    std::mt19937 rng(id_ + std::chrono::system_clock::now().time_since_epoch().count());
    std::normal_distribution<double> noise(0.0, 0.2);

    auto filtra = [&](std::deque<double>& h, double v) {
        h.push_back(v); if(h.size()>M) h.pop_front();
        double s=0; for(auto x:h) s+=x; return s/h.size();
    };

    while (rodando_) {
        double px, py, ang, temp;
        { std::lock_guard<std::mutex> l(mtxFisico_); px=fis_pos_x_; py=fis_pos_y_; ang=fis_ang_deg_; temp=fis_temp_C_; }

        // Aplica ruido e filtra
        px = filtra(hist_x, px + noise(rng));
        py = filtra(hist_y, py + noise(rng));
        ang = filtra(hist_ang, ang);
        temp = filtra(hist_temp, temp);

        SensoresCaminhao s;
        s.i_posicao_x = (int)std::lround(px); s.i_posicao_y = (int)std::lround(py);
        s.i_angulo_x = (int)std::lround(ang); s.i_temperatura = (int)std::lround(temp);
        
        if(fis_forcarFalhaTemp_) s.i_temperatura = 130;
        s.i_falha_eletrica = fis_forcarFalhaElec_;
        s.i_falha_hidraulica = fis_forcarFalhaHid_;

        RegistroBuffer reg;
        reg.tempoSimulacao_s = std::chrono::duration<double>(std::chrono::steady_clock::now()-inicio).count();
        reg.id_caminhao = id_; reg.sensores = s;
        { std::lock_guard<std::mutex> l(mtxEstados_); reg.estados = estados_; }
        { std::lock_guard<std::mutex> l(mtxComandos_); reg.comandos = comandos_; }
        { std::lock_guard<std::mutex> l(mtxAtuadores_); reg.atuadores = atuadores_; }
        { std::lock_guard<std::mutex> l(mtxSetpoints_); reg.setpoints = setpoints_; }
        { std::lock_guard<std::mutex> l(mtxEstadoLogico_); reg.estado = estadoLogico_; }

        buffer_.inserir(reg);
        std::this_thread::sleep_for(100ms);
    }
}

// Logica de Comando
void Caminhao::tarefaLogicaComando() {
    double ultimoTempoBuffer = 0.0;
    
    // Variavel estatica ou local para persistir a necessidade de rearme na transicao
    bool bloqueio_seguranca_transicao = false; 

    while (rodando_) {
        RegistroBuffer reg{};
        if (!buffer_.tentarLerMaisRecente(reg)) {
            std::this_thread::sleep_for(50ms);
            continue;
        }
        
        ultimoTempoBuffer = reg.tempoSimulacao_s;

        // 1. Atualizar Estados (Comandos de Borda)
        {
            std::lock_guard<std::mutex> l(mtxEstados_);
            
            // Se entrou em modo manual, ativamos o bloqueio de seguranca
            if (reg.comandos.c_man) {
                estados_.e_automatico = false;
                bloqueio_seguranca_transicao = true; // Trava a transicao de volta para auto
            }
            
            // Se apertou rearme, liberamos o bloqueio (e limpa falhas)
            if (reg.comandos.c_rearme) {
                estados_.e_defeito = false;
                bloqueio_seguranca_transicao = false; // Destrava

                // CONSUMIR COMANDO REARME APOS USO para que ele nao fique ativo no proximo ciclo
                {
                    std::lock_guard<std::mutex> l_cmd(mtxComandos_);
                    comandos_.c_rearme = false;
                }
            }

            // Se pediu automatico, so aceita se NAO estiver bloqueado (ou seja, se ja rearmou)
            if (reg.comandos.c_automatico) {
                if (bloqueio_seguranca_transicao) {
                    // Operador pediu Auto, mas ainda nao rearmou apos sair do manual.
                    // Mantem em manual por seguranca.
                    estados_.e_automatico = false;
                } else {
                    // Seguro para entrar em auto
                    estados_.e_automatico = true;
                }
            }
            
            // Falhas (prioridade maxima sobre os estados)
            if (reg.sensores.i_falha_eletrica || reg.sensores.i_falha_hidraulica || reg.sensores.i_temperatura > 120) {
                estados_.e_defeito = true;
            }
        }

        // 2. Determinar o Estado Logico (Parado vs Movimento vs Falha)
        EstadoCaminhao novoEstado = EstadoCaminhao::Parado;
        
        // Pega a velocidade real para saber se esta andando
        double velocidadeAtual = 0.0;
        {
            std::lock_guard<std::mutex> lf(mtxFisico_);
            velocidadeAtual = fis_vel_;
        }

        bool estaAcelerar = (reg.atuadores.o_aceleracao != 0);
        bool estaAAndar = (std::abs(velocidadeAtual) > 0.1);

        if (reg.estados.e_defeito) {
            novoEstado = EstadoCaminhao::EmFalha;
        }
        else if (estaAAndar || estaAcelerar) {
            novoEstado = EstadoCaminhao::EmMovimento;
        }
        else {
            novoEstado = EstadoCaminhao::Parado;
        }

        {
            std::lock_guard<std::mutex> le(mtxEstadoLogico_);
            estadoLogico_ = novoEstado;
        }

        std::this_thread::sleep_for(50ms);
    }
    std::cout << "[Caminhao " << id_ << "] Tarefa LogicaComando encerrada.\n";
}

void Caminhao::tarefaMonitoramentoFalhas() {
    while (rodando_) std::this_thread::sleep_for(200ms);
}

void Caminhao::tarefaControleNavegacao() {
    auto anterior = std::chrono::steady_clock::now();
    const double a_max = 2.0;
    const double fric = 0.2;
    const double Kp_dist = 1.0;
    const double DIST_PARAR = 1.0;

    const int MANUAL_ACEL_VAL = 50;
    const int MANUAL_DIR_PASSO = 10;

    while (rodando_) {
        auto agora = std::chrono::steady_clock::now();
        double dt = std::chrono::duration<double>(agora - anterior).count();
        anterior = agora; if(dt<=0) dt=0.01;

        AtuadoresCaminhao atu;
        { std::lock_guard<std::mutex> l(mtxAtuadores_); atu = atuadores_; }

        {
            std::lock_guard<std::mutex> l(mtxFisico_);
            
            // Atualiza o angulo fisico imediatamente com o valor do atuador
            fis_ang_deg_ = static_cast<double>(atu.o_direcao); 

            double rad = fis_ang_deg_ * PI / 180.0;
            double a = (static_cast<double>(atu.o_aceleracao)/100.0) * a_max;
            
            fis_vel_ += a * dt;
            fis_vel_ -= fric * fis_vel_ * dt; // Atrito
            
            fis_pos_x_ += fis_vel_ * cos(rad) * dt;
            fis_pos_y_ += fis_vel_ * sin(rad) * dt;
            
            // Simulacao de temperatura (aquece com a velocidade)
            double alvoTemp = 40.0 + 2.0 * fabs(fis_vel_);
            fis_temp_C_ += 0.5 * (alvoTemp - fis_temp_C_) * dt;
        }

        // --- BLOCO DE CONTROLE (NAVEGACAO) ---
        RegistroBuffer reg{};
        if(buffer_.tentarLerMaisRecente(reg)) {
            EstadosCaminhao ests = reg.estados;
            SetpointsCaminhao sp = reg.setpoints;
            SensoresCaminhao s = reg.sensores;
            ComandosCaminhao cmds = reg.comandos;

            AtuadoresCaminhao novosAtu = atu;
            bool temDefeito = ests.e_defeito || (reg.estado == EstadoCaminhao::EmFalha);

            // 1. Seguranca (Anticolisao - Reducao de Velocidade)
            if (em_reducao_seguranca_ || temDefeito) { // Parada de seguranca ou falha
                novosAtu.o_aceleracao = 0; 
            }
            // 2. Automatico 
            else if (ests.e_automatico && !temDefeito) {
                double dx = static_cast<double>(sp.sp_posicao_x - s.i_posicao_x);
                double dy = static_cast<double>(sp.sp_posicao_y - s.i_posicao_y);
                double dist = sqrt(dx*dx + dy*dy);
                
                // Controle de Direcao
                double cmd_dir = static_cast<double>(sp.sp_angulo_x);
                while (cmd_dir > 180.0) cmd_dir -= 360.0;
                while (cmd_dir < -180.0) cmd_dir += 360.0;
                novosAtu.o_direcao = static_cast<int>(std::lround(cmd_dir));
                
                // Controle de Aceleracao
                double cmd_acel = Kp_dist * dist;
                if(cmd_acel > 100.0) cmd_acel = 100.0;
                if(cmd_acel < 0.0) cmd_acel = 0.0;
                
                if(dist <= DIST_PARAR) cmd_acel = 0.0;
                
                novosAtu.o_aceleracao = static_cast<int>(std::lround(cmd_acel));
            } 
            // 3. Manual
            else if (!ests.e_automatico && !temDefeito) {
                int dir = novosAtu.o_direcao;
                if (cmds.c_direita) dir -= MANUAL_DIR_PASSO;
                if (cmds.c_esquerda) dir += MANUAL_DIR_PASSO;
                if (dir > 180) dir = 180;
                if (dir < -180) dir = -180;
                novosAtu.o_direcao = dir;

                novosAtu.o_aceleracao = cmds.c_acelera ? MANUAL_ACEL_VAL : 0;
            } 
            // 4. Defeito (Parada Total - Travagem de Emergencia)
            else {
                novosAtu.o_aceleracao = 0;
                
                // Forca a parada imediata em caso de falha ou bloqueio de seguranca
                {
                    std::lock_guard<std::mutex> lf(mtxFisico_);
                    fis_vel_ = 0.0; 
                }
            }

            { std::lock_guard<std::mutex> l(mtxAtuadores_); atuadores_ = novosAtu; }
        }
        std::this_thread::sleep_for(50ms);
    }
    std::cout << "[Caminhao " << id_ << "] Tarefa ControleNavegacao encerrada.\n";
}

void Caminhao::tarefaPlanejamentoRota() {
    while (rodando_) {
        RegistroBuffer reg{};
        if (buffer_.tentarLerMaisRecente(reg)) {
            std::lock_guard<std::mutex> l(mtxSetpoints_);
            
            if(rota_definida_) {
                setpoints_.sp_posicao_x = rota_destino_x_;
                setpoints_.sp_posicao_y = rota_destino_y_;
                
                double dx = static_cast<double>(rota_destino_x_ - reg.sensores.i_posicao_x);
                double dy = static_cast<double>(rota_destino_y_ - reg.sensores.i_posicao_y);
                if (std::abs(dx) > 1.0 || std::abs(dy) > 1.0) {
                    double ang_rad = std::atan2(dy, dx);
                    setpoints_.sp_angulo_x = static_cast<int>(std::lround(ang_rad * 180.0/PI));
                }
            } else {
                setpoints_.sp_posicao_x = reg.sensores.i_posicao_x;
                setpoints_.sp_posicao_y = reg.sensores.i_posicao_y;
                setpoints_.sp_angulo_x = reg.sensores.i_angulo_x;
            }
        }
        std::this_thread::sleep_for(100ms);
    }
}

// --- Coletor de Dados ---
void Caminhao::tarefaColetorDados() {
    bool defeitoAnterior = false;
    bool manualAnterior = false;
    bool alertaTempAnterior = false;

    // Codigos ANSI
    const std::string COR_AMARELA = "\033[1;33m";
    const std::string COR_VERMELHA = "\033[1;31m";
    const std::string COR_RESET = "\033[0m";

    while (rodando_) {
        RegistroBuffer reg{};
        if (buffer_.tentarLerMaisRecente(reg)) {
            std::string textoEvento;
            int temp = reg.sensores.i_temperatura;

            // 1. Detecta Eventos de Falha
            if (!defeitoAnterior && reg.estados.e_defeito) {
                if (reg.sensores.i_falha_eletrica) textoEvento = "FALHA ELETRICA";
                else if (reg.sensores.i_falha_hidraulica) textoEvento = "FALHA HIDRAULICA";
                else if (temp > 120) textoEvento = "SOBREAQUECIMENTO (>120C)";
                else textoEvento = "FALHA CRITICA GENERICA";
            }
            else if (defeitoAnterior && !reg.estados.e_defeito) {
                textoEvento = "REARME";
            }
            
            // 2. Alerta de Temperatura (Apenas Aviso)
            else if (temp > 95 && temp <= 120) {
                if (!alertaTempAnterior) {
                    textoEvento = "ALERTA TEMP (>95C)";
                    alertaTempAnterior = true;
                }
            }
            else {
                alertaTempAnterior = false; 
            }

            // 3. Modos de Operacao
            if (textoEvento.empty()) {
                if (!manualAnterior && reg.comandos.c_man) {
                    textoEvento = "MODO MANUAL / EMERGENCIA";
                }
                else if (manualAnterior && reg.comandos.c_automatico) {
                    textoEvento = "MODO AUTO";
                }
            }

            defeitoAnterior = reg.estados.e_defeito;
            manualAnterior = reg.comandos.c_man;

            // Log Terminal: Imprime SEMPRE a posicao e estado
            std::cout << (reg.estados.e_defeito ? COR_VERMELHA : "") 
                      << "[Caminhao " << id_ << "] "
                      << "x=" << reg.sensores.i_posicao_x << " "
                      << "y=" << reg.sensores.i_posicao_y << " "
                      << "def=" << reg.estados.e_defeito
                      << (textoEvento.empty() ? "" : (" | " + textoEvento)) 
                      << (reg.estados.e_defeito ? COR_RESET : "") << "\n";

            // Log CSV
            {
                std::lock_guard<std::mutex> lock(mtxLog_);
                if (arquivoLog_.is_open()) {
                    arquivoLog_ << reg.tempoSimulacao_s << ";" << id_ << ";"
                                << estadoToString(reg.estado) << ";"
                                << reg.estados.e_defeito << ";" << reg.estados.e_automatico << ";"
                                << reg.sensores.i_posicao_x << ";" << reg.sensores.i_posicao_y << ";"
                                << reg.sensores.i_angulo_x << ";" << reg.sensores.i_temperatura << ";"
                                << reg.atuadores.o_aceleracao << ";" << reg.atuadores.o_direcao << ";"
                                << textoEvento << "\n";
                    arquivoLog_.flush(); 
                }
            }
            
            // MQTT
            if (mqtt_) {
                std::string json = "{ \"id\": " + std::to_string(id_) + 
                ", \"x\": " + std::to_string(reg.sensores.i_posicao_x) +
                ", \"y\": " + std::to_string(reg.sensores.i_posicao_y) +
                ", \"temp\": " + std::to_string(reg.sensores.i_temperatura) + 
                ", \"defeito\": " + (reg.estados.e_defeito?"true":"false") +
                ", \"auto\": " + (reg.estados.e_automatico?"true":"false") + " }";
                mqtt_->publicar("mina/caminhao/"+std::to_string(id_)+"/estado", json);
            }
        }
        std::this_thread::sleep_for(500ms);
    }
    std::cout << "[Caminhao " << id_ << "] Tarefa ColetorDados encerrada.\n";
}

void Caminhao::processarMensagemMqtt(const std::string& topico, const std::string& payload) {
    (void)topico; 
    std::cout << "[MQTT Recv " << id_ << "] " << payload << std::endl;

    if (payload.rfind("ROTA:", 0) == 0) {
        int x1, y1, x2, y2;
        if (sscanf(payload.c_str() + 5, "%d,%d,%d,%d", &x1, &y1, &x2, &y2) == 4) {
            definirRota(x1, y1, x2, y2);
        }
    }
    else if (payload == "CMD:AUTO") comandarAutomatico();
    else if (payload == "CMD:MANUAL") comandarManual();
    else if (payload == "CMD:REARME") comandarRearme();
}