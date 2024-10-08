from fastapi import FastAPI, File, UploadFile
import numpy as np
import cv2
from datetime import datetime

app = FastAPI()

rostos_detectados = []

detetive_de_rostos = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')

@app.post("/upload")
async def enviar_imagem(arquivo: UploadFile = File(...)):
    conteudo = await arquivo.read()
    np_array = np.frombuffer(conteudo, np.uint8)
    imagem = cv2.imdecode(np_array, cv2.IMREAD_COLOR)
    tons_de_cinza = cv2.cvtColor(imagem, cv2.COLOR_BGR2GRAY)
    rostos = detetive_de_rostos.detectMultiScale(tons_de_cinza, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))

    rostos_detectados.clear()
    rostos_detectados.extend([(x, y, w, h) for (x, y, w, h) in rostos])

    for (x, y, w, h) in rostos:
        cv2.rectangle(imagem, (x, y), (x + w, y + h), (255, 0, 0), 2)

    timestamp = datetime.now().strftime("%d%m%Y%H%M%S")
    nome_do_arquivo = f"/app/images/imagem_detectada-{timestamp}.jpg"
    cv2.imwrite(nome_do_arquivo, imagem)

    return {"mensagem": f"Imagem enviada com sucesso e salva como {nome_do_arquivo}"}

@app.get("/ver_rostos")
def ver_rostos():
    return {"rostos_detectados": rostos_detectados}

if __name__ == '__main__':
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8000, debug=True)