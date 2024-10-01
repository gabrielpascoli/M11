FROM python:3.11-slim

# Instalar dependências do sistema, incluindo bibliotecas necessárias para o OpenCV
RUN apt-get update && \
    apt-get install -y \
    libgl1 \
    libglib2.0-0 \
    libsm6 \
    libxext6 \
    libxrender1 \
    libfontconfig1 \
    libx11-dev

# Instalar dependências do Python
COPY requirements.txt .
RUN pip install -r requirements.txt

# Copiar o código do app
COPY . /app

# Definir o diretório de trabalho
WORKDIR /app

# Comando para rodar a aplicação
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]
