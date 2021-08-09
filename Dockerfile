FROM ubuntu:20.04 AS build

COPY ./ /opt/url-shortener

RUN \
    apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
         wget                    \
         git                     \
         unzip                   \
         lintian                 \
         build-essential         \
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
         patch                   \
 && pip3 install conan           \
    \
 && conan profile new --detect default                                \
 && conan profile update settings.compiler.libcxx=libstdc++11 default \
 && conan profile update settings.compiler.cppstd=17 default          \
    \
 && cd /opt/url-shortener/deps/ \
    \
 && mkdir hashidsxx_1.0.0.orig/                                   \
 && tar zxvf hashidsxx_1.0.0.orig.tar.gz -C hashidsxx_1.0.0.orig/ \
 && cd hashidsxx_1.0.0.orig/ && debuild -uc -us && cd ../         \
 && dpkg -i hashidsxx_1.0.0_amd64.deb                             \
    \
 && mkdir pistache_0.0.002.orig/                                    \
 && tar zxvf pistache_0.0.002.orig.tar.gz -C pistache_0.0.002.orig/ \
 && patch -p0 < pistache.001.native-format.patch                    \
 && patch -p0 < pistache.002.ssl-disable.patch                      \
 && cd pistache_0.0.002.orig/ && debuild -uc -us && cd ../          \
 && dpkg -i libpistache0_0.0.002-pistache1_amd64.deb                \
 && dpkg -i libpistache-dev_0.0.002-pistache1_amd64.deb             \
    \
 && cd /opt/url-shortener/ \
    \
 && mkdir build-in-docker/ && cd build-in-docker/ \
 && conan install .. --build=missing              \
 && cmake .. -GNinja -DCMAKE_BUILD_TYPE=Release   \
 && ninja all

FROM ubuntu:20.04 AS deploy

WORKDIR /var/www

COPY --from=build /opt/url-shortener/build-in-docker/bin/url-shortener ./
COPY --from=build /opt/url-shortener/entrypoint.sh ./
COPY --from=build /opt/url-shortener/img/favicon.ico ./
COPY --from=build /opt/url-shortener/html ./html

RUN \
    apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
         postgresql-client \
    \
 && apt-get clean               \
 && rm -rf /var/lib/apt/lists/* \
    \
 && chmod +x entrypoint.sh

EXPOSE 9080
ENTRYPOINT ["/var/www/entrypoint.sh"]
