FROM python:3.11-slim

RUN apt-get update && \
    apt-get upgrade && \
    apt-get install -y \
    libgl1 \
    libglib2.0-0 \
    libsm6 \
    libxext6 \
    libxrender1 \
    libfontconfig1 \
    libx11-dev

COPY requirements.txt .

RUN pip install -r requirements.txt

COPY . .

WORKDIR /app

CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]
