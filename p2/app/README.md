# Documentação do Backend de Detecção de Rosto sincronizado com um IOT

## Descrição

Este backend foi desenvolvido para receber imagens enviadas por um dispositivo IoT (ESP32-CAM), detectar rostos utilizando OpenCV e salvar a imagem processada com os rostos destacados localmente. O sistema foi construído com FastAPI e OpenCV, e utiliza Docker para facilitar a execução em ambientes padronizados.

## Estrutura do Projeto

```
.
├── Dockerfile               # Arquivo Docker para criar o container
├── requirements.txt         # Dependências do Python
├── app/
│   ├── main.py              # Arquivo principal com a lógica da API
│   └── images/              # Diretório onde as imagens processadas são salvas
└── README.md                # Este arquivo de documentação
```

## Pré-requisitos

- [Docker](https://www.docker.com/get-started) instalado na sua máquina
- Dispositivo ESP32-CAM configurado para capturar imagens e enviá-las para o backend

## Passos para Executar a Aplicação com Docker

### 1. Construir a Imagem Docker

Acesse o diretório do projeto onde o `Dockerfile` está localizado e execute o comando para criar a imagem Docker:

```bash
docker build -t backend .
```

### 2. Executar o Container

Após construir a imagem, você pode rodar o container com o comando abaixo. Este comando irá mapear a porta `8000` do container para a porta `8000` do host.

```bash
docker run -p 8000:8000 -v $(pwd)/app/images:/app/images backend
```

* pwd: Diretório local para salvar os arquivos vindos do Container Docker /app/images

- **`-p 8000:8000`**: Mapeia a porta 8000 do container para a porta 8000 do host, onde o FastAPI estará rodando.
- **`-v $(pwd)/app/images:/app/images`**: Mapeia o diretório local onde as imagens são salvas, garantindo que as imagens processadas fiquem acessíveis fora do container.

Agora o backend está rodando na sua máquina e pode ser acessado em `http://localhost:8000`.

### 3. Testar o Endpoint de Upload

Você pode testar o upload de uma imagem para o backend utilizando um cliente HTTP como o Postman ou o `curl`:

```bash
curl -X POST "http://localhost:8000/upload" -H "accept: application/json" -H "Content-Type: multipart/form-data" -F "file=@caminho/para/sua/imagem.jpg"
```

Substitua `caminho/para/sua/imagem.jpg` pelo caminho real da imagem que você deseja enviar.

### 4. Receber Imagens do ESP32-CAM

O ESP32-CAM está configurado para capturar imagens e enviar uma requisição HTTP POST para o backend. A URL de envio é:

```
http://<ENDEREÇO_IP_DO_BACKEND>:8000/upload
```

Onde `<ENDEREÇO_IP_DO_BACKEND>` é o IP do servidor onde o container Docker está rodando.

## Funcionamento do Backend

### Recebimento da Imagem

O backend possui um endpoint `/upload`, que recebe imagens no formato multipart/form-data. As imagens enviadas pelo ESP32-CAM são processadas para detectar rostos.

### Detecção de Rosto

Utilizando o OpenCV, o backend carrega o classificador Haar (`haarcascade_frontalface_default.xml`) para detectar rostos. Após a detecção, ele desenha um retângulo ao redor do rosto e salva a imagem processada localmente no diretório `/app/images`.

### Nomeação das Imagens

As imagens são salvas no formato `received_image-<TIMESTAMP>.jpg`, onde `<TIMESTAMP>` representa a data e hora da captura, garantindo que cada imagem tenha um nome único. Exemplo de nome de arquivo gerado:

```
received_image-24092024143015.jpg
```

### Diretório de Salvamento

As imagens processadas são salvas no diretório `/app/images`, que foi mapeado para o sistema de arquivos do host via Docker volume, permitindo acesso às imagens fora do container.

Para complementar a documentação do seu backend que processa imagens enviadas pelo ESP32-CAM e as detecta para rostos, aqui está a adição sobre a nova rota `/get_faces` que foi criada para fornecer as coordenadas dos rostos detectados:

## Rota `/get_faces` para Coordenadas de Detecção de Rosto

### Finalidade

A rota `/get_faces` foi adicionada ao backend para permitir que dispositivos ou serviços externos recuperem as coordenadas dos rostos detectados mais recentemente. Isso é útil para aplicações que precisam de feedback em tempo real sobre a localização dos rostos numa série de imagens capturadas.

### Funcionamento

Quando uma imagem é enviada ao endpoint `/upload` e processada, as coordenadas dos rostos detectados são armazenadas temporariamente numa lista no servidor. Essa lista é então acessível através do endpoint GET `/get_faces`.

### Formato da Resposta

A resposta deste endpoint é um objeto JSON que contém uma lista de coordenadas dos rostos detectados. Cada coordenada na lista é representada como uma tupla de quatro elementos `(x, y, w, h)`, onde `x` e `y` são as coordenadas do canto superior esquerdo do retângulo que envolve o rosto, `w` é a largura do retângulo, e `h` é a altura.

#### Exemplo de resposta:

```json
{
  "faces": [
    [30, 50, 100, 150],
    [120, 80, 90, 110]
  ]
}
```

## Dependências

As principais dependências da aplicação estão listadas no arquivo `requirements.txt`. As bibliotecas mais importantes são:

- **FastAPI**: Framework para construir APIs rápidas em Python.
- **OpenCV**: Biblioteca de visão computacional para detecção de rostos.
  
As dependências do sistema, incluindo bibliotecas gráficas necessárias para o OpenCV, são instaladas no Dockerfile.

## Arquivo `Dockerfile`

Este é o conteúdo do `Dockerfile` que configura o ambiente Docker para a aplicação:

```Dockerfile
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

# Criar diretório para salvar imagens
RUN mkdir -p /app/images

# Comando para rodar a aplicação
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]
```

# Documentação Arduino - ESP32-CAM

## Descrição

Este código é utilizado para capturar imagens de uma ESP32-CAM e enviá-las para um backend FastAPI que processa e detecta rostos nas imagens enviadas. O dispositivo se conecta a uma rede Wi-Fi e envia imagens via HTTP para o servidor especificado.

## Configurações de Hardware

A ESP32-CAM deve estar conectada corretamente e configurada com os seguintes pinos:

- **Camera Model**: AI-THINKER
- **Pinos GPIO**: 
  - PWDN_GPIO_NUM: 32
  - SIOD_GPIO_NUM: 26
  - SIOC_GPIO_NUM: 27
  - VSYNC_GPIO_NUM: 25
  - Outros pinos especificados no código

## Conexão Wi-Fi

O dispositivo se conecta a uma rede Wi-Fi utilizando as credenciais fornecidas:

```cpp
const char *ssid = "Inteli.Iot";
const char *password = "@Intelix10T#";
```

Você pode alterar o nome da rede (SSID) e a senha para corresponder à sua rede local.

## Envio de Imagens

As imagens capturadas pela ESP32-CAM são enviadas para o backend FastAPI. O URL do servidor é definido da seguinte forma:
***Altere o endereço de Ip conforme disponibilizado pela rede: Use os comandos ifconfig, hostname -I ou ip a (Ubuntu) para descobrir o ip de rede***

```cpp
const char* serverUrl = "http://{ip-de-rede}:8000/upload";
```

Certifique-se de ajustar o IP para o endereço correto do seu servidor.

## Ciclo de Captura e Envio

O loop principal captura uma imagem da câmera a cada 5 segundos e envia a imagem para o backend.

```cpp
delay(5000); // Aguardar 5 segundos antes de capturar a próxima imagem
```

Esse tempo pode ser ajustado conforme necessário.

## Estrutura do Código

- **Configuração Wi-Fi**: A ESP32-CAM tenta se conectar à rede Wi-Fi assim que inicia.
- **Inicialização da Câmera**: O código configura os parâmetros da câmera e inicializa a câmera.
- **Captura e Envio de Imagens**: A cada ciclo do loop, o dispositivo captura uma imagem e a envia ao backend usando HTTP POST com multipart/form-data.

## Referências de Estudos

1. [Vídeo no YouTube](https://www.youtube.com/watch?v=LOqVIe9cnW8)
2. [Usina Info - Programando ESP32-CAM Wi-Fi](https://www.usinainfo.com.br/blog/programando-esp32-cam-wifi-com-esp32-cam-mb/)
3. [Random Nerd Tutorials - Streaming de Vídeo e Web Server](https://randomnerdtutorials.com/esp32-cam-video-streaming-web-server-camera-home-assistant/) (Principal)

## Video de FUncionamento

link : https://drive.google.com/file/d/1WqQAlM_rFnWhVKW_rLEUG5wxOOtBrRRj/view?usp=sharing