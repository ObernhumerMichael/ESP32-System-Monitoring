# ESP32-System-Monitoring

## Goal

This project should achieve the following things:

- Monitor my Raspberry Pi 5 if it is reachable and or didn't freeze.
- If the PI froze it should cut it should switch the smart Tapo P110 off and on to force a restart.
- It should monitor the temperature and air quality of the room in which the raspberry is located.

## Things needed

- ESP32 Devkitc v5
- Raspberry Pi 5
- TP-Link Tapo P110
- Docker installed on the Raspberry Pi
- Router

## Roadmap

- [x] Make a git repository.
- [x] Connect to the ESP32 and check if it works.
- [x] Connect the ESP to the Wi-Fi.
- [x] IP address is reachable form an other device
- [ ] Make a heartbeat of the Raspi to the ESP
  - [x] Mosquitto docker for the Raspi
  - [ ] Hearbeat script for the Raspi
  - [ ] Hearteat script for the ESP
- [ ] Reverse engineer the Tapo P110 API that controls the power state
- [ ] When several heartbeats are missed the ESP should switch the power state off and back on again to force a reboot of the Raspi.

## Raspberry installation

Copy the Raspberry folder to the desired destination on the Raspi.
Execute the following command in the folder:

```sh
docker compose up -d
```

This command starts the docker and the MQTT server.
