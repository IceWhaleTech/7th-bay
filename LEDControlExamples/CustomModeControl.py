import requests
import json
import time

# Define the IP address and port number of ESP32-C3
url = "http://172.16.1.1/post"

# Define the JSON data to be sent
data_template = {
  "id": 5,  # ID for custom mode
  "data": [
    # Each object represents the color and brightness settings for one light
    {"h": 0, "s": 255, "v": 255},  # Light 1 Red
    {"h": 85, "s": 255, "v": 255},  # Light 2 Green
    {"h": 168, "s": 255, "v": 255},  # Light 3 Blue
    {"h": 42, "s": 255, "v": 255},  # Light 4 Yellow
    {"h": 212, "s": 255, "v": 255},  # Light 5 Purple
    {"h": 128, "s": 255, "v": 255},  # Light 6 Cyan
    {"h": 21, "s": 255, "v": 255},  # Light 7 Orange
    {"h": 128, "s": 255, "v": 255},  # Light 8 Cyan
    {"h": 212, "s": 255, "v": 255},  # Light 9 Purple
    {"h": 42, "s": 255, "v": 255},  # Light 10 Yellow
    {"h": 168, "s": 255, "v": 255},  # Light 11 Blue
    {"h": 85, "s": 255, "v": 255},   # Light 12 Green
    {"h": 0, "s": 255, "v": 255}   # Light 13 Red
  ]
}



# Send a POST request
response = requests.post(url, json=data_template)

# Print the response status code
print(f"Status Code: {response.status_code}")
# If the request is successful, it returns Status Code: 200

