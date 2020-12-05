#!/usr/bin/env python
# coding: utf-8

# In[1]:


import re
import os
import pandas as pd
import numpy as np
from sklearn import preprocessing
from sklearn.decomposition import PCA
from sklearn.preprocessing import StandardScaler,MinMaxScaler
from sklearn.model_selection import train_test_split
import time
import datetime
import pickle
st_time=datetime.datetime.now()


# In[2]:


# 替换此处即可
dspath = '..//data//'


# In[3]:


def main():
    path = dspath + 'data1//'
    files = os.listdir(path)
    files_csv = list(filter(lambda x: x[-4:] == '.csv', files))
    data_list = pd.DataFrame()
    i = 0
    for file in files_csv:
        file_path = path + file
        df = pd.read_csv(file_path, encoding='gbk', low_memory=False)
        if i == 0:
            data_list = data_list.append(df)
        else:
            data_list = pd.merge(data_list, df, on='entname', how='outer')
        # data_list=data_list.append(df,ignore_index=True)
        i = i + 1
    # data_list=data_list.fillna(value=0)
    data_list = data_list.fillna(0)
    return data_list
    # print(data_list)


# In[4]:


def main1():
    file_path = dspath + 'enterprise_insurance//enterprise_insurance.csv'
    df = pd.read_csv(file_path, encoding='gbk', low_memory=False)
    df = df.drop(['xzbz', 'sbjgbh', 'cbztmc', 'dwbh', 'xzbzmc', 'cbrq'], axis=1)
    df = df.fillna(0)
    df.loc[df.cbzt == 3, 'cbzt'] = 0
    df = df.groupby(['entname']).sum().reset_index()
    return df


# In[5]:


def main2():
    path = dspath + 'justice_enforced//'
    files = os.listdir(path)
    files_csv = list(filter(lambda x: x[-4:] == '.csv', files))
    data_list = pd.DataFrame()
    i = 0
    for file in files_csv:
        # print(file)
        file_path = path + file
        # print(file_path)
        df = pd.read_csv(file_path, encoding='gbk', low_memory=False)
        if i == 0:
            data_list = data_list.append(df)
        else:
            data_list = pd.merge(data_list, df, on='entname', how='outer')
        # data_list=data_list.append(df,ignore_index=True)
        i = i + 1
    # data_list=data_list.fillna(value=0)
    data_list = data_list.drop('record_date', axis=1)
    data_list.drop(data_list[data_list.enforce_amount == "——"].index, inplace=True)
    data_list = data_list.fillna(0)
    data_list = data_list.groupby(['entname']).sum().reset_index()
    # print(data_list)
    return data_list


# In[6]:


def main3():
    file_path = dspath + 'company_baseinfo//company_baseinfo.csv'
    df = pd.read_csv(file_path, encoding='gbk', low_memory=False)
    df = df.drop_duplicates(subset='entname', keep='last', inplace=False)
    replace_values = {'entcat': 'NA', 'industryphy': 'NA', 'industryco': 'NA'}
    df_rep = df.fillna(value=replace_values)

    def is_nan(i):
        if not np.isnan(i):
            return 1
        else:
            return 0

    df_rep['is_candate'] = df_rep.apply(lambda row: is_nan(row['candate']), axis=1)
    df_rep['is_revdate'] = df_rep.apply(lambda row: is_nan(row['revdate']), axis=1)

    def encode_entstatus(i: str):
        if i == '在营（开业）企业':
            return 3
        elif i == '迁出':
            return 2
        elif i == '注销企业':
            return 1
        elif i == '吊销企业':
            return 0

    df_rep['encode_entstatus'] = df_rep.apply(lambda row: encode_entstatus(row['entstatus']), axis=1)

    df_del = df_rep.drop(['regcap', 'estdate', 'candate', 'revdate', 'entstatus',
                          'opto', 'enttype', 'entcat', 'industryphy', 'regcapcur',
                          'industryco', 'opfrom', ], axis=1)
    col_list = ['entname', 'empnum', 'is_candate', 'is_revdate', 'encode_entstatus']
    df_res = df_del.reindex(columns=col_list)
    return df_res


# In[7]:


