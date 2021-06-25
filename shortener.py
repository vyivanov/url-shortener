import flask
import database

BASE_URL = 'http://localhost:5000'

service = flask.Flask(__name__)
db      = database.Database()

@service.route('/api', methods=['GET', 'POST'])
def shortening_via_api():
    url = flask.request.args.get('url', type=str)
    if url:
        return '{0}/{1}'.format(BASE_URL, db.insert(url))
    else:
        return 'api index'

@service.route('/', methods=['GET', 'POST'])
def shortening_via_web():
    url = flask.request.args.get('url', type=str)
    if url:
        link = '{0}/{1}'.format(BASE_URL, db.insert(url))
        return '<center><a href="{0}">{0}</a></center>'.format(link)
    else:
        return 'web index'

@service.route('/<string:key>', methods=['GET', 'POST'])
def redirecting(key):
    url = db.search(key)
    if url:
        return flask.redirect(url)
    else:
        return '<center><b>not found</b></center>'

@service.route('/favicon.ico', methods=['GET', 'POST'])
def favicon():
    return str()

if __name__ == '__main__':
    service.run(port=5000, debug=True)
