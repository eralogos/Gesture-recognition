#import necessary package
import socket
import time
import sys
import RPi.GPIO as GPIO

GPIO.setmode(GPIO.BCM)
 
DS = 6
SHCP = 19
STCP = 13
 
def init():
    GPIO.setup(DS, GPIO.OUT)
    GPIO.setup(SHCP, GPIO.OUT)
    GPIO.setup(STCP, GPIO.OUT)
    GPIO.setup([17,27,23,24],GPIO.OUT)
    GPIO.setup(22, GPIO.OUT)
    
    GPIO.output(DS, GPIO.LOW)
    GPIO.output(SHCP, GPIO.LOW)
    GPIO.output(STCP, GPIO.LOW)
    GPIO.output([17,27,23,24],[0,0,0,0])
 
def writeBit(data):
    GPIO.output(DS, data)
 
    GPIO.output(SHCP, GPIO.LOW)
    GPIO.output(SHCP, GPIO.HIGH)
 
#写入8位LED的状态
def writeByte(data):
    for i in range (0, 8):
        writeBit((data >> i) & 0x01)
    #状态刷新信号
    GPIO.output(STCP, GPIO.LOW)
    GPIO.output(STCP, GPIO.HIGH)
#init
init()
#define host ip: Rpi's IP
HOST_IP = "192.168.137.100"
HOST_PORT = 1234
print("Starting socket: TCP...")
socket_tcp = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print("TCP server listen @ %s:%d!" %(HOST_IP, HOST_PORT) )
host_addr = (HOST_IP, HOST_PORT)
#2.bind socket to addr:socket.bind(address)
socket_tcp.bind(host_addr)
#3.listen connection request:socket.listen(backlog)
socket_tcp.listen(1)
#4.waite for client:connection,address=socket.accept()
socket_con, (client_ip, client_port) = socket_tcp.accept()
print("Connection accepted from %s." %client_ip)
#5.handle
#GPIO.setmode(GPIO.BOARD)
#GPIO.setup(11,GPIO.OUT)
a = [0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80]
b=[[0,0,0,1],[0,0,1,1],[0,0,1,0],[0,1,1,0],[0,1,0,0],[1,1,0,0],[1,0,0,0],[1,0,0,1]]
c=[[0,0,0],[0,1,0],[0,0,1]]
#17hong 27lv  23 lan 1000 0100 0010
i=0
j=0
k=0
m=0
flag=1
print("Receiving package...")
while True:
    data=socket_con.recv(10)
    if len(data)>0:
        #liu shui deng
        if data[0] == "2":
            writeByte(a[i])
            if i<7:
               i+=1
            else:
               i=0
        elif data[0] == "3":
            writeByte(a[i])
            if i>0:
                i-=1
            else:
                i=7
        #bu jing dian ji
        elif data[0]=="1":
            if data[1]=="l" and flag!=1:
                flag=1
                print("left")
                j=0
                k=0
                while k<1:
                    GPIO.output([17,27,23,24],b[j])
                    if j<7:
                        j+=1
                    else:
                        j=0
                        k+=1
                    time.sleep(0.005)
                
            elif data[1]=="r" and flag!=2:
                flag=2
                print("right")
                j=7
                k=0
                while k<1:
                    GPIO.output([17,27,23,24],b[j])
                    if j>0:
                        j-=1
                    else:
                        j=7
                        k+=1
                    time.sleep(0.005)
                    
            elif data[2]=="d":
                print("down")
                if m>0:
                    #GPIO.output([17,27,23],c[m])
                    m-=1                    
            elif data[2]=="u":
                print("up")
                if m<3:
                    #GPIO.output([17,27,23],c[m])
                    m+=1
        elif data[0]=="4":
            if data[1]=="l":
                print("yigenshouzhi")
                GPIO.output(22,GPIO.HIGH)
            elif data[1]=="r":
                print("yigenshouzhi")
                GPIO.output(22,GPIO.LOW)
    
