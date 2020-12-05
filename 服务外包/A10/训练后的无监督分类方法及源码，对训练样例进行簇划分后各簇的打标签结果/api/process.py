import json
import os
import pandas as pd
import numpy as np
import pickle
import timeit
from sklearn.cluster import KMeans
from sklearn.decomposition import PCA
from sklearn.preprocessing import StandardScaler, MinMaxScaler
import logging
from logging import handlers


def now_time():
    return int(round(timeit.default_timer() * 1000))


def get_time_dict(pcs_start_t:int,pcs_end_t:int,prd_start_t:int,prd_end_t:int):
    time_dict=dict()
    time_dict['pcs_time']=pcs_end_t-pcs_start_t
    time_dict['prd_time']=prd_end_t-prd_start_t
    return time_dict


def json_to_datalist(js,df_weight:pd.DataFrame):
    try:
        #js=json.load(js)
        js=js['coptra']
        #print(len(js['rage']))
        df = pd.DataFrame(dict([(k, pd.Series(v)) for k, v in js.items()]))
        df=df.fillna(0)
        
        #main1
        df = df.drop(['xzbz', 'sbjgbh', 'cbztmc', 'dwbh', 'xzbzmc', 'cbrq'], axis=1)
        df.loc[df.cbzt == 3, 'cbzt'] = 0
        df['cbzt']=df['cbzt'].sum()
        #main2
        df = df.drop('record_date', axis=1)
        df['enforce_amount']=df['enforce_amount'].sum()
        
        #main3
        def is_nan(i):
            if i!=0:
                return 1
            else:
                return 0
        df['is_candate'] = df.apply(lambda row: is_nan(row['candate']), axis=1)
        df['is_revdate'] = df.apply(lambda row: is_nan(row['revdate']), axis=1)
        def encode_entstatus(i: str):
            if i == '在营（开业）企业':
                return 3
            elif i == '迁出':
                return 2
            elif i == '注销企业':
                return 1
            elif i == '吊销企业':
                return 0
        df['encode_entstatus'] = df.apply(lambda row: encode_entstatus(row['entstatus']), axis=1)
        df = df.drop(['regcap', 'estdate', 'candate', 'revdate', 'entstatus',
                            'opto', 'enttype', 'entcat', 'industryphy', 'regcapcur',
                            'industryco', 'opfrom', ], axis=1)
        
        #main4
        def get_rage(rage, typ: str):
            if str(typ) in str(rage):
                return 1
            return 0
        
        for i in range(1, 7):
            name = 'rage_{}'.format(i)
            df[name] = df.apply(lambda row: get_rage(row['rage'], i), axis=1)
            
        def encode_guaranperiod(idata):
            if idata == 1:
                return 1
            elif idata == 0:
                return 0
            
        df['encode_guaranperiod'] = df.apply(lambda row: encode_guaranperiod(row['guaranperiod']), axis=1)
        
        def encode_gatype(idata):
            if idata == 2:
                return 2
            elif idata == 1:
                return 1
            elif idata == 0:
                return 0
        
        df['encode_gatype'] = df.apply(lambda row: encode_gatype(row['gatype']), axis=1)
        
        def is_rage(idata: int):
            if int(idata) > 0:
                return 1
            elif int(idata) == 0:
                return 0
        
        df = df.eval("sum_rage = rage_1 + rage_2 + rage_3 + rage_4 + rage_5 + rage_6 ", inplace=False)
        df['is_rage'] = df.apply(lambda row: is_rage(row['sum_rage']), axis=1)
        df = df.eval("sub_pefperfromto = pefperto - pefperfrom", inplace=False)
        df = df.drop(
            ['priclaseckind', 'pefperfrom', 'pefperto', 'guaranperiod', 'gatype', 'iftopub', 'rage', 'rage_1', 'rage_2',
            'rage_3', 'rage_4', 'rage_5', 'rage_6', 'sum_rage'], axis=1)
        df['is_rage']=df['is_rage'].sum()/len(js['rage'])
        df['priclasecam']=df['priclasecam'].sum()/len(js['priclasecam'])
        df['encode_guaranperiod']=df['encode_guaranperiod'].sum()/len(js['guaranperiod'])
        df['encode_gatype']=df['encode_gatype'].sum()/len(js['gatype'])
        
        #main5
        df = df.eval(
            "unpaidsocialins = unpaidsocialins_so110 + unpaidsocialins_so210 + unpaidsocialins_so310 + unpaidsocialins_so410 + unpaidsocialins_so510",
            inplace=False)
        df = df.drop(['updatetime', 'unpaidsocialins_so110', 'unpaidsocialins_so210', 'unpaidsocialins_so310',
                        'unpaidsocialins_so410', 'unpaidsocialins_so510'], axis=1)
        
        #main6
        df = df.drop(['time','title','casetype','judgeresult','casecause','evidence','courtrank','datatype','latypes'], axis=1)
        #df['law_sum']=len(js['time'])
        newlen=len(js['time'])
        if newlen==1 and js['time']==[0]:
            df['law_sum']=0
        else:
            df['law_sum']=newlen

        #main7
        df = df.drop(['declaredate', 'declarestyle'], axis=1)
        df['appellant']=df['appellant'].sum()
        df['defendant']=df['defendant'].sum()
        
        #main8
        df = df.drop(['remark', 'dataflag', 'altitem', 'cxstatus', 'altdate', 'openo'], axis=1)
        df['alttime']=df['alttime'].sum()
        
        #main9
        df = df.drop(['invtype', 'conform', 'conprop', 'condate'], axis=1)
        df = df.rename(columns={'subconam': 'sum_subconam'})
        df['sum_subconam']=df['sum_subconam'].sum()
        
        #main10
        def get_evaluation(a, b, c, d):
            if (a >= b) and (c <= d):
                return 1
            else:
                return 0
        
        df = df.drop(['subconcurrency', 'subconform', 'anchetype', 'acconcurrency', 'acconform'], axis=1)
        df['evaluation'] = df.apply(lambda x: get_evaluation(x.liacconam, x.lisubconam, x.accondate, x.subcondate), axis=1)
        df = df.drop(['accondate', 'subcondate', 'liacconam', 'lisubconam'], axis=1)
        l_js=len(js['accondate'])
        df['evaluation']=(df['evaluation'].sum()-(df.shape[0]-l_js))/l_js
        
        df=df[0:1]
        #后续预处理
        data_list=df
        data_list.loc[data_list.credit_grade == 'C', 'credit_grade'] = 1
        data_list.loc[data_list.credit_grade == 'B-', 'credit_grade'] = 2
        data_list.loc[data_list.credit_grade == 'B', 'credit_grade'] = 3
        data_list.loc[data_list.credit_grade == 'A-', 'credit_grade'] = 4
        data_list.loc[data_list.credit_grade == 'A', 'credit_grade'] = 5
        data_list.loc[data_list.credit_grade == 'N', 'credit_grade'] = 6
        data_list.loc[data_list.credit_grade == 'N+', 'credit_grade'] = 7

        data_list = data_list.drop(['is_jnsn'], axis=1)

        data_list = data_list.eval("emplynum = qcwynum + zhycnum + zlzpnum", inplace=False)
        data_list = data_list.drop(['qcwynum', 'zhycnum', 'zlzpnum'], axis=1)

        data_list['enforce_amount'] = data_list['enforce_amount'].astype(np.float)

        entname = data_list['entname']
        # data_list1 = data_list1.drop('entname',axis=1)
        # 下面drop掉了entname
        for idx in df_weight.columns:
            if np.isnan(df_weight[idx][0]) or df_weight[idx][0] == 0:
                #print(df_weight[idx])
                df_weight.drop([idx], axis=1, inplace=True)
                data_list.drop([idx], axis=1, inplace=True)


        cols=['taxunpaidnum', 'level_rank', 'is_justice_creditaic',
       'is_except', 'is_brap', 'investnum', 'ipat_num', 'is_infoa',
       'is_justice_credit', 'ibrand_num', 'is_kcont', 'branchnum', 'idom_num',
       'shopnum', 'bidnum', 'is_punish', 'is_bra', 'pledgenum', 'is_infob',
       'icopy_num', 'passpercent', 'cbzt', 'enforce_amount', 'empnum',
       'encode_entstatus', 'priclasecam', 'encode_guaranperiod',
       'encode_gatype', 'is_rage', 'sub_pefperfromto', 'unpaidsocialins',
       'law_sum', 'defendant', 'alttime', 'evaluation', 'credit_grade']

        data_list=data_list.reindex(columns=cols)
        #print(data_list.columns)
        return entname,data_list #用来min-max雷达图
    except Exception as e:
        raise RuntimeError("Json Preprocess Failed: "+str(e))


