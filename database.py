#!/usr/bin/env python

import hashids
import pymysql

from urllib.parse import urlparse

class Database(object):

    def __init__(self):
        self.db = \
            pymysql.connect(host     = 'vova-ivanov.info',
                            user     = 'url',
                            password = '!6BpH9RAsWGAqYfq!L6',
                            database = 'url')

        self.hash = \
            hashids.Hashids(salt = 'shortener')

        self.cursor = \
            self.db.cursor()

    def insert(self, url):
        if not self.__is_net(url):
            url = 'http://{0}'.format(url)
        sql = "INSERT INTO `url` (`link`) VALUES ('{0}')".format(url)
        self.__do_request(sql)
        idx = self.cursor.lastrowid
        key = self.hash.encode(idx)
        return key

    def search(self, key):
        idx = self.hash.decode(key)
        assert len(idx) == 1 or len(idx) == 0
        url = None
        if len(idx) == 1:
            sql = "SELECT `link` FROM `url` WHERE `id` = '{0}'".format(idx[0])
            out = self.__do_request(sql)
            if out:
                assert len(out) == 1 and len(out[0]) == 1
                url = out[0][0]
        return url

    def __do_request(self, sql):
        self.db.ping(reconnect = True)
        self.cursor.execute(sql)
        self.db.commit()
        return self.cursor.fetchall()

    def __is_net(self, url):
        return bool(urlparse(url).netloc)

