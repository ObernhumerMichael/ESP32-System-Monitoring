from datetime import datetime
import json
import paho.mqtt.client as mqtt
import subprocess
import os
from time import sleep


def getTimestamp():
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")


def log(message):
    log = getTimestamp() + " | " + message + "\n"
    print(log)
    with open("logs.txt", "a") as file:
        file.write(log)


def checkFileSystem():
    try:
        # Open a file in write mode
        with open("temp.txt", "w") as file:
            file.write(
                "hello, this is some text written into the file to check if the system is read only or something similar!\n"
            )
        print("file created and text written successfully!")

        # Read the content of the file
        with open("temp.txt", "r") as file:
            content = file.read()
        print("file was read successfully")

        # Delete the file
        os.remove("temp.txt")
        print("File has been deleted")

        return True
    except IOError as e:
        log("Error: Unable to write/read/delete to the file:" + str(e))
    except Exception as e:
        log("Error:" + (e))
    return False


def ping(target="1.1.1.1"):
    try:
        # Run the ping command
        result = subprocess.run(
            ["ping", "-c", "5", target], capture_output=True, text=True, timeout=10
        )

        # Check the return code to see if the ping was successful
        if result.returncode == 0:
            print("Ping to " + target + " successful!")
            return True
        else:
            log("ping to " + target + " failed! error message: " + result.stderr)
    except subprocess.TimeoutExpired:
        log("Ping timed out.")
    except Exception as e:
        log("An error occurred:" + str(e))
    return False


def checkSystem():
    isDNSFunctional = ping("google.com")
    isInternetReachable = ping()
    isFileSystemAlive = checkFileSystem()

    data = {
        "timestamp": getTimestamp(),
        "isDNSFunctional": isDNSFunctional,
        "isInternetReachable": isInternetReachable,
        "isFileSystemAlive": isFileSystemAlive,
    }

    if isFileSystemAlive and isInternetReachable and isDNSFunctional:
        print("Everything seems fine")
    else:
        print("There is something wrong with the system")

    return data


def on_connect(client, userdata, flags, rc):
    log("Connected to MQTT broker established.")


def on_connect_fail(client, userdata, flags, rc):
    log("Failed to establish a connection with the MQTT broker!")


def on_disconnect(client, userdata, rc):
    log("Disconnected from MQTT broker!")


def on_publish(client, userdata, rc):
    print("Published successfully")


# MQTT config
broker_address = "localhost"
port = 1883
keepalive = 60
topic = "raspi-01/health"

# initialize the instance
client = mqtt.Client()
client.on_connect = on_connect
client.on_connect_fail = on_connect_fail
client.on_disconnect = on_disconnect
client.on_publish = on_publish
client.loop_start()

while True:
    try:
        if client.is_connected():
            print("Still connected to the MQTT broker")
            data = checkSystem()
            client.publish(topic, json.dumps(data))
            sleep(30)
        else:
            log("There is no connection to the MQTT broker")
            client.connect(broker_address, port, keepalive)
            sleep(5)
    except Exception as e:
        log("Error occurred: " + str(e))
        sleep(5)
