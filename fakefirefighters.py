import time
import json
import requests
import random

headcount = 10
values = 5
fireFightersDefaultValues = [[' ' for y in range(values)] for x in range(headcount)]
headers = {'content-type': 'application/json', 'Accept-Charset': 'UTF-8'}

for x in range(headcount):
    fireFightersDefaultValues [x][0] = 37.4633
    fireFightersDefaultValues [x][1] = -122.2310
    fireFightersDefaultValues [x][2] = 70 + random.randrange(30)


while True:
	for x in range(headcount):
		fireFightersDefaultValues [x][2] += random.randrange(4)-2
		lat = fireFightersDefaultValues [x][0] + ((x % 6)-3)*0.001
		lon = fireFightersDefaultValues [x][1] - ((x % 6)-3)*0.001
		heartbeatrate = fireFightersDefaultValues [x][2]
		data = {"test_data":"python script works", "client_id": (x+1), "ts": time.strftime("%c"), "latitude": lat, "longitude": lon, "heartbeatrate": heartbeatrate}
		json_data = json.dumps(data)
		print "sending firefighter " + str(x+1) + " " + json_data
		r = requests.post("http://demo.onfire.io/post", data=json_data, headers=headers)
		print r
	time.sleep(10)
