# Define your item pipelines here
#
# Don't forget to add your pipeline to the ITEM_PIPELINES setting
# See: https://docs.scrapy.org/en/latest/topics/item-pipeline.html


# useful for handling different item types with a single interface
import json

from openpyxl import Workbook
from scrapy.utils.project import get_project_settings
settings = get_project_settings()

class BilibiliPipeline(object):
    excelBook = Workbook()
    activeSheet = excelBook.active
    file = ['number', 'title', 'link', 'media_id', 'season_id', 'index_show', 'cover_img', 'badge']
    activeSheet.append(file)
    def process_item(self, item, spider):
        files = [item['number'], item['title'], item['link'], item['media_id'], item['season_id'], item['index_show'],
                 item['cover_img'], item['badge']]
        self.activeSheet.append(files)
        self.excelBook.save('C:/Users/59723/PycharmProjects/bilibili/bilibili/drama.xlsx')
        return item
