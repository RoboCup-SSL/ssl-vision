import cv2
import requests
import numpy as np
import base64
import sys

list_of_arguments = sys.argv

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
                # img = cv2.imdecode(np.frombuffer(jpg, dtype=np.uint8), cv2.IMREAD_COLOR)
                encoded_img = base64.b64encode(jpg).decode('utf-8')
                with open(f"/home/sebbe/ssl-vision/camera{camera_number}.txt", "w") as file:
                    file.write(encoded_img)
                # Display the MJPEG video stream
                # cv2.imshow("MJPEG Stream", img)
                # if cv2.waitKey(1) & 0xFF == ord('q'):
                #     break

finally:
    # Release resources
    response.close()
