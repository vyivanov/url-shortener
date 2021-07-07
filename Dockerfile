FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
COPY ./ /opt/url-shortener
WORKDIR /opt

RUN \
    apt update && apt install -y \
        wget build-essential     \
        git cmake                \
        devscripts debhelper     \
        python3 python3-pip      \
 && pip3 install conan           \
    \
 && conan profile new --detect default                                \
 && conan profile update settings.compiler.libcxx=libstdc++11 default \
 && conan profile update settings.compiler.cppstd=17 default          \
    \
 && git clone https://github.com/vyivanov/hashidsxx.git \
 && cd hashidsxx/ && debuild -uc -us && cd ../          \
 && dpkg -i hashidsxx_1.0.0_amd64.deb                   \
    \
 && cd url-shortener/                             \
 && mkdir build-in-docker/ && cd build-in-docker/ \
 && conan install .. --build=missing              \
 && cmake .. -DCMAKE_BUILD_TYPE=Debug             \
 && make -j$(nproc --all)

# TODO: move to debuild
# TODO: get rid of conan

FROM ubuntu:20.04

WORKDIR /root
COPY --from=0 /opt/url-shortener/build-in-docker/bin/url-shortener ./
COPY --from=0 /opt/url-shortener/img/favicon.ico ./
COPY --from=0 /opt/url-shortener/html ./html

EXPOSE 9080
ENTRYPOINT ["/root/url-shortener"]
