import matplotlib.pyplot as plt
import numpy as np

class change:

    def move(self,xx,yy,x,y):
        showx=[]
        showy=[]
        vis = [[0.] * 3] * 3
        vis = np.array(vis)
        vis[2][0] = x
        vis[2][1] = y
        for i in range(3):
            for j in range(3):
                if i == j:
                    vis[i][j] = 1
        for t in range(len(xx)-1):
            ii=xx[t]
            jj=yy[t]
            p = np.array([ii, jj, 1])
            p=np.dot(p,vis)
            showx.append(p[0])
            showy.append(p[1])
        showx.append(showx[0])
        showy.append(showy[0])
        plt.plot(showx, showy, color='r', markerfacecolor='blue', marker='o')
        for a, b in zip(showx, showy):
            plt.text(a, b, (a, b), ha='center', va='bottom', fontsize=10)
        plt.ion()
        plt.show()
        plt.pause(4)
        plt.close()

    def scale(self,xx,yy,x,y):
        showx = []
        showy = []
        vis = [[0.] * 3] * 3
        vis = np.array(vis)
        vis[0][0] = x
        vis[1][1] = y
        vis[2][2] = 1
        for t in range(len(xx) - 1):
            ii = xx[t]
            jj = yy[t]
            p = np.array([ii, jj, 1])
            p = np.dot(p, vis)
            showx.append(p[0])
            showy.append(p[1])
        showx.append(showx[0])
        showy.append(showy[0])
        plt.plot(showx, showy, color='r', markerfacecolor='blue', marker='o')
        for a, b in zip(showx, showy):
            plt.text(a, b, (a, b), ha='center', va='bottom', fontsize=10)
        plt.ion()
        plt.show()
        plt.pause(4)
        plt.close()


    def rotate(self,xx,yy,x):
        showx = []
        showy = []
        vis = [[0.] * 3] * 3
        vis = np.array(vis)
        vis[0][0] = np.cos(x)
        vis[0][1] = np.sin(x)
        vis[1][0] = np.sin(-x)
        vis[1][1] = np.cos(x)
        vis[2][2] = 1
        for t in range(len(xx) - 1):
            ii = xx[t]
            jj = yy[t]
            p = np.array([ii, jj, 1])
            p = np.dot(p, vis)
            showx.append(p[0])
            showy.append(p[1])
        showx.append(showx[0])
        showy.append(showy[0])
        plt.plot(showx, showy, color='r', markerfacecolor='blue', marker='o')
        for a, b in zip(showx, showy):
            plt.text(a, b, (a, b), ha='center', va='bottom', fontsize=10)
        plt.ion()
        plt.show()
        plt.pause(4)
        plt.close()


if __name__ == "__main__":
    xx = [0,2,1,0]
    yy = [0,0,1,0]
    print("原图形：")
    for i in range(len(xx)-1):
        print('(',xx[i],',',yy[i],')')
    plt.plot(xx, yy, color='r', markerfacecolor='blue', marker='o')
    for a, b in zip(xx, yy):
        plt.text(a, b, (a, b), ha='center', va='bottom', fontsize=10)
    plt.ion()
    plt.show()
    plt.pause(4)
    plt.close()
    x=float(input("x轴平移："))
    y=float(input("y轴平移："))
    change.move(1,xx,yy,x,y)
    print(xx,yy)
    x = float(input("x轴缩放："))
    y = float(input("y轴缩放："))
    change.scale(1,xx,yy,x,y)
    x = float(input("旋转角度（以弧度为单位）："))
    change.rotate(1,xx,yy,x)
