// src/Caminhao.cpp
#include "Caminhao.hpp"

#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <random>
#include <deque>

using namespace std::chrono_literals;

namespace {
    constexpr double PI = 3.14159265358979323846;
}

Caminhao::Caminhao(int id, std::size_t capacidadeBuffer)
    : id_(id),
      buffer_(capacidadeBuffer),
      filaEventos_(),
      comandos_{},
      estadoLogico_(EstadoCaminhao::Parado),
      tempoNoEstado_s_(0.0),
      estados_{false, true},
      fis_pos_x_(0.0),
      fis_pos_y_(0.0),
      fis_vel_(0.0),
      fis_ang_deg_(0.0),
      fis_temp_C_(40.0),
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
    if (arquivoLog_.is_open()) {
        arquivoLog_ << "tempo_s;id_caminhao;estado;e_defeito;e_automatico;"
                    << "i_posicao_x;i_posicao_y;i_angulo_x;i_temperatura;"
                    << "o_aceleracao;o_direcao\n";
        arquivoLog_.flush();
        std::cout << "[Caminhao " << id_ << "] Log em arquivo: " << nomeArquivo << "\n";
    } else {
        std::cerr << "[Caminhao " << id_ << "] ERRO ao abrir arquivo de log: "
                  << nomeArquivo << "\n";
    }
}

Caminhao::~Caminhao() {
    parar();
}

void Caminhao::iniciar() {
    if (rodando_) return;
    rodando_ = true;

    thTratamentoSensores_  = std::thread(&Caminhao::tarefaTratamentoSensores,  this);
    thLogicaComando_       = std::thread(&Caminhao::tarefaLogicaComando,       this);
    thMonitoramentoFalhas_ = std::thread(&Caminhao::tarefaMonitoramentoFalhas, this);
    thControleNavegacao_   = std::thread(&Caminhao::tarefaControleNavegacao,   this);
    thPlanejamentoRota_    = std::thread(&Caminhao::tarefaPlanejamentoRota,    this);
    thColetorDados_        = std::thread(&Caminhao::tarefaColetorDados,        this);

    std::cout << "[Caminhao " << id_ << "] Tarefas iniciadas.\n";
}

void Caminhao::parar() {
    rodando_ = false;

    if (thTratamentoSensores_.joinable())  thTratamentoSensores_.join();
    if (thLogicaComando_.joinable())       thLogicaComando_.join();
    if (thMonitoramentoFalhas_.joinable()) thMonitoramentoFalhas_.join();
    if (thControleNavegacao_.joinable())   thControleNavegacao_.join();
    if (thPlanejamentoRota_.joinable())    thPlanejamentoRota_.join();
    if (thColetorDados_.joinable())        thColetorDados_.join();
}

int Caminhao::getId() const {
    return id_;
}

EstadoCaminhao Caminhao::lerEstadoLogico() const {
    std::lock_guard<std::mutex> lock(mtxEstadoLogico_);
    return estadoLogico_;
}

bool Caminhao::lerUltimoRegistro(RegistroBuffer& out) const {
    return buffer_.tentarLerMaisRecente(out);
}

void Caminhao::comandarAutomatico() {
    std::lock_guard<std::mutex> lock(mtxComandos_);
    comandos_.c_automatico = true;
    comandos_.c_man        = false;
    std::cout << "[Caminhao " << id_ << "] Comando do operador: modo AUTOMÁTICO.\n";
}

void Caminhao::comandarManual() {
    std::lock_guard<std::mutex> lock(mtxComandos_);
    comandos_.c_man        = true;
    comandos_.c_automatico = false;
    std::cout << "[Caminhao " << id_ << "] Comando do operador: modo MANUAL.\n";
}

void Caminhao::comandarRearme() {
    {
        std::lock_guard<std::mutex> lock(mtxComandos_);
        comandos_.c_rearme = true;
    }

    Evento ev{};
    ev.tipo = TipoEvento::ComandoRearme;
    ev.descricao = "Rearme solicitado pelo operador.";
    ev.tempoSimulacao_s = 0.0;
    ev.id_caminhao = id_;
    filaEventos_.postar(ev);

    std::cout << "[Caminhao " << id_ << "] Comando do operador: REARME.\n";
}

