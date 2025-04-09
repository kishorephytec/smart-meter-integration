# smart-meter-integration
This repository integrates smart meters with RF mesh and PLC communication technologies. It includes a UART daemon, Wirepas and Wi-SUN, enabling efficient, low-power, and scalable data transmission over mesh networks and power lines for smart meter applications.

For PLC Gateway Setup on linux system install below packages:
**************************************************************
1.apt update
2.apt-get install build-essential
3.apt install libxml2-dev
4.apt install pkg-config
5.apt install -y mosquitto mosquitto-clients
6.apt install libdbus-1-dev
7.apt install libmosquitto-dev
8.apt install python3-dbus
9.apt install python3-pip
10.apt install sqlite3 libsqlite3-dev
11.apt-get install libjson-c-dev
12.apt-get install libsqlite3-dev

For Custom FG23 Gateway Setup on linux system install below packages:
*********************************************************************
1.apt update
2.apt-get install build-essential
3.apt install libxml2-dev
4.apt install pkg-config
5.apt install -y mosquitto mosquitto-clients
6.apt install libdbus-1-dev
7.apt install libmosquitto-dev
8.apt install python3-dbus
9.apt install python3-pip
10.pip install gurux-common --br
11.pip install gurux-serial --br
12.pip install gurux-net --br
13.pip install gurux-dlms --br
