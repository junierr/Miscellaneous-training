import matplotlib.pyplot as plt
import numpy as np
import math

def getsum(xx,t,nows,nowi):
    if nows==0:
        return xx[nowi]
    else:
        return (1-t)*getsum(xx,t,nows-1,nowi)+t*getsum(xx,t,nows-1,nowi+1)

def getpoint(xx,t):
    s=len(xx)-1
    x=getsum(xx,t,s,0)
    return x

def bezier(xx,yy,step):
    t=0
    showx=[]
    showy=[]
    while t<=1:
        x=getpoint(xx,t)
        y=getpoint(yy,t)
        showx.append(x)
        showy.append(y)
        plt.plot(showx, showy, color='r', markerfacecolor='blue', marker='o')
        plt.show()
        plt.pause(0.00001)
        t+=step


x=[1,3,6,8]
y=[1,3,3,1]
plt.plot(x, y,  color='r',markerfacecolor='blue',marker='o')
plt.ion()
plt.show()
plt.pause(1)
bezier(x,y,0.01)
plt.pause(3)
plt.ioff()