void Caminhao::definirRota(int x_inicial, int y_inicial, int x_destino, int y_destino) {
    {
        std::lock_guard<std::mutex> lockFis(mtxFisico_);
        fis_pos_x_ = static_cast<double>(x_inicial);
        fis_pos_y_ = static_cast<double>(y_inicial);
        fis_vel_   = 0.0;

        double dx = static_cast<double>(x_destino - x_inicial);
        double dy = static_cast<double>(y_destino - y_inicial);
        double ang_rad = std::atan2(dy, dx);
        fis_ang_deg_ = ang_rad * 180.0 / PI;
        fis_temp_C_  = 40.0;
    }

    {
        std::lock_guard<std::mutex> lockRota(mtxRota_);
        rota_origem_x_  = x_inicial;
        rota_origem_y_  = y_inicial;
        rota_destino_x_ = x_destino;
        rota_destino_y_ = y_destino;
        rota_definida_  = true;
    }

    {
        std::lock_guard<std::mutex> lockEst(mtxEstadoLogico_);
        estadoLogico_    = EstadoCaminhao::Parado;
        tempoNoEstado_s_ = 0.0;
    }

    std::cout << "[Caminhao " << id_ << "] Rota definida: origem=("
              << x_inicial << "," << y_inicial << ") destino=("
              << x_destino << "," << y_destino << ")\n";
}

void Caminhao::tarefaTratamentoSensores() {
    using clock = std::chrono::steady_clock;
    auto inicio = clock::now();

    const int M = 10;
    std::deque<double> hist_x, hist_y, hist_ang, hist_temp;

    std::mt19937 rng(static_cast<unsigned>(
        clock::now().time_since_epoch().count() + id_));
    std::normal_distribution<double> noisePos(0.0, 0.2);
    std::normal_distribution<double> noiseAng(0.0, 1.0);
    std::normal_distribution<double> noiseTemp(0.0, 0.5);

    auto filtraMM = [&](std::deque<double>& hist, double novo) {
        hist.push_back(novo);
        if (static_cast<int>(hist.size()) > M) {
            hist.pop_front();
        }
        double soma = 0.0;
        for (double v : hist) soma += v;
        return soma / static_cast<double>(hist.size());
    };

    while (rodando_) {
        auto agora = clock::now();
        std::chrono::duration<double> diff = agora - inicio;
        double tempoSim = diff.count();

        double posx, posy, ang, temp;
        {
            std::lock_guard<std::mutex> lock(mtxFisico_);
            posx = fis_pos_x_;
            posy = fis_pos_y_;
            ang  = fis_ang_deg_;
            temp = fis_temp_C_;
        }

        double posx_ruido = posx + noisePos(rng);
        double posy_ruido = posy + noisePos(rng);
        double ang_ruido  = ang  + noiseAng(rng);
        double temp_ruido = temp + noiseTemp(rng);

        double posx_f = filtraMM(hist_x, posx_ruido);
        double posy_f = filtraMM(hist_y, posy_ruido);
        double ang_f  = filtraMM(hist_ang, ang_ruido);
        double temp_f = filtraMM(hist_temp, temp_ruido);

        SensoresCaminhao sensores{};
        sensores.i_posicao_x   = static_cast<int>(std::lround(posx_f));
        sensores.i_posicao_y   = static_cast<int>(std::lround(posy_f));
        sensores.i_angulo_x    = static_cast<int>(std::lround(ang_f));
        sensores.i_temperatura = static_cast<int>(std::lround(temp_f));
        sensores.i_falha_eletrica   = false;
        sensores.i_falha_hidraulica = false;

        EstadosCaminhao estados;
        {
            std::lock_guard<std::mutex> lock(mtxEstados_);
            estados = estados_;
        }

        ComandosCaminhao comandos;
        {
            std::lock_guard<std::mutex> lock(mtxComandos_);
            comandos = comandos_;
        }

        AtuadoresCaminhao atuadores;
        {
            std::lock_guard<std::mutex> lock(mtxAtuadores_);
            atuadores = atuadores_;
        }

        SetpointsCaminhao setpoints;
        {
            std::lock_guard<std::mutex> lock(mtxSetpoints_);
            setpoints = setpoints_;
        }

        EstadoCaminhao estadoLogico;
        {
            std::lock_guard<std::mutex> lock(mtxEstadoLogico_);
            estadoLogico = estadoLogico_;
        }

        RegistroBuffer reg{};
        reg.tempoSimulacao_s = tempoSim;
        reg.id_caminhao      = id_;
        reg.estado           = estadoLogico;
        reg.sensores         = sensores;
        reg.atuadores        = atuadores;
        reg.estados          = estados;
        reg.comandos         = comandos;
        reg.setpoints        = setpoints;

        buffer_.inserir(reg);

        std::this_thread::sleep_for(100ms);
    }

    std::cout << "[Caminhao " << id_ << "] Tarefa TratamentoSensores encerrada.\n";
}

