from socket import *
ADDR = ('127.0.0.1', 21567)
udpCliSock = socket(AF_INET, SOCK_DGRAM)
data = input('请输入字符: ')
udpCliSock.sendto(data.encode('utf-8'), ADDR)
udpCliSock.close()