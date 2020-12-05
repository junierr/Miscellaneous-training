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
from models import *
from metrics import *

r_id = 'kmeans0'
np.set_printoptions(threshold=sys.maxsize)


dspath= 'data//'



def show(X,pname):
    plt.scatter(X[:, 0], X[:, 1])  
    plt.xlabel('x')  
    plt.ylabel('y') 
    #plt.legend(loc=2)  
    #plt.show()
    plt.savefig(f'fig/fig_{pname}_1.png',dpi=1200)


if __name__ == '__main__':
    path = dspath+ 'split_dataset//'
    files = os.listdir(path)
    files_csv = list(filter(lambda x: x[-4:]=='.csv' , files))
    for file in  files_csv:
        st_time=datetime.datetime.now()
        print(f'Model Running on {file}:')
        log_rs = open(f'logs/rs_{r_id}_{file}.log', 'w')
        log_score=open(f'scores/{r_id}_{file}.log', 'w')
        saver_model = open(f'model/kmeans_{file}.pickle', 'wb')
        saver_ap = open(f'ap/kmeans_ap_{file}.pickle', 'wb')
        saver_ss=open(f'ss/kmeans_ss_{file}.pickle', 'wb')
        file_path=path+file
        df=pd.read_csv(file_path,encoding='gbk',low_memory=False,index_col=0)
        entname =  df['entname']
        df = df.drop('entname',axis=1)
        df=np.array(df)
        ss = StandardScaler()
        df = ss.fit_transform(df)
        pickle.dump(ss,saver_ss)
        saver_ss.close()

        if file =='final4_df_jn.csv':
            n_c=5
        elif file =='final4_df_xf.csv':
            n_c=6
        else:
            n_c=4
        print(f'k={n_c}')

        if file =='final4_df_wd.csv':
            X = df
        else:
            saver_pca=open(f'pca/kmeans_pca_{file}.pickle', 'wb')

            if file =='final4_df_xf.csv':
                n=4
            else:
                n=2
            print(f'n={n}')

            pca = PCA(n_components=n,random_state=0)
            X= np_pca =pca.fit_transform(df)
            print(pca.explained_variance_ratio_)
            pickle.dump(pca,saver_pca)
            saver_pca.close()
        #print(df_pca)
        if file !='final4_df_wd.csv':
            #show(X,r_id+file)
            all_predictions=kmeans(n_c,0,X,X,r_id+file,logger=log_rs,saver_model=saver_model,saver_ap=saver_ap)
        else:
            all_predictions=kmeans(n_c,0,X,X,r_id+file,logger=log_rs,saver_model=saver_model,saver_ap=saver_ap)
        tr_time=datetime.datetime.now()
        print(f'Model Train Time: {tr_time-st_time}')
        print(f'Model Train Time: {tr_time-st_time}',file=log_rs)
        print()
        cp=Model_evaluation_CP(all_predictions,X)
    
        print(f'CP={cp}')
        print(f'CP={cp}',file=log_score)
        dbi=davies_bouldin_score(X,all_predictions)
        print(f'DBI={dbi}')
        print(f'DBI={dbi}',file=log_score)
        sp=Model_evaluation_SP(all_predictions,X)
        print(f'SP={sp}')
        print(f'SP={sp}',file=log_score)
        ss=silhouette_score(X,all_predictions)
        print(f'SS={ss}')
        print(f'SS={ss}',file=log_score)
        chs=calinski_harabasz_score(X,all_predictions)
        print(f'CHS={chs}')
        print(f'CHS={chs}',file=log_score)
        print()

        saver_model.close()
        saver_ap.close()
        log_rs.close()
        log_score.close()







