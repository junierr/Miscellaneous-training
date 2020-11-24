import scrapy
import json
from bilibili.items import BilibiliItem


class DramaSeries(scrapy.Spider):
    name = 'drama'
    allowed_domains = ['https://api.bilibili.com/']
    i = 1
    start_urls = ['https://api.bilibili.com/pgc/season/index//result?page=%s&season_type=1&pagesize=20&type=1' % s for s
                  in range(1, 101)]

    def parse(self, response):
        item = BilibiliItem()
        drama = json.loads(response.text)
        data = drama['data']
        data_list = data['list']
        #print(data_list)
        for filed in data_list:
            item['number'] = self.i
            item['badge'] = filed['badge']
            item['cover_img'] = filed['cover']
            item['index_show'] = filed['index_show']
            item['link'] = filed['link']
            item['media_id'] = filed['media_id']
            item['order_type'] = filed['order_type']
            item['season_id'] = filed['season_id']
            item['title'] = filed['title']
            #print(self.i, item)
            self.i += 1
            yield item
