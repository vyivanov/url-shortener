import psycopg2

conn = psycopg2.connect(
    dbname='url', user='url', password='test', host='localhost')

cursor = conn.cursor()
cursor.execute('SELECT * FROM url')

records = cursor.fetchall()
print(records)
