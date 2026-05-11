FROM node:22-slim

RUN apt-get update && apt-get install -y chromium --no-install-recommends && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY package.json .
RUN npm install --omit=dev
COPY . .

EXPOSE 8000
CMD ["sh", "start.sh"]
