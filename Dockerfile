FROM debian:latest

COPY . /app
WORKDIR /app

RUN apt update && apt upgrade -y
RUN apt install cmake make clang gcc g++ -y
RUN sh dependencies.sh
RUN mkdir -p build && cd build
RUN cmake ..
RUN make

EXPOSE 5588

CMD ["./build/SMS_general"]