void Caminhao::tarefaLogicaComando() {
    double ultimoTempoBuffer = 0.0;
    const double DIST_LIMITE = 1.0;

    while (rodando_) {
        RegistroBuffer reg{};
        if (!buffer_.tentarLerMaisRecente(reg)) {
            std::this_thread::sleep_for(50ms);
            continue;
        }

        double dt = reg.tempoSimulacao_s - ultimoTempoBuffer;
        if (dt < 0.0) dt = 0.0;
        ultimoTempoBuffer = reg.tempoSimulacao_s;

        ComandosCaminhao cmds = reg.comandos;
        EstadosCaminhao  ests = reg.estados;

        EstadosCaminhao novosEstados = ests;

        if (cmds.c_automatico) {
            novosEstados.e_automatico = true;
        }
        if (cmds.c_man) {
            novosEstados.e_automatico = false;
        }
        if (cmds.c_rearme) {
            novosEstados.e_defeito = false;
        }

        {
            std::lock_guard<std::mutex> lock(mtxEstados_);
            estados_ = novosEstados;
        }

        bool rotaDefinida;
        int dest_x = 0, dest_y = 0;
        {
            std::lock_guard<std::mutex> lock(mtxRota_);
            rotaDefinida = rota_definida_;
            dest_x       = rota_destino_x_;
            dest_y       = rota_destino_y_;
        }

        EstadoCaminhao estadoAtual;
        double tempoNoEstado;
        {
            std::lock_guard<std::mutex> lock(mtxEstadoLogico_);
            estadoAtual    = estadoLogico_;
            tempoNoEstado  = tempoNoEstado_s_;
        }

        EstadoCaminhao novoEstado = estadoAtual;

        if (novosEstados.e_defeito) {
            novoEstado = EstadoCaminhao::EmFalha;
        } else if (novosEstados.e_automatico && rotaDefinida) {
            double dx = static_cast<double>(dest_x - reg.sensores.i_posicao_x);
            double dy = static_cast<double>(dest_y - reg.sensores.i_posicao_y);
            double dist = std::sqrt(dx*dx + dy*dy);

            switch (estadoAtual) {
                case EstadoCaminhao::Parado:
                    if (dist > DIST_LIMITE) {
                        novoEstado = EstadoCaminhao::EmMovimento;
                    }
                    break;
                case EstadoCaminhao::EmMovimento:
                    if (dist <= DIST_LIMITE) {
                        novoEstado = EstadoCaminhao::Parado;
                    }
                    break;
                case EstadoCaminhao::EmFalha:
                    break;
            }
        } else {
            if (!novosEstados.e_automatico && estadoAtual == EstadoCaminhao::EmMovimento) {
                novoEstado = EstadoCaminhao::Parado;
            }
        }

        {
            std::lock_guard<std::mutex> lock(mtxEstadoLogico_);
            if (novoEstado != estadoLogico_) {
                estadoLogico_    = novoEstado;
                tempoNoEstado_s_ = 0.0;
            } else {
                tempoNoEstado_s_ += dt;
            }
        }

        if (cmds.c_automatico || cmds.c_man || cmds.c_rearme) {
            std::lock_guard<std::mutex> lock(mtxComandos_);
            comandos_.c_automatico = false;
            comandos_.c_man        = false;
            comandos_.c_rearme     = false;
        }

        std::this_thread::sleep_for(50ms);
    }

    std::cout << "[Caminhao " << id_ << "] Tarefa LogicaComando encerrada.\n";
}

