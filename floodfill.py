import numpy as np
import cv2
import matplotlib.pyplot as plt
import time

def dfs(x,y,vis,dic,im):
    b = np.array([[[x, y], [x, y], [x, y], [x, y]]], dtype=np.int32)
    cv2.fillPoly(im, b, 255)
    plt.imshow(im)
    plt.show()
    plt.pause(0.2)
    i=0
    while(i<4):
        xx=x+dic[i][0]
        yy=y+dic[i][1]
        if vis[xx][yy]==1:
            i+=0
        else:
            vis[xx][yy]=1
            dfs(xx,yy,vis,dic,im)
        i+=1
if __name__ == "__main__":
    a = np.array([[[0,0], [0,1], [0,2], [0,3],[0,4],[0,5],[1,5],[2,5],[2,6],
                   [3,6],[3,7],[3,8],[3,9],[4,9],[5,9],[6,9],[7,9],[8,9],
                   [8,8],[8,7],[8,6],[9,6],[10,6],[10,5],[10,4],[9,4],[8,4],
                   [7,4],[6,4],[5,4],[5,3],[5,2],[5,1],[5,0],[4,0],[3,0],[2,0],[1,0],[0,0]]], dtype = np.int32)
    vis=[[0]*20]*20
    vis=np.array(vis)
    for i in a[0]:
        vis[i[0]][i[1]]=1
   # print(vis[0][6])
    dic=[[0,1],[0,-1],[1,0],[-1,0]]#四联通
    #dic=[[0,1],[0,-1],[1,0],[-1,0],[-1,-1],[-1,1],[1,1],[1,-1]]#八联通
    im = np.zeros([20, 20], dtype = np.uint8)
    cv2.polylines(im, a, 1, 255)
    plt.ion()
    plt.imshow(im)
    plt.show()
    plt.pause(1)
    dfs(1,1,vis,dic,im)
    plt.ioff()

