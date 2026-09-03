FROM debian:bookworm-slim AS backend-builder
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake libcurl4-openssl-dev nlohmann-json3-dev ca-certificates \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY CMakeLists.txt ./
COPY include ./include
COPY src ./src
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc)

FROM node:20-alpine AS frontend-builder
WORKDIR /app
COPY frontend/package.json frontend/package-lock.json ./frontend/
RUN cd frontend && npm ci
COPY frontend ./frontend
RUN cd frontend && npm run build

FROM debian:bookworm-slim AS backend
RUN apt-get update && apt-get install -y --no-install-recommends libcurl4 ca-certificates curl && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY --from=backend-builder /app/build/fincon ./fincon
COPY --from=backend-builder /app/build/fincon_evaluation ./fincon_evaluation
COPY src/.env.example ./src/.env.example
RUN mkdir -p src
EXPOSE 8080
HEALTHCHECK --interval=10s --timeout=3s --retries=3 --start-period=10s CMD curl -f http://localhost:8080/health || exit 1
CMD ["./fincon"]

FROM nginx:alpine AS frontend
COPY --from=frontend-builder /app/frontend/dist /usr/share/nginx/html
COPY nginx.conf /etc/nginx/conf.d/default.conf
EXPOSE 80
HEALTHCHECK --interval=10s --timeout=3s --retries=3 CMD wget -qO- http://localhost/ | grep -q FinCon || exit 1
