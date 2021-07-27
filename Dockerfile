FROM ubuntu:20.04 AS build

ENV DEBIAN_FRONTEND=noninteractive
COPY ./ /opt/url-shortener

RUN \
    apt update && apt install -y \
         wget                    \
         build-essential         \
         git                     \
         cmake                   \
         devscripts              \
         debhelper               \
         python3                 \
         python3-pip             \
         meson                   \
         cppcheck                \
         dh-exec                 \
         libcurl4-openssl-dev    \
         libgtest-dev            \
         libssl-dev              \
         pkg-config              \
         rapidjson-dev           \
         valgrind                \
 && pip3 install conan           \
    \
 && conan profile new --detect default                                \
 && conan profile update settings.compiler.libcxx=libstdc++11 default \
 && conan profile update settings.compiler.cppstd=17 default          \
    \
 && cd /opt/                                            \
 && git clone https://github.com/vyivanov/hashidsxx.git \
 && cd hashidsxx/ && debuild -uc -us && cd ../          \
 && dpkg -i hashidsxx_1.0.0_amd64.deb                   \
    \
 && cd /opt/url-shortener/deps/                                \
 && mkdir pistache_0.0.002/                                    \
 && tar zxvf pistache_0.0.002.orig.tar.gz -C pistache_0.0.002/ \
 && cd pistache_0.0.002/ && debuild -uc -us && cd ../          \
 && dpkg -i libpistache0_0.0.002-pistache1_amd64.deb           \
 && dpkg -i libpistache-dev_0.0.002-pistache1_amd64.deb        \
    \
 && cd /opt/url-shortener/                        \
 && mkdir build-in-docker/ && cd build-in-docker/ \
 && conan install .. --build=missing              \
 && cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release   \
 && ninja all

FROM ubuntu:20.04 AS deploy

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /var/www

COPY --from=build /opt/url-shortener/build-in-docker/bin/url-shortener ./
COPY --from=build /opt/url-shortener/entrypoint.sh ./
COPY --from=build /opt/url-shortener/img/favicon.ico ./
COPY --from=build /opt/url-shortener/html ./html

RUN \
    apt update && apt install -y postgresql-client \
    \
 && rm -rf /var/lib/apt/lists/* \
    \
 && chmod +x entrypoint.sh

EXPOSE 9080
EXPOSE 9443

ENTRYPOINT ["/var/www/entrypoint.sh"]
