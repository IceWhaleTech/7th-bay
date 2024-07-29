import requests
import json
import time

# Define the IP address and port number of ESP32-C3
url = "http://172.16.1.1/post"

# Define the JSON data to be sent
data_template = {
    "id": 1,
    "speed": 4,
    "data": [
        {"h": 21, "s": 255, "v": 255}
    ]
}

# Send a POST request
response = requests.post(url, json=data_template)

# Print the response status code and result
print(f"Status Code: {response.status_code}")
# If the request is successful, it returns Status Code: 200
