import numpy
import cv2
from socket import *
import struct


#UDP Multicast
MCAST_GRP = '225.1.1.1'
MCAST_PORT = 5007
MULTICAST_TTL = 2
multiSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)
#multiSock.setsockopt(IPPROTO_IP, IP_MULTICAST_TTL, MULTICAST_TTL)
# Bind to the server address
multiSock.bind( ('',MCAST_PORT) )

group = inet_aton(MCAST_GRP)
mreq = struct.pack('4sL', group, INADDR_ANY)
multiSock.setsockopt(IPPROTO_IP, IP_ADD_MEMBERSHIP, mreq)

#cv2.imshow("Remote Screen",img)

N=60000


cv2.namedWindow("Remote Screen",cv2.WINDOW_NORMAL)

img = None
while True:
    packetData, address = multiSock.recvfrom(N)
    w = int.from_bytes(packetData[0:4], byteorder='little')
    h = int.from_bytes(packetData[4:8], byteorder='little')
    x = int.from_bytes(packetData[8:12], byteorder='little')
    y = int.from_bytes(packetData[12:16], byteorder='little')
    ps = int.from_bytes(packetData[16:20], byteorder='little')
    packetData = packetData[20:]
    if img is None or w!=img.shape[1] or h!=img.shape[0]:
        img = numpy.zeros((h,w,3),dtype=numpy.uint8)
    imgData = numpy.frombuffer(packetData, dtype=numpy.uint8)
    #print(len(imgData))
    imgPart = cv2.imdecode(imgData, 1)
    xs=int(w/4)
    ys=int(h/4)
    #print(w,h,x,y,ps,xs,ys)
    img[y*ys:y*ys+ys,x*xs:x*xs+xs]=imgPart
    cv2.imshow("Remote Screen",img)
    cv2.waitKey(1)

"""
imgDataBuffer=bytearray()
while True:
    packetData, address = multiSock.recvfrom(N)
    i = int.from_bytes(packetData[0:4], byteorder='little')
    j = int.from_bytes(packetData[4:8], byteorder='little')
    np = int.from_bytes(packetData[8:12], byteorder='little')
    ps = int.from_bytes(packetData[12:16], byteorder='little')
    print(i,j,np,ps)
    packetData = packetData[16:]
    if j==0 or i!=old_i:
        old_i=i
        data=bytearray()
        data.extend(packetData)
        n=1
    else:
        data.extend(packetData)
        n+=1
    if j==np-1 and n==np:
        print("Packet received",len(data))
        try:
            imgData = numpy.frombuffer(data, dtype=numpy.uint8)
            img = cv2.imdecode(imgData, 1)
            print("Image decoded",img.shape)
            cv2.imshow("Remote Screen",img)
            cv2.waitKey(1)
        except:
            pass
"""