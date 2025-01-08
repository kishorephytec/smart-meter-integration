from gurux_dlms import GXDLMSTranslator
import sys
import re

def is_valid_hex(data):
    """Check if the input string is a valid hexadecimal representation."""
    cleaned_data = data.replace(" ", "")
    return bool(re.fullmatch(r'^[0-9a-fA-F]*$', cleaned_data)) and len(cleaned_data) % 2 == 0

def format_hex_message(data):
    """Format the continuous hex string into a space-separated string."""
    return ' '.join(data[i:i+2] for i in range(0, len(data), 2))

def decode_message(hex_message):
    """Main function to decode a hexadecimal message."""
    formatted_message = format_hex_message(hex_message)
    if not is_valid_hex(formatted_message):
        return "Error: The provided data is not a valid hexadecimal string."

    # Format the hex message for GXDLMSTranslator
    

    t = GXDLMSTranslator()
    t.comments = True
    t.systemTitle = "qwertyui".encode()
    t.blockCipherKey = bytearray((0x62,) * 16)
    t.authenticationKey = bytearray((0x62,) * 16)
       
    # Convert the message to XML
    try:
        xml_output = t.messageToXml(formatted_message)
        return xml_output
    except Exception as e:
        return f"Error during XML conversion: {str(e)}"

if __name__ == "__main__":
    # Check if the script is run with the required argument
    if len(sys.argv) != 2:
        print("Usage: python dlms.py <hex_data>")
        sys.exit(1)

    message = sys.argv[1]
    decode_message(message)

