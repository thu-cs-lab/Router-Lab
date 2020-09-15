FROM debian:stretch
RUN sed -i 's/deb.debian.org/mirrors.tuna.tsinghua.edu.cn/g' /etc/apt/sources.list
RUN apt update
RUN apt install -y python3-pip python3-py python3-lxml libpcap-dev libxml2-dev libxslt-dev
RUN pip3 install -i https://pypi.tuna.tsinghua.edu.cn/simple pyshark

