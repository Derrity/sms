FROM debian:11
COPY build/server.txt /app/server.txt
COPY frontend /app/frontend
COPY build/SMS_general /app/SMS_general
WORKDIR /app

RUN apt update
RUN apt install cmake clang gcc g++ libasio-dev curl libcurl4-openssl-dev libssl-dev libboost-all-dev make aria2 git -y


EXPOSE 5588

CMD ["./SMS_general"]
