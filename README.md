# udpgen

## Overview

Yet another tool used to generate SNMPv2 traps and Syslog messages.

This tool was written to help stress test OpenNMS' Syslog and SNMP trap listeners, by generating large volumes of traffic.

## Building

### Requirements

* cmake
* netsnmp-devel

### Compiling

```sh
mkdir build
cd build
cmake ..
make
```

## Usage

### Generate Syslog Message

Generate 100000 Syslog messages per second over 10 threads, targeted at 172.23.1.1:514.

```sh
./udpgen -r 100000 -t 10 -h 172.23.1.1 -p 514
```

Pin `udpgen` to the first core, and generate as many Syslog messages as possible using a single thread.
```sh
taskset -c 0 ./udpgen -r 0
```

### Generate SNMP Traps

Generate 200000 SNMPv2 traps per second over 8 threads, targeted at 127.0.0.1:1162.

```sh
./udpgen -x snmp -r 200000 -t 8
```
