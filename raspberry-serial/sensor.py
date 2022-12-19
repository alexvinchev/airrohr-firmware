from datetime import datetime
import requests
import pytz
import json
from config import settings


class Sensor(object):

    sensor_type = None
    sampling_rate = None
#    url = "http://localhost:8000/v1/push-sensor-data/"
    url = "https://api.dusti.xyz/v1/push-sensor-data/"
    whitelist = []

    def __init__(self, filename=None):
        if self.sensor_type:
            self.sensor_uid = 'SENSOR_{}_UID'.format(self.sensor_type)
        else:
            self.sensor_uid = 'SENSOR_UID'
        if filename:
            self.filename = "{}.{}".format(filename, self.sensor_type)

    def filter(self, json_data, prefix=False):
        # filter all fields not in whitelist
        data = {}
        for key in json_data.keys():
            if prefix:
                key_prefix = key.split('_')[0]
                if key_prefix.lower() != self.sensor_type.lower():
                    continue
                key_suffix = '_'.join(key.split('_')[1:])
            else:
                key_suffix = key
            if key_suffix in self.whitelist:
                data[key_suffix] = json_data[key]
        return data

    def check(self, message):
        # some checks to elimanate some of the 400s from the API
        if ':' not in message:
            return False
        if ';' not in message:
            return False
        if len(message.split(':')) != len(message.split(';')) + 1:
            return False
        return True

    def parse(self, message):
        if not self.check(message):
            return False
        json_data = dict(map(lambda x: x.split(':'),
                             message.strip().split(';')))
        return self.filter(json_data, prefix=True)

    def send(self, data, timestamp=None):
        payload = {
            "sampling_rate": self.sampling_rate,
            "sensordatavalues": []
        }
        if timestamp:
            payload['timestamp'] = timestamp

        for key, value in data.iteritems():
            d = {'value': value, 'value_type': key}
            payload['sensordatavalues'].append(d)

        headers = {'SENSOR': settings.get(self.sensor_uid),
                   'Content-Type': 'application/json'}
        r = requests.post(self.url, data=json.dumps(payload),
                          headers=headers, verify=False)
        print(r.status_code)
        print(r.text)

    def log(self, data):
        dt = str(pytz.timezone('Europe/Berlin').localize(datetime.now()))
        line = "{}| {}\n".format(dt, json.dumps(data))
        with open(self.filename, "a") as fp:
            print(line.strip())
            fp.write(line)


class SensorPPD42NS(Sensor):

    sensor_type = 'PPD42NS'
    sampling_rate = '15000'
    whitelist = ['P1', 'P2', 'durP1', 'durP2', 'ratioP1', 'ratioP2']


class SensorSHT10(Sensor):

    sensor_type = 'SHT10'
    sampling_rate = None
    whitelist = ['temperature', 'humidity']


class SensorGP2Y10(Sensor):

    sensor_type = 'GP2Y1010AU0F'
    sampling_rate = '40'
    whitelist = ['vo_raw', 'voltage', 'dust_density']


class SensorDSM501A(Sensor):

    sensor_type = 'dsm501a'
    sampling_rate = '15000'
    whitelist = ['P10', 'P25', 'durP10', 'durP25', 'ratioP10', 'ratioP25']


class SensorBMP180(Sensor):

    sensor_type = 'BMP180'
    sampling_rate = None
    whitelist = ['temperature', 'pressure', 'altitude', 'pressure_sealevel']
