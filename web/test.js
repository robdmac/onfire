var redis = require("redis"),
    client = redis.createClient();

client.on("error", function(error) {
    console.log("error " + error);
});

const express = require("express"),
      app = express();

var body_parser = require("body-parser");
app.use(body_parser.json());

var PubNub = require('pubnub'),
    pubnub = new PubNub({
	publishKey: "pub-c-07741803-e3cd-44c0-ade1-7e50df4dad5b",
	subscribeKey: "sub-c-d8f3705a-9b20-11e7-b8fe-0e6b753288ab"
    });

app.engine('html', require('ejs').renderFile);

app.get('/', function(req, res) {
    client.get("test_data", function(error, value) {
	res.render('index.html', {test_data: value});
    });
});

app.use('/static', express.static(__dirname + "/static"));

var point_list = [];

app.post('/post', function(req, res) {
    if(!("client_id" in req.body))
      {
	res.end("no client_id field");
      }
    else if("latitude" in req.body && "longitude" in req.body)
    {
	var color = "green";
	var heart_rate = 80;
	if("heart_rate" in req.body) { heart_rate = req.body.heart_rate; }
	if("color" in req.body) { color = req.body.color; }
        point_list[req.body.client_id] = {client_id: req.body.client_id, latitude: req.body.latitude, longitude: req.body.longitude, color: color, heart_rate: heart_rate};
	  pubnub.publish({
	      channel: "points",
	      message: {points: point_list}
	  });
	  res.end("OK");
      }
    else
      {
	  client.set("test_data", req.body.test_data, redis.print);
	  pubnub.publish({
	      channel: "test_data",
	      message: req.body.test_data
	  });
	  res.end("OK");
      }
});

app.listen(80, function() {
    console.log("Listening on port 80!");
});

//client.quit();
