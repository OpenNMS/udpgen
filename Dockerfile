FROM ubuntu:jammy-20231128

RUN apt-get update && \
    apt-get install -y libsnmp40 \
      net-tools \
      iputils-ping \
      tcpdump && \
      rm -rf /var/lib/apt/lists/*

COPY build/udpgen udpgen

ENTRYPOINT [ "/udpgen" ]

LABEL org.opencontainers.image.description="udpgen - A cli tool to generate SNMP Traps, Syslog and NetFlow v9 packets" \
      org.opencontainers.image.source="https://github.com/opennms/udpgen.git" \
      org.opencontainers.image.vendor="The OpenNMS Group, Inc." \
      org.opencontainers.image.authors="ronny@opennms.org" \
      org.opencontainers.image.licenses="AGPLv3"
