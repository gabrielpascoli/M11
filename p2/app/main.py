from fastapi import FastAPI, File, UploadFile
import numpy as np
import cv2
from datetime import datetime

app = FastAPI()

detected_faces = []  # Lista para armazenar as coordenadas dos rostos detectados

@app.get("/")
def root():
    return {"message": "Hello World"}

# Carregar o classificador em cascata para detecção de rostos
face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')

@app.post("/upload")
async def upload_image(file: UploadFile = File(...)):
    # Ler o conteúdo do arquivo enviado
    content = await file.read()
    
    # Converter o conteúdo do arquivo para um array numpy
    np_array = np.frombuffer(content, np.uint8)
    
    # Decodificar a imagem do array numpy
    image = cv2.imdecode(np_array, cv2.IMREAD_COLOR)
    
    # Converter a imagem para escala de cinza
    gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    
    # Detectar rostos na imagem
    faces = face_cascade.detectMultiScale(gray, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))

    # Armazenar as coordenadas dos rostos detectados
    detected_faces = [(x, y, w, h) for (x, y, w, h) in faces]
    
    # Desenhar retângulos ao redor dos rostos detectados
    for (x, y, w, h) in faces:
        cv2.rectangle(image, (x, y), (x + w, y + h), (255, 0, 0), 2)
    
    # Obter a data e hora atual
    timestamp = datetime.now().strftime("%d%m%Y%H%M%S")
    
    # Criar o nome do arquivo com o timestamp
    filename = f"/app/images/received_image-{timestamp}.jpg"
    
    # Salvar a imagem processada no diretório especificado
    cv2.imwrite(filename, image)
    
    return {"message": f"Imagem recebida e salva como {filename}"}

@app.get("/get_faces")
def get_faces():
    # Retornar as coordenadas dos rostos detectados - Ponderada 3
    return {"faces": detected_faces}

if __name__ == '__main__':
    import uvicorn
    uvicorn.run(app, host="0.0.0.0" , port=8000, debug=True)