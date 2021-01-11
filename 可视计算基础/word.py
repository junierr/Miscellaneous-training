import requests
import re

url = 'http://www.biquge.info/0_383/'

response = requests.get(url)
response.encoding = response.apparent_encoding
html_data = response.text

result_list = re.findall('<dd><a href="(.*?)" title=".*">.*</a></dd>', html_data)


top_10 = result_list[1:11]

for top in top_10:
    all_url = url + top

    response_2 = requests.get(all_url)
    response_2.encoding = response_2.apparent_encoding
    html_data_2 = response_2.text

    title = re.findall('<h1>(.*?)</h1>', html_data_2, re.S)[0]
    contend = re.findall('<div id="content"><!--go-->(.*?)</div>', html_data_2, re.S)[0]
    print(title, contend)

    with open(title + '.txt', mode='w', encoding='utf-8') as f:
        f.write(contend.replace('&nbsp;', '').replace('<br/>', '\n'))
        print('loading:', title)
    print(" down over")