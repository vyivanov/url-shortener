FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
COPY ./ /opt/url-shortener
WORKDIR /opt/url-shortener

RUN \
    apt update && apt install -y \
        build-essential          \
        cmake                    \
        python3                  \
        python3-pip              \
 && pip3 install conan           \
    \
 && conan profile new --detect default                                \
 && conan profile update settings.compiler.libcxx=libstdc++11 default \
 && conan profile update settings.compiler.cppstd=17 default          \
    \
 && mkdir ./build-in-docker && cd ./build-in-docker \
 && conan install .. --build=missing                \
 && cmake .. -DCMAKE_BUILD_TYPE=Debug               \
 && make -j$(nproc --all)

FROM ubuntu:20.04

WORKDIR /root
COPY --from=0 /opt/url-shortener/build-in-docker/bin/url-shortener ./
COPY --from=0 /opt/url-shortener/html ./html

EXPOSE 9080
ENTRYPOINT ["/root/url-shortener"]
