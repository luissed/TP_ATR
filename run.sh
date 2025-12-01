#!/bin/bash

set -e


echo "--- [1/3] Iniciando Broker MQTT (Mosquitto) em background..."
mosquitto -d -p 1883
sleep 1


LOG_DIR="/app/logs_output"
mkdir -p $LOG_DIR
echo "--- [2/3] Logs serão escritos em: $LOG_DIR"


cd $LOG_DIR

echo "--- [3/3] Iniciando Backend e GUI (gui_gestao)..."

/usr/local/app/simulacao_backend &
BACKEND_PID=$!
sleep 1 

/usr/local/app/gui_gestao

echo "--- 🧹 Finalizando processos em background..."

# Encerra o processo do backend e do broker
if kill $BACKEND_PID 2>/dev/null; then
    echo "Backend de simulação (PID: $BACKEND_PID) encerrado."
fi

kill $(pgrep mosquitto) 2>/dev/null
echo "Broker Mosquitto encerrado. Sistema finalizado."