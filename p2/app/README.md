# Detecção de Rostos com FastAPI

Este projeto implementa uma API simples para detecção de rostos em imagens utilizando FastAPI, OpenCV e NumPy. As imagens enviadas pelo usuário são processadas para detectar rostos, e as coordenadas dos rostos são retornadas via API. A aplicação é executada em um container Docker.

## Funcionalidades

- **Upload de Imagens**: Envie uma imagem para a API, e ela retornará a imagem processada com a detecção de rostos.
- **Detecção de Rostos**: Detecta e marca rostos em imagens usando um classificador de cascata Haar (Haar Cascade Classifier).
- **Listagem de Rostos**: Consulte as coordenadas dos rostos detectados.

## Pré-requisitos

- **Docker**: Certifique-se de ter o Docker instalado na sua máquina. Se não tiver, siga as instruções em [Docker Official Website](https://www.docker.com/).

## Como Rodar

1. **Clone o repositório**:
    ```bash
    git clone https://github.com/gabrielpascoli/M11.git
    cd M11
    ```

2. **Construa a imagem Docker**:
    ```bash
    docker build -t deteccao-de-rostos .
    ```

3. **Execute o container Docker**:
    ```bash
    
    docker run -p 8000:8000 -v $(pwd)/app/imagens:/app/imagens deteccao-de-rostos
    ```

4. **Acesse a API**:
   - Acesse `http://localhost:8000/docs` no seu navegador para visualizar a documentação da API gerada automaticamente pelo FastAPI.
   - Utilize o **/upload** para enviar imagens e o **/ver_rostos** para ver os rostos detectados.

## Endpoints da API

### `POST /upload`

Envia uma imagem para detecção de rostos. O arquivo deve ser enviado como multipart/form-data.

- **Exemplo de Requisição**:
    ```bash
    curl -X 'POST' \
      'http://localhost:8000/upload' \
      -H 'accept: application/json' \
      -H 'Content-Type: multipart/form-data' \
      -F 'arquivo=@/caminho/para/sua/imagem.jpg'
    ```

- **Resposta**:
    ```json
    {
      "mensagem": "Imagem enviada com sucesso e salva como /app/images/imagem_detectada-08052021153015.jpg"
    }
    ```

### `GET /ver_rostos`

Retorna as coordenadas dos rostos detectados na última imagem enviada.

- **Resposta**:
    ```json
    {
      "rostos_detectados": [
        [100, 100, 150, 150],
        [300, 200, 100, 100]
      ]
    }
    ```

## Estrutura do Projeto

```
├── main.py                 # Código principal da API
├── Dockerfile              # Arquivo Docker para build da imagem
├── requirements.txt        # Dependências do projeto
└── README.md               # Documentação do projeto
```

## Dependências

- FastAPI
- OpenCV
- NumPy
- Uvicorn

Essas dependências são instaladas automaticamente ao construir a imagem Docker com base no arquivo `requirements.txt`.