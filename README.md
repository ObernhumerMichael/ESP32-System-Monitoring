# ESP32-System-Monitoring

## Goal

This project should achieve the following things:

- Monitor a Linux client if it is reachable and or didn't freeze.
- If the client froze it should send an email notification.
- If the client froze it should cut its power supply shortly to force a reboot.

## Things needed

- ESP32 Devkitc v5
- Linux client e.g. Raspberry Pi 5
- TP-Link Tapo P110
- Docker installed on the Raspberry Pi
- Working WLAN infrastructure
- Platformio

## Roadmap

- [x] Make a git repository.
- [x] Connect to the ESP32 and check if it works.
- [x] Connect the ESP to the Wi-Fi.
- [x] IP address is reachable form another device
- [ ] Make a heartbeat from the client to the ESP
  - [x] Mosquitto docker for the client
  - [x] Heartbeat script for the client
    - [x] Filesystem
    - [x] Internet
    - [x] DNS
    - [x] MQTT
    - [x] Dockerize
  - [ ] Heartbeat script for the ESP
- [ ] Make the ESP send an email when the client goes down.
- [ ] Reverse-engineer the Tapo P110 API that controls the power state
- [ ] When several heartbeats are missed the ESP should switch the power state off and back on again to force a reboot of the client.
- [ ] Make the MQTT connection secure.

## Client installation

Copy the Heartbeat folder to the desired destination on the client.
Execute the following command in the folder:

```sh
docker compose up -d
```

This command starts the docker and the MQTT server.
