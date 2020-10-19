FROM debian:stretch
RUN sed -i 's/deb.debian.org/mirrors.tuna.tsinghua.edu.cn/g' /etc/apt/sources.list
RUN sed -i 's/security.debian.org/mirrors.tuna.tsinghua.edu.cn/g' /etc/apt/sources.list
RUN apt update
RUN apt install -y python3-pip python3-py python3-lxml libpcap-dev libxml2-dev libxslt-dev gpg
RUN pip3 install -i https://pypi.tuna.tsinghua.edu.cn/simple pyshark
RUN gpg --keyserver hkp://keyserver.ubuntu.com --recv-keys E2BE4761926ABDB7A9525790D9006F13D7DB311A


