#!/bin/bash

TCP_PORT=8080
UDP_PORT=9090

start_tcp_server() {
    echo "starting tcp server on port ${TCP_PORT}..."
    while true; do
        echo "listening for tcp connections..."
        nc -l -p ${TCP_PORT} -q 1 | while read line; do
            echo "received: $line"
            echo "response from tcp server" | nc -q 1 localhost ${TCP_PORT}
        done
    done &
    TCP_SERVER_PID=$!
}

start_udp_server() {
    echo "starting udp server on port ${UDP_PORT}..."
    while true; do
        echo "listening for udp messages..."
        nc -u -l -p ${UDP_PORT} | while read line; do
            echo "received udp: $line"
            echo "response from udp server" | nc -u -q 1 localhost ${UDP_PORT}
        done
    done &
    UDP_SERVER_PID=$!
}

stop() {
    echo "stopping servers"
    [ -n "${TCP_SERVER_PID}" ] && kill ${TCP_SERVER_PID} 2>/dev/null
    [ -n "${UDP_SERVER_PID}" ] && kill ${UDP_SERVER_PID} 2>/dev/null
}

trap stop EXIT

start_tcp_server
start_udp_server
echo "Servers have started"
wait