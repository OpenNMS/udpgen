# udpgen [![CircleCI](https://circleci.com/gh/OpenNMS/udpgen.svg?style=svg)](https://circleci.com/gh/OpenNMS/udpgen)

## Overview

This tool was written to help stress test OpenNMS' UDP protocol handling by generating large volumes of traffic.

It currently supports generating:
* SNMP Traps
* Syslog Messages
* Netflow 5 flows
* Netflow 9 flows

The tool does not currently give fine grained control over the payload generation.

## Building

### Requirements

* cmake
* net-snmp-devel

### Compiling

```sh
mkdir build
cd build
cmake ..
make
```

## Usage

`udpgen [-d] [-i] [-h host] [-p port] [-r rate] [-t threads] [-z packets] [-x type]`
* `-x`: Type of payload: snmp, syslog, netflow5, netflow9 (default: syslog)
* `-d`: Daemonize (default: false)
* `-i`: Disable interactivity (default: false)
* `-h`: Target host / IP address (default: 127.0.0.1)
* `-p`: Target port (default: depends on mode)
* `-r`: Rate - number of packets per second to generate (default: 10000) Set the rate to 0 in order to disable rate limiting
* `-s`: Send X number of packets and then stop
* `-S`: Senx packets for X number of seconds and then stop
* `-t`: Number of threads used to generate packets (default: 1)
* `-z`: Number of packets per iteration (default: 1) Increase this when sending packets at a high rate

## Examples

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

### Generate Netflow 9 flows

Generate as many Netflow 9 flows as possible using a single thread pinned to the first core:

```sh
taskset -c 0 ./udpgen -x netflow9 -r 0
```

