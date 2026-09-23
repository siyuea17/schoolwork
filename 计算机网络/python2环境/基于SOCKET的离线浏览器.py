from socket import *

HOST = '172.27.94.102'
PORT = 8080
ADDR = (HOST, PORT)

data = 'GET / HTTP/1.1\r\nAccept-Encoding: identity\r\nHost: 172.27.94.102:8080\r\nConnection: close\r\nUser-Agent: Python-urllib/2.7\r\n\r\n'

tcpCliSock = socket(AF_INET, SOCK_STREAM)
tcpCliSock.connect(ADDR)
tcpCliSock.sendall(data)

response = ''
while True:
    datanew = tcpCliSock.recv(1024)
    if not datanew:
        break
    response += datanew

print response
tcpCliSock.close()