FROM python:bullseye

# Set the working directory
WORKDIR /app

# Install dependencies
RUN apt-get update && apt-get install -y \
    alsa-utils \
    sox \
    libsox-fmt-mp3 \
    && rm -rf /var/lib/apt/lists/*

# Install FastAPI
RUN pip install fastapi uvicorn

# Copy the app files to the container
COPY . /app

# Expose the port on which the app will run (change if necessary)
EXPOSE 8000

# Start the app
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]

