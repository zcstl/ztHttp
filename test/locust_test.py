#!/usr/bin/env python

from socket import *

HOST='localhost'
PORT=8080
BUFSIZE=1024
ADDR=(HOST, PORT)

c_sock=socket(AF_INET, SOCK_STREAM)
c_sock.connect(ADDR)

data=c_sock.recv(BUFSIZE)

print data

c_sock.close()
