import time
import json
import requests
import random
import math

headcount = 10
values = 5
fireFightersDefaultValues = [[' ' for y in range(values)] for x in range(headcount)]
headers = {'content-type': 'application/json', 'Accept-Charset': 'UTF-8'}

for x in range(headcount):
    fireFightersDefaultValues [x][0] = 37.7633 + (random.randrange(100)-50)*0.0005
    fireFightersDefaultValues [x][1] = -122.4310 - (random.randrange(100)-50)*0.0005
    fireFightersDefaultValues [x][2] = 70 + random.randrange(30)


while True:
	for x in range(headcount):
		fireFightersDefaultValues [x][2] += random.randrange(4)-2
		fireFightersDefaultValues [x][0] += (random.randrange(6)-3)*0.0005
		fireFightersDefaultValues [x][1] += (random.randrange(6)-3)*0.0005
		lat = fireFightersDefaultValues [x][0]
		lon = fireFightersDefaultValues [x][1]
		heartbeatrate = fireFightersDefaultValues [x][2]
		data = {"test_data":"python script works", "client_id": (x+1), "ts": time.strftime("%c"), "latitude": lat, "longitude": lon, "heart_rate": heartbeatrate}
		json_data = json.dumps(data)
		print "sending firefighter " + str(x+1) + " " + json_data
		r = requests.post("http://demo.onfire.io/post", data=json_data, headers=headers)
		print r
	time.sleep(10)
