# coding: utf-8

from sklearn import datasets
import matplotlib.pyplot as plt
from sklearn.cluster import KMeans
from sklearn import preprocessing
from sklearn.decomposition import PCA
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import train_test_split
from sklearn import metrics
from sklearn.metrics import silhouette_score,calinski_harabasz_score,davies_bouldin_score
import pandas as pd
import numpy as np
import os
import sys
import pickle
import math
import time
import datetime

r_id = 'k_silhouette'
np.set_printoptions(threshold=sys.maxsize)


dspath= 'data//'


def k_silhouette(X, clusters,df_name):
    K = range(2, clusters + 1)
    # 构建空列表，用于存储个中簇数下的轮廓系数
    S = []
    for k in K:
        print(f'k={k}')
        kmeans = KMeans(n_clusters=k)
        kmeans.fit(X)
        labels = kmeans.labels_

        # 调用字模块metrics中的silhouette_score函数，计算轮廓系数
        S.append(metrics.silhouette_score(X, labels, metric='euclidean'))

    # 中文和负号的正常显示
    plt.clf()
    #plt.rcParams['font.sans-serif'] = [u'SimHei']
    plt.rcParams['axes.unicode_minus'] = False
    # 设置绘图风格
    plt.style.use('ggplot')
    # 绘制K的个数与轮廓系数的关系
    plt.plot(K, S, 'b*-')
    plt.xlabel('Number of Clusters')
    plt.ylabel('Silhouette Score')
    # 显示图形
    plt.savefig(f'fig0/fig_{df_name}.png',dpi=1200) 
    


if __name__ == '__main__':
    path = dspath+ 'split_dataset//'
    files = os.listdir(path)
    files_csv = list(filter(lambda x: x[-4:]=='.csv' , files))
    for file in  files_csv:
        print(f'f={file}')
        file_path=path+file
        df=pd.read_csv(file_path,encoding='gbk',low_memory=False,index_col=0)
        entname =  df['entname']
        df = df.drop('entname',axis=1)
        df=np.array(df)
        ss = StandardScaler()
        df = ss.fit_transform(df)
    #    print('\ndf=\n',file=log_rs)
    #    print(df,file=log_rs)

        if file =='final4_df_jn.csv':
            n_c=5
        elif file =='final4_df_xf.csv':
            n_c=6
        else:
            n_c=4

        if file =='final4_df_wd.csv':
            X = df
        elif file =='final4_df_xf.csv':
            pca = PCA(n_components=4,random_state=0)
            X= np_pca =pca.fit_transform(df)
        else:
            pca = PCA(n_components=2,random_state=0)
            X= np_pca =pca.fit_transform(df)
        #print(df_pca)
        if file !='final4_df_wd.csv':
            k_silhouette(X, 10,file)
            #all_predictions=kmeans(n_c,1,X,X,r_id+file,logger=log_rs,saver_model=saver_model,saver_ap=saver_ap)
        else:
            k_silhouette(X, 10,file)
            #all_predictions=kmeans(n_c,0,X,X,r_id+file,logger=log_rs,saver_model=saver_model,saver_ap=saver_ap)







