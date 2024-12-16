#!/bin/bash

SERVER_IP="x.x.x.x"
TCP_PORT=8080
UDP_PORT=9090

send_tcp_traffic() {
    echo "sending TCP traffic to ${SERVER_IP}:${TCP_PORT}..."
    echo "hello from TCP client!!" | nc ${SERVER_IP} ${TCP_PORT}
}

send_udp_traffic() {
    echo "sending UDP traffic to ${SERVER_IP}:${UDP_PORT}..."
    echo "hello from UDP client!!" | nc -u ${SERVER_IP} ${UDP_PORT}
}

generate_traffic() {
    send_tcp_traffic
    send_udp_traffic
}

echo "sending traffic to remote servers"
generate_traffic
echo "done"

#sudo tcpdump -i eth0|en0 -nn port 8080 or port 9090