void Caminhao::tarefaMonitoramentoFalhas() {
    while (rodando_) {
        std::this_thread::sleep_for(200ms);
    }

    std::cout << "[Caminhao " << id_ << "] Tarefa MonitoramentoFalhas encerrada.\n";
}

void Caminhao::tarefaControleNavegacao() {
    auto anterior = std::chrono::steady_clock::now();

    const double a_max    = 2.0;
    const double fric     = 0.2;
    const double Kp_dist  = 1.0;
    const double Kp_ang   = 1.0;
    const double DIST_PARAR = 1.0;

    while (rodando_) {
        auto agora = std::chrono::steady_clock::now();
        std::chrono::duration<double> diff = agora - anterior;
        anterior = agora;
        double dt = diff.count();
        if (dt <= 0.0) dt = 0.01;

        AtuadoresCaminhao atu;
        {
            std::lock_guard<std::mutex> lock(mtxAtuadores_);
            atu = atuadores_;
        }

        {
            std::lock_guard<std::mutex> lock(mtxFisico_);
            fis_ang_deg_ = static_cast<double>(atu.o_direcao);
            double ang_rad = fis_ang_deg_ * PI / 180.0;

            double a = (static_cast<double>(atu.o_aceleracao) / 100.0) * a_max;

            fis_vel_ += a * dt;
            fis_vel_ -= fric * fis_vel_ * dt;

            fis_pos_x_ += fis_vel_ * std::cos(ang_rad) * dt;
            fis_pos_y_ += fis_vel_ * std::sin(ang_rad) * dt;

            if (fis_pos_x_ < -1000.0) fis_pos_x_ = -1000.0;
            if (fis_pos_x_ >  1000.0) fis_pos_x_ =  1000.0;
            if (fis_pos_y_ < -1000.0) fis_pos_y_ = -1000.0;
            if (fis_pos_y_ >  1000.0) fis_pos_y_ =  1000.0;

            double base     = 40.0;
            double ktemp    = 2.0;
            double alvoTemp = base + ktemp * std::fabs(fis_vel_);
            fis_temp_C_    += 0.5 * (alvoTemp - fis_temp_C_) * dt;
        }

        RegistroBuffer reg{};
        if (buffer_.tentarLerMaisRecente(reg)) {
            EstadosCaminhao   ests = reg.estados;
            SetpointsCaminhao sp   = reg.setpoints;
            SensoresCaminhao  s    = reg.sensores;

            AtuadoresCaminhao novosAtu = atu;

            if (ests.e_automatico && !ests.e_defeito &&
                reg.estado != EstadoCaminhao::EmFalha) {

                double dx   = static_cast<double>(sp.sp_posicao_x - s.i_posicao_x);
                double dy   = static_cast<double>(sp.sp_posicao_y - s.i_posicao_y);
                double dist = std::sqrt(dx*dx + dy*dy);

                double erro_ang = static_cast<double>(sp.sp_angulo_x - s.i_angulo_x);
                while (erro_ang > 180.0)  erro_ang -= 360.0;
                while (erro_ang < -180.0) erro_ang += 360.0;

                double cmd_dir = s.i_angulo_x + Kp_ang * erro_ang;
                if (cmd_dir > 180.0)  cmd_dir = 180.0;
                if (cmd_dir < -180.0) cmd_dir = -180.0;
                novosAtu.o_direcao = static_cast<int>(std::lround(cmd_dir));

                double cmd_acel = Kp_dist * dist;
                if (cmd_acel > 100.0) cmd_acel = 100.0;
                if (cmd_acel < 0.0)   cmd_acel = 0.0;

                if (dist <= DIST_PARAR) {
                    cmd_acel = 0.0;
                }

                novosAtu.o_aceleracao = static_cast<int>(std::lround(cmd_acel));
            } else {
                novosAtu.o_aceleracao = 0;
                novosAtu.o_direcao    = s.i_angulo_x;
            }

            {
                std::lock_guard<std::mutex> lock(mtxAtuadores_);
                atuadores_ = novosAtu;
            }
        }

        std::this_thread::sleep_for(50ms);
    }

    std::cout << "[Caminhao " << id_ << "] Tarefa ControleNavegacao encerrada.\n";
}

