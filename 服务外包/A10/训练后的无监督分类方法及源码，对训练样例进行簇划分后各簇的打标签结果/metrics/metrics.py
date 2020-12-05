# coding: utf-8

from sklearn import datasets
from sklearn import preprocessing
from sklearn.metrics import silhouette_score,calinski_harabasz_score,davies_bouldin_score
import pandas as pd
import numpy as np
import sys
import math


def Model_evaluation_CP(label,x):
    key=list(set(label))
    #print(key)
    dis_mean=[]
    pos_mean=[]
    knc=len(key)
    for i in range(knc):
        s=x[np.where(label==key[i])]
        pos_mean.append(np.mean((s),axis=0))
        n=len(s)
        sum_result=0
        for j in range(n):
            temp=s[j]-pos_mean[i]
            temp1=math.sqrt(np.dot(temp,temp))
            sum_result+=float(temp1)/n
        dis_mean.append(sum_result)
    result=sum(dis_mean)/knc
    return result
#Model_evaluation_CP(all_predictions,iris.data)



def Model_evaluation_DBI(label,x):
    key=list(set(label))
    dis_mean=[]
    pos_mean=[]
    max_dis=[]
    knc=len(key)
    for i in range(knc):
        s=x[np.where(label==key[i])]
        pos_mean.append(np.mean((s),axis=0))
        n=len(s)
        sum_result=0
        for j in range(n):
            temp=s[j]-pos_mean[i]
            temp1=math.sqrt(np.dot(temp,temp))
            sum_result+=float(temp1)/n
        dis_mean.append(sum_result)
    for i in range(knc):
        x=[]
        for j in range(knc):
            if i!=j:
                temp=pos_mean[i]-pos_mean[j]
                temp1=math.sqrt(np.dot(temp,temp))
                x.append((dis_mean[i]+dis_mean[j])/temp1)
        max_dis.append(x[np.argmax(np.array(x),axis=0)])
        result=sum(max_dis)/knc
    return result
#Model_evaluation_DBI(all_predictions,iris.data)



def Model_evaluation_SP(label,x):
    key=list(set(label))
    pos_mean=[]
    knc=len(key)
    result=0
    z=knc*(knc-1)
    for i in range(knc):
        s=x[np.where(label==key[i])]
        pos_mean.append(np.mean((s),axis=0))
    for i in range(knc):
        for j in range(i+1,knc):
            temp=pos_mean[i]-pos_mean[j]
            #temp1=math.sqrt(np.dot(temp,temp))
            temp1=np.linalg.norm(temp,ord=2)
            result=result+(temp1*2)/z
    return result
#Model_evaluation_SP(all_predictions,iris.data)