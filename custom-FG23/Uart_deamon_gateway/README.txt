/*********************************************************/
DBUS Service
1. Copy DecodeService.conf file in /etc/dbus-1/system.d/ 
2. Need to run dbus_service.py file before uart daemon.

***Can also written as service file

/********************************************************/
Go inside the folder uart-deamon/src.
Run the following commands to generate binary.
1. make clean
2. make

A binary named as "uart_daemon" is generated.

Run the binary as this:
./uart_daemon /dev/ttyXYZ 115200

Note:  /dev/ttyXYZ  here XYZ is your's UART Port
*****