void Caminhao::tarefaPlanejamentoRota() {
    while (rodando_) {
        RegistroBuffer reg{};
        if (!buffer_.tentarLerMaisRecente(reg)) {
            std::this_thread::sleep_for(100ms);
            continue;
        }

        bool rotaDefinida;
        int dest_x = 0, dest_y = 0;
        {
            std::lock_guard<std::mutex> lock(mtxRota_);
            rotaDefinida = rota_definida_;
            dest_x       = rota_destino_x_;
            dest_y       = rota_destino_y_;
        }

        SetpointsCaminhao sp = reg.setpoints;

        if (rotaDefinida &&
            reg.estados.e_automatico &&
            !reg.estados.e_defeito &&
            reg.estado != EstadoCaminhao::EmFalha) {

            sp.sp_posicao_x = dest_x;
            sp.sp_posicao_y = dest_y;

            double dx = static_cast<double>(dest_x - reg.sensores.i_posicao_x);
            double dy = static_cast<double>(dest_y - reg.sensores.i_posicao_y);
            double ang_rad = std::atan2(dy, dx);
            double ang_deg = ang_rad * 180.0 / PI;
            sp.sp_angulo_x = static_cast<int>(std::lround(ang_deg));
        } else {
            sp.sp_posicao_x = reg.sensores.i_posicao_x;
            sp.sp_posicao_y = reg.sensores.i_posicao_y;
            sp.sp_angulo_x  = reg.sensores.i_angulo_x;
        }

        {
            std::lock_guard<std::mutex> lock(mtxSetpoints_);
            setpoints_ = sp;
        }

        std::this_thread::sleep_for(100ms);
    }

    std::cout << "[Caminhao " << id_ << "] Tarefa PlanejamentoRota encerrada.\n";
}

void Caminhao::tarefaColetorDados() {
    while (rodando_) {
        RegistroBuffer reg{};
        if (buffer_.tentarLerMaisRecente(reg)) {
            std::cout << "[Caminhao " << id_ << "] t=" << reg.tempoSimulacao_s
                      << "s | Estado=" << estadoToString(reg.estado)
                      << " | x=" << reg.sensores.i_posicao_x
                      << " | y=" << reg.sensores.i_posicao_y
                      << " | ang=" << reg.sensores.i_angulo_x << " deg"
                      << " | Tmot=" << reg.sensores.i_temperatura << " C"
                      << " | e_defeito=" << (reg.estados.e_defeito ? 1 : 0)
                      << " | e_auto="   << (reg.estados.e_automatico ? 1 : 0)
                      << "\n";

            {
                std::lock_guard<std::mutex> lock(mtxLog_);
                if (arquivoLog_.is_open()) {
                    arquivoLog_ << reg.tempoSimulacao_s << ";"
                                << id_ << ";"
                                << estadoToString(reg.estado) << ";"
                                << (reg.estados.e_defeito ? 1 : 0) << ";"
                                << (reg.estados.e_automatico ? 1 : 0) << ";"
                                << reg.sensores.i_posicao_x << ";"
                                << reg.sensores.i_posicao_y << ";"
                                << reg.sensores.i_angulo_x << ";"
                                << reg.sensores.i_temperatura << ";"
                                << reg.atuadores.o_aceleracao << ";"
                                << reg.atuadores.o_direcao
                                << "\n";
                    arquivoLog_.flush();
                }
            }
        }

        std::this_thread::sleep_for(1s);
    }

    std::cout << "[Caminhao " << id_ << "] Tarefa ColetorDados encerrada.\n";
}
