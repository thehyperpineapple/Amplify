# Import requirements
from flask import Flask, render_template, jsonify
import os
from dotenv import find_dotenv,load_dotenv
import thingspeak

import json

# Fetch environment variables
dotenv_path = find_dotenv()
load_dotenv(dotenv_path)

channel_id = os.getenv("CHANNEL_ID")
read_key = os.getenv("READ_API_KEY")

# Connect to ThingSpeak & Fetch data
channel = thingspeak.Channel(id=channel_id, api_key = read_key)
response = channel.get({"results": 1000})
response = json.loads(response)

# Flask App
app = Flask(__name__)

@app.route("/",methods=['GET'])
def home():
    unit_list, time_list, date_list, threshold_list, red_line_list, blue_line_list, yellow_line_list, net_current_list = [], [], [], [], [], [], [], []

    unit = response['channel']['field1']
    time = response['channel']['field2']
    date = response['channel']['field3']
    threshold = response['channel']['field4']
    red_line = response['channel']['field5']
    blue_line = response['channel']['field6']
    yellow_line = response['channel']['field7']
    net_current = response['channel']['field8']
    entries = response['channel']['last_entry_id']

    for entry in range(0, len(response['feeds'])):
        if float(response['feeds'][entry]['field4']) < float(response['feeds'][entry]['field8']):
            unit_list.append(response['feeds'][entry]['field1'])
            time_list.append(response['feeds'][entry]['field2'])
            date_list.append(response['feeds'][entry]['field3'])
            threshold_list.append(response['feeds'][entry]['field4'])
            red_line_list.append(response['feeds'][entry]['field5'])
            blue_line_list.append(response['feeds'][entry]['field6'])
            yellow_line_list.append(response['feeds'][entry]['field7'])
            net_current_list.append(response['feeds'][entry]['field8'])

    return render_template("index.html", length = len(unit_list), unit_list = unit_list[::-1], date_list = date_list[::-1], time_list = time_list[::-1], threshold_list = threshold_list[::-1], red_line_list = red_line_list[::-1], blue_line_list = blue_line_list[::-1], yellow_line_list = yellow_line_list[::-1], net_current_list = net_current_list[::-1])

if __name__ == "__main__":
    app.run(host='0.0.0.0', debug=True)