def main4():
    file_path = dspath + 'ent_guarantee//ent_guarantee.csv'
    df = pd.read_csv(file_path, encoding='gbk', low_memory=False)

    def get_rage(rage, typ: str):
        if str(typ) in str(rage):
            return 1
        return 0

    for i in range(1, 7):
        name = 'rage_{}'.format(i)
        df[name] = df.apply(lambda row: get_rage(row['rage'], i), axis=1)

    def encode_guaranperiod(idata: str):
        if idata == '期限':
            return 1
        elif idata == '未约定':
            return 0

    df['encode_guaranperiod'] = df.apply(lambda row: encode_guaranperiod(row['guaranperiod']), axis=1)

    def encode_gatype(idata: str):
        if idata == '连带保证':
            return 2
        elif idata == '一般保证':
            return 1
        elif idata == '未约定':
            return 0

    df['encode_gatype'] = df.apply(lambda row: encode_gatype(row['gatype']), axis=1)

    def is_rage(idata: int):
        if int(idata) > 0:
            return 1
        elif int(idata) == 0:
            return 0

    df_cal = df.eval("sum_rage = rage_1 + rage_2 + rage_3 + rage_4 + rage_5 + rage_6 ", inplace=False)
    df_cal['is_rage'] = df_cal.apply(lambda row: is_rage(row['sum_rage']), axis=1)
    df_cal = df_cal.eval("sub_pefperfromto = pefperto - pefperfrom", inplace=False)
    df_del = df_cal.drop(
        ['priclaseckind', 'pefperfrom', 'pefperto', 'guaranperiod', 'gatype', 'iftopub', 'rage', 'rage_1', 'rage_2',
         'rage_3', 'rage_4', 'rage_5', 'rage_6', 'sum_rage'], axis=1)
    df_res = df_del.groupby(['entname']).mean().reset_index()
    return df_res


# In[8]:


def main5():
    file_path = dspath + 'ent_social_security//ent_social_security.csv'
    df = pd.read_csv(file_path, encoding='gbk', low_memory=False)
    df = df.drop_duplicates(subset='entname', keep='last', inplace=False)
    # 对nan的处理
    df = df.fillna(0)
    df = df.eval(
        "unpaidsocialins = unpaidsocialins_so110 + unpaidsocialins_so210 + unpaidsocialins_so310 + unpaidsocialins_so410 + unpaidsocialins_so510",
        inplace=False)
    df_res = df.drop(['updatetime', 'unpaidsocialins_so110', 'unpaidsocialins_so210', 'unpaidsocialins_so310',
                      'unpaidsocialins_so410', 'unpaidsocialins_so510'], axis=1)
    return df_res


# In[9]:


def main6():
    file_path = dspath + 'justice_judge_new//justice_judge_new.csv'
    df = pd.read_csv(file_path, encoding='gbk', low_memory=False)
    # df.drop(df[np.isnan(df['open'])].index, inplace=True)
    df = df.groupby(['entname']).size().reset_index().rename(columns={0: 'law_sum'})
    return df


# In[10]:


def main7():
    file_path = dspath + 'justice_declare//justice_declare.csv'
    df = pd.read_csv(file_path, encoding='gbk', low_memory=False)
    df = df.drop(['declaredate', 'declarestyle'], axis=1)
    df = df.fillna(0)
    df = df.groupby(['entname']).sum().reset_index()
    return df


# In[11]:


def main8():
    file_path = dspath + 'change_info//change_info.csv'
    df = pd.read_csv(file_path, encoding='gbk', low_memory=False)
    df = df.fillna(0)
    df_del = df.drop(['remark', 'dataflag', 'altitem', 'cxstatus', 'altdate', 'openo'], axis=1)
    df_res = df_del.groupby(['entname']).sum().reset_index()
    df_res['alttime'] = df_res['alttime'].astype(int)
    return df_res


# In[12]:


def main9():
    file_path = dspath + 'ent_contribution//ent_contribution.csv'
    df = pd.read_csv(file_path, encoding='gbk', low_memory=False)
    df_del = df.drop(['invtype', 'conform', 'conprop', 'condate'], axis=1)
    df_del = df_del.rename(columns={'subconam': 'sum_subconam'})
    df_del = df_del.fillna(0)
    df_res = df_del.groupby(['entname']).sum().reset_index()
    return df_res


# In[13]:


def main10():
    def function(a, b, c, d):
        if (a >= b) and (c <= d):
            return 1
        else:
            return 0

    file_path = dspath + 'ent_contribution_year//ent_contribution_year.csv'
    df = pd.read_csv(file_path, encoding='gbk', low_memory=False)
    df = df.fillna(0)
    df = df.drop(['subconcurrency', 'subconform', 'anchetype', 'acconcurrency', 'acconform'], axis=1)
    df['evaluation'] = df.apply(lambda x: function(x.liacconam, x.lisubconam, x.accondate, x.subcondate), axis=1)
    df = df.drop(['accondate', 'subcondate', 'liacconam', 'lisubconam'], axis=1)
    df = df.groupby(['entname']).mean().reset_index()
    return df


# In[14]:


def main11():
    file_path = dspath + 'jn_credit_info//jn_credit_info.csv'
    df = pd.read_csv(file_path, encoding='gbk', low_memory=False)
    df = df.drop_duplicates(subset='entname', keep='last', inplace=False)
    return df


# In[15]:


data_list = pd.DataFrame()
data_list1 = pd.DataFrame()
# 直接连接
data_list = main()
# print(data_list.columns)


# In[16]:


# enterprise_insurance
data_list1 = main1()
data_list = pd.merge(data_list, data_list1, on='entname', how='outer')


# In[17]:


# justice_enforced
data_list1 = main2()
data_list = pd.merge(data_list, data_list1, on='entname', how='outer')


# In[18]:


# company_baseinfo
data_list1 = main3()
data_list = pd.merge(data_list, data_list1, on='entname', how='outer')


