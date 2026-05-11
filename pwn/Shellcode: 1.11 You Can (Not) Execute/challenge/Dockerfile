FROM ubuntu@sha256:cc925e589b7543b910fea57a240468940003fbfc0515245a495dd0ad8fe7cef1

RUN apt update && apt install -y socat

WORKDIR /app
RUN useradd ctf

COPY ./a.out /app/a.out
COPY ./libponi.so /app/libponi.so
COPY ./flag.txt /app/flag.txt
COPY ./ld-linux.so.2 /app/ld-linux.so.2
RUN chmod +x /app/a.out
RUN chmod 444 /app/flag.txt

USER ctf

EXPOSE 1337
CMD socat tcp-l:1337,reuseaddr,fork exec:/app/a.out