from flask import Flask, jsonify, request
from flask_cors import cross_origin
import json
import os
import sys
import pandas as pd
import numpy as np
import pickle
import timeit
from sklearn.cluster import KMeans
from sklearn.decomposition import PCA
from sklearn.preprocessing import StandardScaler, MinMaxScaler
from process import *

app = Flask(__name__)
saved='save/'

# init load
logger=Logger(level='debug').logger
logger.debug("Performing initial load...")
try:
    df_weight=pd.read_csv(f'{saved}df/df_weight.csv')

    mm=pickle.load(open(f'{saved}mm/mm.pickle','rb'))

    ss_dict=dict()
    pca_dict=dict()
    kmodel_dict=dict()

    dfname_list= ['df_bj','df_wd','df_jn','df_jf','df_sf','df_xf']
    for i in dfname_list:
        ss_dict[i]=pickle.load(open(f'{saved}ss/kmeans_ss_final4_{i}.csv.pickle','rb'))
        kmodel_dict[i]=pickle.load(open(f'{saved}model/kmeans_final4_{i}.csv.pickle','rb'))
        if i!='df_wd':
            pca_dict[i]=pickle.load(open(f'{saved}pca/kmeans_pca_final4_{i}.csv.pickle','rb'))

except Exception as e:
    logger.error("Model load failed: " + str(e))
    logger.error("API will now exit." + str(e))
    sys.exit(1)
else:
    logger.debug('Model loaded!')


@app.route("/")
def hello():
    return "Model API is running!"

@app.route("/predict",methods=['POST', 'OPTIONS'])
@cross_origin()
def predict():
    resp={'flag':0,'data':None,'errorMsg':None}
    data=dict()
    try:
        js = request.get_json()
        pcs_start_t = now_time()
        entname,data_list=json_to_datalist(js,df_weight)
        df_mm=dfshow_minmax(entname,data_list,mm)
        dfp=df_to_dfp(entname,df_weight,data_list)
        dfp_dict=dfp_to_dfp_dict(dfp)
        X_dict=do_kmeans_ss_pca(dfp_dict,ss_dict,pca_dict)
        prd_start_t = pcs_end_t = now_time()
        prd_dict=do_kmeans_predict(X_dict,kmodel_dict)
        prd_end_t = now_time()
        time_dict = get_time_dict(pcs_start_t,pcs_end_t,prd_start_t,prd_end_t)
        fmt_dict:dict=(data_list.to_dict(orient = 'records'))[0]
        mms_dict:dict=(df_mm.to_dict(orient = 'records'))[0]
        data['entname']=mms_dict['entname']
        mms_dict.pop('entname')
        data['fmt_data']=fmt_dict
        data['mms_data']=mms_dict
        data['model_pred']=prd_dict
        data['time_stat']=time_dict
        resp['data']=data

    except Exception as e:
        resp['flag']=1
        resp['errorMsg']=str(e)
        logger.error(str(e))
    
    #print(resp)
    return jsonify(resp)
        

if __name__ == '__main__':
    try:
        port = int(sys.argv[0])
    except:
        port = 12000
    app.run(host='0.0.0.0', port=port, debug=False)


