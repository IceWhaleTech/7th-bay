import requests
import json
import time

# Define the IP address and port number of ESP32-C3
url = "http://172.16.1.1/post"

# Define the JSON data to be sent
data_template = {
    "id": 5,
    "data": [
        # Hue (color tone), Saturation (color purity), Value (brightness)
        # h = 0 ~ 365
        # s = 0 ~ 255
        # v = 0 ~ 255
        {"h": 32, "s": 255, "v": 255},
        {"h": 32, "s": 255, "v": 255},   
        {"h": 32, "s": 255, "v": 255},   
        {"h": 32, "s": 255, "v": 255},  
        {"h": 32, "s": 255, "v": 255},    
        {"h": 32, "s": 255, "v": 255},    
        {"h": 32, "s": 255, "v": 255},
        {"h": 32, "s": 255, "v": 255},
        {"h": 32, "s": 255, "v": 255},
        {"h": 32, "s": 255, "v": 255},
        {"h": 32, "s": 255, "v": 255},
        {"h": 32, "s": 255, "v": 255},
        {"h": 32, "s": 255, "v": 255}
    ]
}

# Send a POST request
response = requests.post(url, json=data_template)

# Print the response status code
print(f"Status Code: {response.status_code}")
# If the request is successful, it returns Status Code: 200

