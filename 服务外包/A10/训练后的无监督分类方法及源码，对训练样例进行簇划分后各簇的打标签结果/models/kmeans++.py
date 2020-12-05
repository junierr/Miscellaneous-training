# coding: utf-8

from sklearn import datasets
import matplotlib.pyplot as plt
from sklearn.cluster import KMeans
from sklearn import preprocessing
from sklearn.decomposition import PCA
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import train_test_split
from sklearn.metrics import silhouette_score,calinski_harabasz_score,davies_bouldin_score
import pandas as pd
import numpy as np
import os
import sys
import pickle
import math
import time
import datetime


def kmeans(n_c,flag,np_s,X,pname,logger,saver_model,saver_ap):
    model = KMeans(n_clusters=n_c,random_state=0,n_init=20)
    model.fit(np_s) 
    all_predictions = model.predict(np_s)
    pickle.dump(all_predictions,saver_ap)
    #print(all_predictions)
    print(all_predictions,file=logger)
    print(" ",file=logger)
    print(" ",file=logger)

    dis = {}
    for i in range(len(all_predictions)):
        if not dis.get(all_predictions[i]):
            dis.setdefault(all_predictions[i], [i])
        else:
            dis[all_predictions[i]].append(i)
    # print(dis)
    for i in dis:
        print(i,file=logger)
        print(dis[i],file=logger)
        print(" ",file=logger)
        print(" ",file=logger)

    if flag==1:
        plt.scatter(X[:, 0], X[:, 1], c=all_predictions)
    #plt.show()
        plt.savefig(f'fig/fig_{pname}_2.png',dpi=1200)
    pickle.dump(model,saver_model)
    return all_predictions






