# dbus_service.py
import dbus
import dbus.service
import dbus.mainloop.glib
from gi.repository import GLib
import xml.etree.ElementTree as ET
from gurux_dlms import GXDLMSTranslator
import re

class DecodeService(dbus.service.Object):
    def __init__(self, bus_name, object_path='/com/example/DecodeService'):
        super().__init__(bus_name, object_path)

    @dbus.service.method('com.example.DecodeServiceInterface', in_signature='s', out_signature='s')
    def decode_message(self, hex_message):
        """Decode a hexadecimal DLMS message and return the specific value."""
        print(f"Recieved Hex message: {hex_message}")
        #formatted_message = ' '.join(hex_message[i:i+2] for i in range(0, len(hex_message), 2))
        #print(f"Converted Hex Message: {formatted_message}")  # Log the converted message

        def is_valid_hex(data):
            return bool(re.fullmatch(r'^[0-9a-fA-F]*$', data)) and len(data) % 2 == 0

        def format_hex_message(data):
            return ' '.join(data[i:i+2] for i in range(0, len(data), 2))

        # Validate and format hex message
        if not is_valid_hex(hex_message):
            return "Error: Invalid hexadecimal string."
        
        #formatted_message = format_hex_message(hex_message)
        t = GXDLMSTranslator()
        t.comments = True
        xml_output = t.messageToXml(hex_message)

        # Parse XML to find the specific value
        root = ET.fromstring(xml_output)
        specific_value = root.find(".//String").get("Value")
        return specific_value or "No value found"

def main():
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    session_bus = dbus.SystemBus()
    bus_name = dbus.service.BusName('com.example.DecodeService', bus=session_bus)
    service = DecodeService(bus_name)
    loop = GLib.MainLoop()
    loop.run()

if __name__ == '__main__':
    main()
