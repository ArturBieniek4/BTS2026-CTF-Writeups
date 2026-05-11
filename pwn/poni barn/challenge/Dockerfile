FROM ubuntu:22.04

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        qemu-system-arm \
        socat \
    && rm -rf /var/lib/apt/lists/*

RUN useradd -m -s /bin/false ctf

WORKDIR /home/ctf
COPY rom.elf .
COPY flag.txt .
COPY serve.sh .
RUN chmod +x serve.sh && chown -R ctf:ctf /home/ctf

USER ctf
EXPOSE 1337

CMD ["/home/ctf/serve.sh"]
