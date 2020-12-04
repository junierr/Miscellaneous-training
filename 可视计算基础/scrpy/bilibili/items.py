# Define here the models for your scraped items
#
# See documentation in:
# https://docs.scrapy.org/en/latest/topics/items.html

import scrapy


class BilibiliItem(scrapy.Item):
    # define the fields for your item here like:
    # name = scrapy.Field()
    number = scrapy.Field()
    badge = scrapy.Field()
    cover_img = scrapy.Field()
    index_show = scrapy.Field()
    link = scrapy.Field()
    media_id = scrapy.Field()
    order_type = scrapy.Field()
    season_id = scrapy.Field()
    title = scrapy.Field()
    pass
