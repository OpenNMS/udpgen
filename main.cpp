#include <iostream>
#include <thread>
#include <atomic>
#include <cstring>

#include "syslog_generator.hpp"
#include "trap_generator.hpp"
#include "netflow5_generator.hpp"

void daemonize();

int main(int argc, char **argv) {
    const int HOST_LEN = 512;
    char host[HOST_LEN];
    memset(host, 0, HOST_LEN);
    int port = -1;
    int rate = -1;
    char daemon = 0;
    char interactive = 1;
    int num_threads = -1;
    unsigned int num_packets_per_send = 0;
    enum payloads {SNMP, SYSLOG, NETFLOW5} payload = SYSLOG;

    int c;
    while ((c = getopt(argc, argv, "dih:p:r:t:x:z:")) != -1) {
        switch (c) {
            case 'd':
                daemon = 1;
                break;
            case 'i':
                interactive = 0;
                break;
            case 'h':
                strncpy(host, optarg, HOST_LEN);
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'r':
                rate = atoi(optarg);
                break;
            case 't':
                num_threads = atoi(optarg);
                break;
            case 'x':
                if (strcmp(optarg, "snmp") == 0) {
                    payload = SNMP;
                } else if (strcmp(optarg, "syslog") == 0) {
                    payload = SYSLOG;
                } else if (strcmp(optarg, "netflow5") == 0) {
                    payload = NETFLOW5;
                } else {
                    printf("Invalid payload type: %s\n", optarg);
                    return 1;
                }
                break;
            case 'z':
                num_packets_per_send = atoi(optarg);
                break;
            default:
                printf("\nUsage: udpgen [-d] [-i] [-h host] [-p port] [-r rate] [-t threads] [-z packets] [-x type]\n\n");
                printf("  -x: Type of payload: snmp, syslog, or netflow5 (default: syslog)\n");
                printf("  -d: Daemonize (default: false)\n");
                printf("  -i: Disable interactivity (default: false)\n");
                printf("  -h: Target host / IP address (default: 127.0.0.1)\n");
                printf("  -p: Target port (default: depends on mode)\n");
                printf("  -r: Rate - number of packets per second to generate (default: 10000)\n");
                printf("             set the rate to 0 in order to disable rate limiting\n");
                printf("  -t: Number of threads used to generate packets (default: 1)\n");
                printf("  -z: Number of packets per iteration (default: 1)\n");
                printf("             increase this when sending packets at a high rate\n\n");
                return 1;
        }
    }

    if (daemon) {
        daemonize();
    }

    std::unique_ptr<UDPGenerator> generator;
    switch (payload) {
        case SNMP: generator = std::unique_ptr<UDPGenerator>(new TrapGenerator()); break;
        case SYSLOG: generator = std::unique_ptr<UDPGenerator>(new SyslogGenerator()); break;
        case NETFLOW5: generator = std::unique_ptr<UDPGenerator>(new Netflow5Generator()); break;
    }

    if (strlen(host) > 0) {
        generator->setHost(host);
    }
    if (port > 0) {
        generator->setPort(port);
    }
    if (rate >= 0) {
        generator->setPacketsPerSecond(rate);
    }
    if (num_threads > 0) {
        generator->setNumThreads(num_threads);
    }
    if (num_packets_per_send > 0) {
        generator->setNumPacketsPerSend(num_packets_per_send);
    }

    if (generator->getPacketsPerSecond() > 10000 && generator->getNumPacketsPerSend() < 10) {
        printf("\nWARNING: Packet rate limiting breaks down at high rates. "
                       "Consider increasing the number of packets per send.\n\n");
    }

    printf("Sending %s to %s:%d at target rate of %.2f packets per seconds across %d thread(s).\n",
           generator->getPacketDescription(), generator->getHost(), generator->getPort(), generator->getPacketsPerSecond(),
           generator->getNumThreads());

    if (generator->getPacketsPerSecond() > 0) {
        printf("\nThe number of packets sent should be printed every %d seconds.\n"
                       "If the more than %d seconds elapses between the reports, "
                       "the program is unable to generate packets at the requested rate.\n"
                       "You can try consider increasing the number of threads.\n\n",
               generator->getReportInterval(), generator->getReportInterval());
    }

    generator->start();
    if (interactive) {
        char waitForKey;
        std::cout << "Type q + enter to exit..." << std::endl;
        std::cin >> waitForKey;
    } else {
        pause();
    }
    generator->stop();
    return 0;
}

void daemonize() {
    FILE *fp= NULL;
    pid_t process_id = 0;
    pid_t sid = 0;
    // Create child process
    process_id = fork();
    // Indication of fork() failure
    if (process_id < 0) {
        printf("fork failed!\n");
        // Return failure in exit status
        std::exit(1);
    }
    // PARENT PROCESS. Need to kill it.
    if (process_id > 0) {
        printf("process_id of child process %d \n", process_id);
        // return success in exit status
        std::exit(0);
    }
    //unmask the file mode
    //umask(0);
    //set new session
    sid = setsid();
    if (sid < 0) {
        // Return failure
        std::exit(1);
    }
    // Change the current working directory to tmp.
    chdir("/tmp");
    // Close stdin. stdout and stderr
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}
