from socket import *
ADDR = ('172.27.96.42', 21567)
tcpCliSock = socket(AF_INET, SOCK_STREAM)
tcpCliSock.connect(ADDR)
data = input('> ')
tcpCliSock.send(data.encode())
tcpCliSock.close()
