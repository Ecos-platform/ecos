
FROM ubuntu:22.04

RUN DEBIAN_FRONTEND="noninteractive"

RUN apt-get update \
  && apt-get install -y build-essential \
    git \
    cmake \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    libtbb-dev \
  && apt-get clean

WORKDIR /home/ecos
COPY . .
RUN cmake . -B build -DECOS_BUILD_EXAMPLES=ON -DECOS_BUILD_TESTS=ON -DCMAKE_BUILD_TYPE="Release"
RUN cmake --build build
