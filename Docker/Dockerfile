FROM debian:bullseye
RUN sed -i 's/deb.debian.org/mirrors.tuna.tsinghua.edu.cn/g' /etc/apt/sources.list
RUN sed -i 's/security.debian.org/mirrors.tuna.tsinghua.edu.cn/g' /etc/apt/sources.list
RUN apt update
RUN apt install -y python3-pip python3-py python3-lxml libxml2-dev libxslt-dev gpg wget
RUN pip3 install -i https://pypi.tuna.tsinghua.edu.cn/simple pyshark
RUN gpg --keyserver hkp://keyserver.ubuntu.com --recv-keys E2BE4761926ABDB7A9525790D9006F13D7DB311A
RUN wget http://mirrors.tuna.tsinghua.edu.cn/debian/pool/main/libp/libpcap/libpcap_1.10.0.orig.tar.gz
RUN tar xvf libpcap_1.10.0.orig.tar.gz
WORKDIR libpcap-1.10.0
RUN apt install -y flex bison
RUN ./configure --prefix=/usr --disable-dbus --disable-rdma --disable-usb --enable-shared
RUN make -j4 install

