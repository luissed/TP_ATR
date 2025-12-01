FROM debian:bookworm

# Instala ferramentas de build e dependências
RUN apt update && apt install -y \
    build-essential \
    cmake \
    git \
    make \
    dos2unix \ 
    libsfml-dev \
    mosquitto \
    mosquitto-clients \
    libpaho-mqtt-dev \
    libpaho-mqttpp-dev \
    fonts-dejavu-core 

# Define o diretório de trabalho seguro, longe da montagem de volume
WORKDIR /usr/local/app

# Copia todo o conteúdo do diretório atual do host 
COPY . /usr/local/app

# Lista o conteúdo para verificar se o run.sh foi copiado 
RUN ls -l /usr/local/app/

# 1. Corrige o formato de quebra de linha do script run.sh
RUN dos2unix /usr/local/app/run.sh

# 2. Garante permissão de execução
RUN chmod +x /usr/local/app/run.sh

# 3. Compila o projeto
RUN make

CMD ["/usr/local/app/run.sh"]