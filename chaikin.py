import matplotlib.pyplot as plt
import numpy as np
import math


def chaikin(xx,yy,num):
    while num:
        plt.clf()
        showx = []
        showy = []
        prex = xx
        prey = yy
        len1=len(xx)-1
        for i in range(len1):
            plt.plot(prex, prey, color='r', markerfacecolor='blue', marker='o')
            plt.show()
            plt.pause(0.1)
            x1 = (xx[i+1] + 2*xx[i]) / 3
            y1 = (yy[i+1] + 2*yy[i]) / 3
            x2 = (2*xx[i+1]+xx[i]) / 3
            y2 = (2*yy[i+1]+yy[i]) / 3
            showx.append(x1)
            showy.append(y1)
            plt.plot(showx, showy, color='r', markerfacecolor='blue', marker='o')
            plt.show()
            plt.pause(0.05)
            showx.append(x2)
            showy.append(y2)
            plt.plot(showx, showy, color='r', markerfacecolor='blue', marker='o')
            plt.show()
            plt.pause(0.05)
        plt.pause(0.5)
        prex=xx
        prey=yy
        xx=showx
        yy=showy
        num-=1
    plt.clf()
    plt.plot(showx, showy, color='r', markerfacecolor='blue', marker='o')
    plt.show()
    plt.pause(2)


x=[1,3,6,8]
y=[1,3,3,1]
plt.ion()
chaikin(x,y,5)
plt.ioff()