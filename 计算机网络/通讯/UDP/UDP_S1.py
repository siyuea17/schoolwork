from socket import * 
ADDR = ('' , 21567) 
udpSerSock = socket(AF_INET,SOCK_DGRAM) 
udpSerSock.bind(ADDR)
data, addr = udpSerSock.recvfrom(65537)
print (data)
udpSerSock.close()