def dfshow_minmax(entname,data_list:pd.DataFrame,mm:MinMaxScaler):
    try:
        cols=data_list.columns
        np_mm=mm.transform(data_list)
        df_mm=pd.DataFrame(np_mm,columns=cols)
        df_mm.insert(value=entname, loc=0, column='entname')
        return df_mm
    except Exception as e:
        raise RuntimeError("MinMaxScaler Apply Failed: "+str(e))

def df_to_dfp(entname,df_weight:pd.DataFrame,data_list:pd.DataFrame):
    try:
        dfp=pd.DataFrame()
        for idx in df_weight.columns:
            mn=df_weight[idx][0]
            if mn>=0:
                mn=1
            else:
                mn=-1
            dfp[idx]=data_list.apply(lambda row: row[idx]*mn,axis=1)

        dfp.insert(value=entname,loc=0,column='entname')
        return dfp
    except Exception as e:
        raise RuntimeError("Weight Assign Failed: "+str(e))

def dfp_to_dfp_dict(dfp:pd.DataFrame):
    try:
        df_bj = pd.DataFrame(dfp,columns=['entname','empnum','encode_entstatus','shopnum', 'branchnum', 'is_infoa', 'is_infob', 'level_rank'])
        df_wd = pd.DataFrame(dfp, columns=['entname','alttime'])
        df_jn = pd.DataFrame(dfp, columns=['entname','evaluation','investnum','bidnum','cbzt','ibrand_num','icopy_num','ipat_num','idom_num','passpercent'])
        df_jf = pd.DataFrame(dfp, columns=['entname','priclasecam','encode_guaranperiod','encode_gatype','is_rage','sub_pefperfromto',
                                                'unpaidsocialins','is_bra','is_brap','pledgenum','taxunpaidnum','is_except'])
        df_sf = pd.DataFrame(dfp, columns=['entname','law_sum','defendant', 'enforce_amount','is_justice_credit'])
        df_xf = pd.DataFrame(dfp, columns=['entname','is_punish','is_kcont','credit_grade','is_justice_creditaic'])
        dfp_dict = {'df_bj':df_bj,'df_wd':df_wd,'df_jn':df_jn,'df_jf':df_jf,'df_sf':df_sf,'df_xf':df_xf}
        return dfp_dict
        
    except Exception as e:
        raise RuntimeError("Dataset spilt Failed: "+str(e))

