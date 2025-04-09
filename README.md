# smart-meter-integration
This repository integrates smart meters with RF mesh and PLC communication technologies. It includes a UART daemon, Wirepas and Wi-SUN, enabling efficient, low-power, and scalable data transmission over mesh networks and power lines for smart meter applications.

For PLC Gateway Setup on linux system install below packages:
*************************************************************
apt update
apt-get install build-essential
apt install libxml2-dev
apt install pkg-config
apt install -y mosquitto mosquitto-clients
apt install libdbus-1-dev
apt install libmosquitto-dev
apt install python3-dbus
apt install python3-pip
apt install sqlite3 libsqlite3-dev
apt-get install libjson-c-dev
apt-get install libsqlite3-dev

For Custom FG23 Gateway Setup on linux system install below packages:
*********************************************************************
apt update
apt-get install build-essential
apt install libxml2-dev
apt install pkg-config
apt install -y mosquitto mosquitto-clients
apt install libdbus-1-dev
apt install libmosquitto-dev
apt install python3-dbus
apt install python3-pip
pip install gurux-common --br
pip install gurux-serial --br
pip install gurux-net --br
pip install gurux-dlms --br

