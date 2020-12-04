from scrapy import cmdline
import os
import sys

sys.path.append(os.path.dirname(os.path.abspath(__file__)))
cmdline.execute(['scrapy', 'crawl', 'drama'])

