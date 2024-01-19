import numpy
import cv2
from mss import mss
from socket import *
import threading
import time
import math
import tkinter as tk
from tkinter import ttk
from PIL import Image, ImageTk
from flask import Flask, send_file
import io

class Gui:
    def __init__(self,root):
        self.root = root

        self.MAX_PACKET_SIZE=60000
        #UDP Multicast
        self.MCAST_GRP = '225.1.1.1'
        self.MCAST_PORT = 5007
        MULTICAST_TTL = 2
        self.multiSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)
        try:
            myIp = gethostbyname(gethostname())
            root.title(myIp+":1234")
            self.multiSock.setsockopt(IPPROTO_IP, IP_MULTICAST_IF, inet_aton(myIp))
        except:
            print("impossibile verificare ip")
            self.multiSock.setsockopt(IPPROTO_IP, IP_MULTICAST_TTL, MULTICAST_TTL)
        self.sct = mss()

        img = numpy.array(self.sct.grab(self.sct.monitors[0]))
        self.imgW=img.shape[1]
        self.imgH=img.shape[0]
        self.x=0
        self.y=0
        self.w=int(self.imgW/2)
        self.h=int(self.imgH)
        self.q=70
        self.fx=0.3
        self.t=1.0/30
        self._imgR = numpy.array([])

        py=10
        self.xl=tk.Label(root,text="X:")
        self.xl.place(x=10,y=py+10)
        self.xs = tk.Scale(self.root, from_=0, to=100, orient=tk.HORIZONTAL,command=self.updated)
        self.xs.place(x=40,y=py)
        py+=40
        self.yl=tk.Label(root,text="Y:")
        self.yl.place(x=10,y=py+10)
        self.ys = tk.Scale(self.root, from_=0, to=100, orient=tk.HORIZONTAL,command=self.updated)
        self.ys.place(x=40,y=py)
        py+=40
        self.wl=tk.Label(root,text="W:")
        self.wl.place(x=10,y=py+10)
        self.ws = tk.Scale(self.root, from_=0, to=100, orient=tk.HORIZONTAL,command=self.updated)
        self.ws.set(50)
        self.ws.place(x=40,y=py)
        py+=40
        self.hl=tk.Label(root,text="H:")
        self.hl.place(x=10,y=py+10)
        self.hs = tk.Scale(self.root, from_=0, to=100, orient=tk.HORIZONTAL,command=self.updated)
        self.hs.set( 100 )
        self.hs.place(x=40,y=py)
        py+=40
        self.ql=tk.Label(root,text="Q:")
        self.ql.place(x=10,y=py+10)
        self.qs = tk.Scale(self.root, from_=0, to=100, orient=tk.HORIZONTAL,command=self.updated)
        self.qs.place(x=40,y=py)
        self.qs.set(70)
        py+=40
        self.fxl=tk.Label(root,text="S:")
        self.fxl.place(x=10,y=py+10)
        self.fxs = tk.Scale(self.root, from_=0, to=100, orient=tk.HORIZONTAL,command=self.updated)
        self.fxs.set( int(100*self.fx) )
        self.fxs.place(x=40,y=py)        
        py+=40

        self.streamButton = ttk.Button(self.root, text="Start Stream", command=self.toogleStream, width = 11)
        self.streamButton.place(x=10,y=py)
        py+=40        

        # Put it in the display window
        self.imgL = tk.Label(self.root)
        self.imgL.place(x=10,y=py)
        self.imgData = None
        self.updatePreview()
        py+=180

        self.streaming=False

        root.geometry("160x"+str(py+20))

    def updatePreview(self):
        img = numpy.array(self.sct.grab(self.sct.monitors[0]))
        imgR = img[self.y:self.h, self.x:self.w]
        self._imgR = imgR
        fx = float(140)/float(self.w-self.x)
        if fx>0.0:
            imgR = cv2.resize(imgR,None,fx=fx ,fy=fx)
            self.imgtk = ImageTk.PhotoImage(image=Image.fromarray(imgR)) 
            self.imgL.config(image=self.imgtk)
        self.imgL.after(100,self.updatePreview)


    def toogleStream(self):
        self.streaming=not self.streaming
        if self.streaming:
            self.streamButton.config(text="Stop stream")
        else:
            self.streamButton.config(text="Start stream")
        if self.streaming:
            threading.Thread(target=self.sendThread).start()        


    def updated(self,event):
        self.x = int(self.xs.get()*self.imgW/100)
        self.y = int(self.ys.get()*self.imgH/100)
        self.w = int(self.ws.get()*self.imgW/100)
        self.h = int(self.hs.get()*self.imgH/100)
        #print(self.x,self.y,self.w,self.h)
        if(self.w<1):
            self.w=1
            self.ws.set(1)
        if(self.h<1):
            self.h=1
            self.hs.set(1)
        if self.x>=self.w:
            self.x=self.w-1
            self.xs.set( self.ws.get()-1 )
        if self.y>=self.h:
            self.y=self.h-1
            self.ys.set( self.hs.get()-1 )
        self.q = self.qs.get()
        self.fx = self.fxs.get()/100

    def sendThread(self):
        i=0
        while self.streaming:
            #img = numpy.array(self.sct.grab(self.sct.monitors[0]))
            #imgR = img[self.y:self.h, self.x:self.w]
            if len(self._imgR)>0:
                imgR = self._imgR
                imgR = cv2.resize(imgR,None,fx=self.fx,fy=self.fx)
                encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), self.q]
                result, imgData = cv2.imencode('.jpg', imgR, encode_param)
                self.imgData = imgData.tobytes()

                dataSize = len(self.imgData)
                nPackets = math.ceil(dataSize/(self.MAX_PACKET_SIZE))
                sentBytes=0
                for j in range(nPackets):
                    packet = bytearray()
                    packet.extend( int(i).to_bytes(4, byteorder='little') )
                    packet.extend( int(j).to_bytes(4, byteorder='little') )
                    packet.extend( int(nPackets).to_bytes(4, byteorder='little') )
                    packetSize = dataSize-sentBytes
                    if packetSize>self.MAX_PACKET_SIZE-16:
                        packetSize=self.MAX_PACKET_SIZE-16
                    packet.extend( int(packetSize).to_bytes(4, byteorder='little') )
                    packet.extend( imgData[sentBytes:sentBytes+packetSize] )
                    self.multiSock.sendto(packet, (self.MCAST_GRP, self.MCAST_PORT))
                    sentBytes+=packetSize
                print("sentBytes",sentBytes,"dataSize",dataSize)
                i+=1
            time.sleep(self.t)

gui = None
def main():
    global gui
    root=tk.Tk("Screen Streamer")
    gui = Gui(root)
    tk.mainloop()

threading.Thread(target=main).start()

app = Flask(__name__)
@app.route('/')
def index():
    return """
<html>
<head>
<title>Screen Streamer</title>
</head>
<body>
<h1>Screen Streamer</h1>
<img src="/logo.jpeg" id="logo" />
<script>
const update = function() {
    const logo = document.getElementById('logo');
    logo.src = '/logo.jpeg?'+Date.now();
    setTimeout(update, 100);
};
update();
</script>
</body>
</html>
"""

@app.route('/logo.jpeg', methods=['GET'])
def logo():
    global gui
    if gui!=None and gui.streaming and gui.imgData!=None and len(gui.imgData)>0:
        return send_file(io.BytesIO(gui.imgData), mimetype='image/jpeg')
    else:
        return ""

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=1234)