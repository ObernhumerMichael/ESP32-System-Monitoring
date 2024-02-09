<h1>ESP32-System-Monitoring</h1>

- [Description](#description)
- [Things needed](#things-needed)
- [Roadmap](#roadmap)
- [Client](#client)
- [Installation](#installation)
  - [Logs](#logs)

# Description

This project should achieve the following things:

- Monitor a Linux client if it is reachable and or didn't freeze.
- If the client froze it should send an email notification.
- If the client froze it should cut its power supply shortly to force a reboot.

# Things needed

- ESP32
- Linux client e.g. Raspberry Pi 5
- TP-Link Tapo P110 (optional but required for full functionality)
- Docker installed on the client
- Working WLAN infrastructure
- Platformio

# Roadmap

- [x] Make a git repository.
- [x] Connect to the ESP32 and check if it works.
- [x] Connect the ESP to the Wi-Fi.
- [x] IP address is reachable form another device
- [x] Make a heartbeat from the client to the ESP
  - [x] Mosquitto docker for the client
  - [x] Heartbeat script for the client
    - [x] Filesystem
    - [x] Internet
    - [x] DNS
    - [x] MQTT
    - [x] Dockerize
  - [x] Heartbeat script for the ESP
- [ ] Make the ESP send an email when the client goes down.
  - [x] Make an ISR that checks the time between the heartbeats
  - [ ] Make it recognize when the system is down
  - [ ] Send email when the system is down
- [ ] Secure the application with a .env file
- [ ] Reverse-engineer the Tapo P110 API that controls the power state
- [ ] When several heartbeats are missed the ESP should switch the power state off and back on again to force a reboot of the client.

- [ ] Make the MQTT connection secure.
- [ ] Make a heartbeat from the ESP to the client
- [ ] Let the client send and email if there is no longer a heartbeat

# Client

# Installation

Copy the Heartbeat folder to the desired destination on the client.
Execute the following command in the folder:

```sh
docker compose up -d
```

This command starts the docker containers that include the MQTT server and the heartbeat script.

## Logs

To view logs from the heartbeat script have a look at `/Heartbeat/data/log.txt`.
For logs of the MQTT server look into `/Heartbeat/mosquitto/log/mosquitto.log`
