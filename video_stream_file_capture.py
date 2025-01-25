import cv2
import requests
import numpy as np
import base64
import sys
import os

list_of_arguments = sys.argv

script_dir = os.path.dirname(os.path.realpath(__file__))

camera_number = list_of_arguments[1]
url = "http://192.168.1." + camera_number + "/mjpg/video.mjpg"

# Open a connection to the MJPEG stream
username = "root"
password = "XdXK3KupL5AGb"
response = requests.get(url, auth=(username, password), stream=True)
bytes_iterator = response.iter_content(chunk_size=4096)

# Function to parse MJPEG frames
def parse_mjpeg(frame):
    # Find the start and end of the JPEG image
    start = frame.find(b'\xff\xd8')
    end = frame.find(b'\xff\xd9')
    
    # Extract the JPEG image from the frame
    if start != -1 and end != -1:
        jpg = frame[start:end + 2]
        return cv2.imdecode(np.frombuffer(jpg, dtype=np.uint8), cv2.IMREAD_COLOR)

    return None

try:
    while True:
        # Read MJPEG frame from the stream
        frame = b''
        for chunk in bytes_iterator:
            frame += chunk
            a = frame.find(b'\xff\xd8')
            b = frame.find(b'\xff\xd9')
            if a != -1 and b != -1:
                jpg = frame[a:b + 2]
                frame = frame[b + 2:]
                # Display the MJPEG video stream
                img = cv2.imdecode(np.frombuffer(jpg, dtype=np.uint8), cv2.IMREAD_COLOR)
                if img is not None:
                    img_rgb = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
                    _, jpg = cv2.imencode('.jpg', img_rgb)
                    encoded_img = base64.b64encode(jpg).decode('utf-8')

                    with open(f"{script_dir}/camera{camera_number}.txt", "w") as file:
                        file.write(encoded_img)
                    # cv2.imshow("MJPEG Stream", img_rgb)
                    # if cv2.waitKey(1) & 0xFF == ord('q'):
                    #     break

finally:
    # Release resources
    response.close()