# In[19]:


# ent_guarantee
data_list1 = main4()
data_list = pd.merge(data_list, data_list1, on='entname', how='outer')


# In[20]:


# ent_social_security
data_list1 = main5()
data_list = pd.merge(data_list, data_list1, on='entname', how='outer')


# In[21]:


# justice_judge_new.csv
data_list1 = main6()
data_list = pd.merge(data_list, data_list1, on='entname', how='outer')


# In[22]:


# jutice_declare
data_list1 = main7()
data_list = pd.merge(data_list, data_list1, on='entname', how='outer')


# In[23]:


# change_info
data_list1 = main8()
data_list = pd.merge(data_list, data_list1, on='entname', how='outer')


# In[24]:


# ent_contribution
data_list1 = main9()
data_list = pd.merge(data_list, data_list1, on='entname', how='outer')


# In[25]:


# ent_contribution_year
data_list1 = main10()
data_list = pd.merge(data_list, data_list1, on='entname', how='outer')


# In[26]:


# jn_credit_info
data_list1 = main11()
data_list = pd.merge(data_list, data_list1, on='entname', how='outer')


# In[27]:


# 对nan的处理
# values={'is_punish':0,'is_bra':0,'is_brap':0,'pledgenum':0,'taxunpaidnum':0}
data_list = data_list.fillna(0)


# In[28]:


# 后续预处理
data_list.loc[data_list.credit_grade == 'C', 'credit_grade'] = 1
data_list.loc[data_list.credit_grade == 'B-', 'credit_grade'] = 2
data_list.loc[data_list.credit_grade == 'B', 'credit_grade'] = 3
data_list.loc[data_list.credit_grade == 'A-', 'credit_grade'] = 4
data_list.loc[data_list.credit_grade == 'A', 'credit_grade'] = 5
data_list.loc[data_list.credit_grade == 'N', 'credit_grade'] = 6
data_list.loc[data_list.credit_grade == 'N+', 'credit_grade'] = 7

data_list = data_list.drop(['is_jnsn'], axis=1)

data_list.loc[data_list.level_rank == '省级', 'level_rank'] = 2
data_list.loc[data_list.level_rank == '市级', 'level_rank'] = 1

data_list = data_list.eval("emplynum = qcwynum + zhycnum + zlzpnum", inplace=False)
data_list = data_list.drop(['qcwynum', 'zhycnum', 'zlzpnum'], axis=1)

data_list['enforce_amount'] = data_list['enforce_amount'].astype(np.float)


# In[29]:


data_list1 = data_list.copy(deep=True)


# In[30]:


data_list1


# In[31]:


file_path = dspath + 'final2_weight.csv'
df_weight = pd.read_csv(file_path, encoding='gbk', low_memory=False)


# In[32]:


df_weight


# In[33]:


entname = data_list1['entname']
# data_list1 = data_list1.drop('entname',axis=1)
# 下面drop掉了entname
for idx in df_weight.columns:
    if np.isnan(df_weight[idx][0]) or df_weight[idx][0] == 0:
        print(df_weight[idx])
        df_weight.drop([idx], axis=1, inplace=True)
        data_list1.drop([idx], axis=1, inplace=True)


# In[34]:

#data_list2=pd.DataFrame()
#for idx in df_weight.columns:
#    mn=df_weight[idx][0]
#    if mn>=0:
#        mn=1
#    else:
#        mn=-1
#    data_list2[idx]=data_list1.apply(lambda row: row[idx]*mn,axis=1)


# In[35]:

cols=['taxunpaidnum', 'level_rank', 'is_justice_creditaic',
       'is_except', 'is_brap', 'investnum', 'ipat_num', 'is_infoa',
       'is_justice_credit', 'ibrand_num', 'is_kcont', 'branchnum', 'idom_num',
       'shopnum', 'bidnum', 'is_punish', 'is_bra', 'pledgenum', 'is_infob',
       'icopy_num', 'passpercent', 'cbzt', 'enforce_amount', 'empnum',
       'encode_entstatus', 'priclasecam', 'encode_guaranperiod',
       'encode_gatype', 'is_rage', 'sub_pefperfromto', 'unpaidsocialins',
       'law_sum', 'defendant', 'alttime', 'evaluation', 'credit_grade']

#data_list2=data_list2.reindex(columns=cols)
data_list2=data_list1.reindex(columns=cols)



# 导出未加权的数据集(但是drop了空权值的列)
data_nowei = data_list2.copy(deep=True)
mm=MinMaxScaler()
saver_mm=open('mm.pickle',mode='wb')
cols=data_nowei.columns
print(cols)
data_nowei_mm=mm.fit_transform(data_nowei)
pickle.dump(mm,saver_mm)
data_nowei_mm=pd.DataFrame(data_nowei_mm,columns=cols)
data_nowei_mm.insert(value=entname, loc=0, column='entname')
data_nowei_mm.to_csv(dspath + "data_nowei_mm.csv")
data_nowei_mm



ed_time=datetime.datetime.now()
print(f'Process Time:{ed_time-st_time}')
