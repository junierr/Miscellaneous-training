#!/usr/bin/env python
# coding: utf-8

# ### 注：运行processB必须先运行processA.py 以获取 final4_mulwei_nopca.csv

# In[21]:


#the all
import re
import os 
import pandas as pd
import numpy as np
from sklearn import preprocessing
from sklearn.decomposition import PCA
from sklearn.preprocessing import StandardScaler
from sklearn.model_selection import train_test_split
import time
import datetime
st_time=datetime.datetime.now()

# In[22]:


# dataset文件夹路径
# 替换此处即可
dspath= '..//data//'


# In[23]:


# 从final.ipynb 导出的final4_mulwei继续
file_path = dspath + 'final4_mulwei_nopca.csv'
data_wei=pd.read_csv(file_path,encoding='gbk',low_memory=False,index_col=0)


# In[24]:


data_wei


# In[25]:


data_wei.columns


# In[26]:


# 拆分数据集
df_bj = pd.DataFrame(data_wei,columns=['entname','empnum','encode_entstatus','shopnum', 'branchnum', 'is_infoa', 'is_infob', 'level_rank'])
df_wd = pd.DataFrame(data_wei, columns=['entname','alttime'])
df_jn = pd.DataFrame(data_wei, columns=['entname','evaluation','investnum','bidnum','cbzt','ibrand_num','icopy_num','ipat_num','idom_num','passpercent'])
df_jf = pd.DataFrame(data_wei, columns=['entname','priclasecam','encode_guaranperiod','encode_gatype','is_rage','sub_pefperfromto',
                                        'unpaidsocialins','is_bra','is_brap','pledgenum','taxunpaidnum','is_except'])
df_sf = pd.DataFrame(data_wei, columns=['entname','law_sum','defendant', 'enforce_amount','is_justice_credit'])
df_xf = pd.DataFrame(data_wei, columns=['entname','is_punish','is_kcont','credit_grade','is_justice_creditaic'])


# In[27]:


df_list = [df_bj,df_wd,df_jn,df_jf,df_sf,df_xf]
dfname_list= ['df_bj','df_wd','df_jn','df_jf','df_sf','df_xf']


# In[28]:


# 分别导出,存到split_dataset文件夹
folder = f"{dspath}split_dataset"
if not os.path.exists(folder):
    os.makedirs(folder)
i=0
for df in df_list:
    df.to_csv(f"{dspath}split_dataset//final4_{dfname_list[i]}.csv")
    i=i+1

ed_time=datetime.datetime.now()
print(f'Process Time:{ed_time-st_time}')


#分表求和并导出
folder = f"{dspath}split_data_sum"
if not os.path.exists(folder):
    os.makedirs(folder)
i=0
dfsum_list=[]
dfsumname_list=[]
for df in df_list:
    colsum=f'{dfname_list[i]}_sum'
    dfsumname_list.append(colsum)
    entname=df['entname']
    df_tmp = df.drop(['entname'],axis=1)
    df_sum=pd.DataFrame(entname)
    df_sum[colsum] = df_tmp.apply(lambda x: x.sum(), axis=1)
    df_sum.to_csv(f'{folder}//final4_{dfname_list[i]}_sum.csv')
    dfsum_list.append(df_sum.copy(deep=True))
    i=i+1


# In[30]:


dfsum_list


# In[46]:


#分表求和的结果合并后导出
i=0
for df in dfsum_list:
    if i==0:
        dfsum_merge=df.copy(deep=True)
    else:
        dfsum_merge=pd.merge(dfsum_merge,df,on='entname',how='inner')
    print(dfsum_merge)
    i=i+1
dfsum_merge.to_csv(f'{dspath}final4_sum_merge.csv')


# In[ ]:


df


# In[74]:


a= dfsum_list[0].duplicated(subset=['entname'])
p= pd.DataFrame(a)
(p.loc[p[0] == True]).size