def do_kmeans_ss_pca(dfp_dict:dict,ss_dict:dict,pca_dict:dict):
    try:
        dfname_list= ['df_bj','df_wd','df_jn','df_jf','df_sf','df_xf']
        X_dict=dict()
        for i in dfname_list:
            dfp:pd.DataFrame=dfp_dict[i]
            entname =  dfp['entname']
            dfp = dfp.drop('entname',axis=1)
            np_dfp=np.array(dfp)

            ss:StandardScaler=ss_dict[i]
            ss_dfp=ss.transform(np_dfp)

            if i=='df_wd':
                X=ss_dfp
            else:
                pca:PCA=pca_dict[i]
                X=ss_pca_dfp=pca.transform(ss_dfp)

            X_dict[i]=X

        return X_dict
    except Exception as e:
        raise RuntimeError("SS/PCA Apply Failed: "+str(e))

def do_kmeans_predict(X_dict:dict,kmodel_dict:dict):
    try:
        dfname_list= ['df_bj','df_wd','df_jn','df_jf','df_sf','df_xf']
        results=dict()
        for i in dfname_list:
            X:np.array=X_dict[i]
            model:KMeans=kmodel_dict[i]
            results[i]=int(model.predict(X)[0])

        return results
    except Exception as e:
        raise RuntimeError("Model Predict Failed: "+str(e))

 
class Logger():
    level_relations = {
        "debug": logging.DEBUG,
        "info": logging.INFO,
        "warning": logging.WARNING,
        "error": logging.ERROR,
        "critical": logging.CRITICAL
    }
 
    def __init__(self, filename="log/api.log", level="info", when="D", interval=1, backupCount=10, fmt="%(asctime)s - %(pathname)s[line:%(lineno)d] - %"
                                                                                              "(levelname)s: %(message)s"):
        format_str = logging.Formatter(fmt)
        streamHandler = logging.StreamHandler()
        streamHandler.setFormatter(format_str)
        fileHandler = handlers.TimedRotatingFileHandler(filename=filename, when=when, backupCount=backupCount, encoding="utf-8")
        fileHandler.setFormatter(format_str)
        self.logger = logging.getLogger(filename)
        self.logger.setLevel(self.level_relations.get(level))
        self.logger.addHandler(streamHandler)
        self.logger.addHandler(fileHandler)






