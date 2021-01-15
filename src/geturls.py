import requests
import re
import inspect
import json
import socket
import os
import logging
import sys
from urllib.parse import unquote
from urllib.parse import urlparse
from urllib.parse import parse_qs

'''
Connect to player and set logfile
'''
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect(("127.0.0.1", 9342))
sock.send(b'[INIT]python connect')

'''
Log setting
'''
dir_path = os.path.dirname(os.path.realpath(__file__))
logname = os.path.join(dir_path, "YTThread")
logging.basicConfig(filename=logname,
                    filemode='w',
                    format='%(asctime)s,%(msecs)d [%(levelname)s %(threadName)s] %(name)s %(message)s',
                    datefmt='%H:%M:%S',
                    level=logging.DEBUG)

logging.info("Module Loaded")

logger = logging.getLogger(__name__)
handler = logging.StreamHandler(stream=sys.stdout)
logger.addHandler(handler)


def handle_exception(exc_type, exc_value, exc_traceback):
    if issubclass(exc_type, KeyboardInterrupt):
        sys.__excepthook__(exc_type, exc_value, exc_traceback)
        return

    logging.error("Uncaught exception", exc_info=(exc_type, exc_value, exc_traceback))

sys.excepthook = handle_exception


class Sig(object):
    """
    sig class
    All methods and attributs are wrritten for running js style code
    """

    def __init__(self, str):
        self.strList = list(str)

    def __getitem__(self, indx):
        return self.strList[indx]

    def __setitem__(self, indx, val):
        self.strList[indx] = val

    def splice(self, start, length):
        del self.strList[start: (start + length)]

    def reverse(self):
        self.strList.reverse()

    @property
    def length(self):
        return len(self.strList)


def getlists(url):
    sock.send(b'woker started')
    logging.info("Worker Started")

    """
    Args:
      param url (string): A watchable playlist url. eg:https://www.youtube.com/watch?v=${VIDEO_ID}&list=${LIST_ID}
    """
    s = requests.session()
    s.headers = {
        'authority': 'www.youtube.com',
        'cache-control': 'max-age=0',
        'upgrade-insecure-requests': '1',
        'user-agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.97 Safari/537.36',
        'accept': 'text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9',
        'sec-fetch-site': 'same-origin',
        'sec-fetch-mode': 'navigate',
        'sec-fetch-user': '?1',
        'sec-fetch-dest': 'document',
        'accept-language': 'en,zh-CN;q=0.9,zh;q=0.8,en-US;q=0.7,zh-TW;q=0.6',
    }
    res = s.get(url)
    if res.status_code != 200:
        return -2
    # Get Player
    playerID = re.search('/s/player/(.*?)/player_ias.vflset/.*?/base.js', res.text)
    baseJS = s.get("https://www.youtube.com%s" % playerID[0])

    '''
    Deciphier Analyze
    Translate JS to python and define original functions in python
    '''
    if baseJS.status_code == 200:
        deciphierProgess = re.search('\{a=a\.split\(""\);(.*?);return a\.join\(""\)\};', baseJS.text)
        deciphierFunctions = re.search('var %s=\{([\s|\S]*?)\};' % deciphierProgess[1].split(";")[0][:2], baseJS.text)
        funs = deciphierFunctions[1]
        trans = [':', "function", "var", "},", "{", "}"]
        for c in trans:
            n = ''
            if c == "(":
                n = " "
            elif c == "{":
                n = ":"
            funs = funs.replace(c, n)
        fs = []
        for st in funs.split("\n"):
            st = "def " + st.replace(":", ":\n ")
            st = st.replace(";", "\n  ") + "\n"
            exec(st)
            fs.append(st)

        # Deciphier progress in queue mode
        progress = re.findall('\.([\S]{2})\(a,(.*?)\)', deciphierProgess[0])

    # Get all videoID
    listId = re.search('list=(.*?)(&|$)', url)[1]
    ids = set(re.findall('"videoId":"(.{11})","playlistId":"%s"' % listId, res.text))
    if len(ids) == 0:
        return -1
    total = len(ids)
    count = 0
    for vid in ids:
        try:
            res = s.get("https://www.youtube.com/get_video_info?video_id=%s&el=detailpage" % vid)
            if res.status_code != 200:
                continue
            '''OLD_PROCESS_ALL_DATA
            #player_response = unquote(unquote(re.search('player_response=(.*?)&', res.text)[1]))
            #print(player_response)
            #streamingData = json.loads(player_response)['streamingData']
            #stream = streamingData['formats'][0]
            #data = stream['signatureCipher'].split('&')
            '''
            # only streamingData signatureCipher
            data = re.search('signatureCipher":"(.*?)"', unquote(unquote(res.text))
                             )[1].replace("\\u0026", "&").split('&')
            print(data)
            if data[1] == 'sp=sig':
                del data[1]
                data[1] = data[1].replace('url=', '')

                # Use Sig class and calculet sig
                sData = Sig(data[0].replace('s=', ''))
                for (fun, arg) in progress:
                    paramLen = len(inspect.signature(eval(fun)).parameters)
                    line = ""
                    if paramLen == 1:
                        line = "{}({})".format(fun, "sData")

                    else:
                        line = "{}({},{})".format(fun, "sData", arg)
                    #print("DECIPHER RUNING: ", line)
                    exec(line)
                del data[0]

                data.append("sig=%s" % ''.join(sData.strList))
                urlr = '&'.join(data)
                count = count + 1
                #print(urlr.encode(), "\n")
                # os._exit(1)

                #l = sock.recv(1024)
                sock.send(urlr.encode())
        except Exception as e:
            logging.exception(e)
            # os._exit(1)
            sock.send(vid.encode() + b'FAILED')
            continue
    return (count, total)

if __name__ == '__main__':
    getlists("https://www.youtube.com/watch?v=83xBPCw5hh4&list=RDCLAK5uy_kmPRjHDECIcuVwnKsx2Ng7fyNgFKWNJFs")
