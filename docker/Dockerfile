FROM alpine:3.15.0 AS base

RUN apk add --no-cache \
    libstdc++ \
    libgcc \
    libtool \
    pkgconfig \
    openssh


#------------------
FROM base AS build

RUN apk add --no-cache \
    cmake \
    g++ \
    make \
    linux-headers

# build and install application
COPY ./ /source
RUN cd /source \
  && mkdir build \
  && cd build \
  && cmake .. -DCMAKE_BUILD_TYPE=Release -DWITH_CPPLINT=OFF \
  && make -j8 \
  && make install


#------------------
FROM base AS hostcontrold

COPY --from=build /usr/sbin/hostcontrold /bin/
COPY ./docker/entrypoint.sh /bin/

# allow non-root users to use ping
RUN chmod u+s /bin/ping

CMD ["/bin/entrypoint.sh"]
