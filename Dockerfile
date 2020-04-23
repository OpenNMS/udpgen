FROM ubuntu:20.04
LABEL license="AGPLv3" \
      vendor="The OpenNMS Group, Inc." \
      name="udpgen"
RUN apt update && apt install -y \
    libsnmp35 \
&& rm -rf /var/lib/apt/lists/*
COPY build/udpgen udpgen
CMD ["/udpgen", "-i", "-r", "1",  "-t", "1", "-h", "127.0.0.1", "-p", "514"]
