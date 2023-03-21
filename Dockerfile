FROM ubuntu:20.04

ENV TZ=Europe/Stockholm
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && apt-get install -y vim git make gcc bison flex libssl-dev bc ncurses-dev kmod xa65 texinfo curl tar xz-utils automake-1.15

RUN curl -Lo gcc-arm-none-eabi.tar.bz2 "https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2?revision=05382cca-1721-44e1-ae19-1e7c3dc96118&rev=05382cca172144e1ae191e7c3dc96118&hash=FDE675133A099796BD1507A3FF215AC4"
RUN mkdir /opt/gcc-arm-none-eabi
RUN tar xvjf gcc-arm-none-eabi.tar.bz2 --strip-components=1 -C /opt/gcc-arm-none-eabi
ENV ARM_HOME=/opt/gcc-arm-none-eabi
ENV PATH=$PATH:$ARM_HOME/bin
ENV ARM_VERSION=9.3.1
RUN mkdir /build
RUN git -C /build clone --recursive https://github.com/henrikenblom/bmc64.git
