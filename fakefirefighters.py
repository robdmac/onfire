import time
import json
import requests

headcount = 5
values = 5
fireFightersDefaultValues = [[' ' for y in range(values)] for x in range(headcount)]
headers = {'content-type': 'application/json', 'Accept-Charset': 'UTF-8'}

for x in range(headcount):
    fireFightersDefaultValues [x][0] = 37.4633
    fireFightersDefaultValues [x][1] = 122.2310

while True:
	for x in range(headcount):
		lat = fireFightersDefaultValues [x][0] + ((x % 6)-3)*0.001
		lon = fireFightersDefaultValues [x][1] - ((x % 6)-3)*0.001
		data = {"test_data":"python script works", "client_id": (x+1), "ts": time.strftime("%c"), "lat": lat, "lon": lon}
		json_data = json.dumps(data)
		print "sending firefighter " + str(x+1) + " " + json_data
		r = requests.post("http://demo.onfire.io/post", data=json_data, headers=headers)
		print r
	time.sleep(10)
