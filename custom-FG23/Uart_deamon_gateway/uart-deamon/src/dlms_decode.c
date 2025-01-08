#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <time.h>
#include <timestamp.h>
#include <dbus/dbus.h>

//DLMS packet translation
char* call_decode_message(const char *hex_message, int len) {
    
    //timestamp
	char timestamp[30];
	get_timestamp(timestamp);

    DBusError err;
    DBusConnection *conn;
    DBusMessage *msg;
    DBusMessageIter args;
    DBusPendingCall *pending;
    char *result;

    // Initialize the error
    dbus_error_init(&err);

    // Connect to the system bus
    conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
    if (dbus_error_is_set(&err)) {
        fprintf(stderr, "%s [ERROR][DBUS          ]:Connection Error: %s\n",timestamp,err.message);
        dbus_error_free(&err);
    }
    if (!conn) exit(1);

    // Create a new method call message
    msg = dbus_message_new_method_call("com.example.DecodeService",  // Target service name
                                       "/com/example/DecodeService", // Object to call on
                                       "com.example.DecodeServiceInterface", // Interface name
                                       "decode_message");           // Method name
    if (!msg) {
        fprintf(stderr, "%s [ERROR][DBUS          ]:Message Null\n",timestamp);
        exit(1);
    }

    // Create a hex buffer to store the hexadecimal representation   
    int hex_buffer_size = (len * 2) + 1;  // Each byte to "XX " format + null terminator
    char *hex_buffer= (char*)malloc(hex_buffer_size * sizeof(char));
    if (!hex_buffer) {
    fprintf(stderr, "%s [ERROR][DBUS          ]:Memory allocation failed\n",timestamp);
    exit(1);
        }

    // Ensure the buffer is large enough
    for (int i = 0; i < len; i++) {
        // Convert each byte to two hex digits and append to the buffer
        snprintf(hex_buffer + (i * 2), 3, "%02X ", (unsigned char)hex_message[i]);
    }

    hex_buffer[hex_buffer_size-1]='\0';
    // Print the hex buffer for debugging
    //  printf("Hex Buffer: %s\n", hex_buffer);

    // Append arguments
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &hex_buffer)) {
        fprintf(stderr, "%s [ERROR][DBUS          ]:Out of Memory!\n",timestamp);
        exit(1);
    }

    // Send message and get a handle for a reply
    if (!dbus_connection_send_with_reply(conn, msg, &pending, -1)) {
        fprintf(stderr, "%s [ERROR][DBUS          ]:Out of Memory!\n",timestamp);
        exit(1);
    }
    if (!pending) {
        fprintf(stderr, "%s [ERROR][DBUS          ]:Pending Call Null\n",timestamp);
        exit(1);
    }
    dbus_connection_flush(conn);

    //printf("Request Sent\n");

    // Free message
    dbus_message_unref(msg);
    free(hex_buffer);

    // Block until we receive a reply
    dbus_pending_call_block(pending);

    // Get the reply message
    msg = dbus_pending_call_steal_reply(pending);
    if (!msg) {
        fprintf(stderr, "%s [ERROR][DBUS          ]:Reply Null\n",timestamp);
        exit(1);
    }

    // Free the pending message handle
    dbus_pending_call_unref(pending);

    // Read the parameters
    if (!dbus_message_iter_init(msg, &args))
        fprintf(stderr, "%s [ERROR][DBUS          ]:Message has no arguments!\n",timestamp);
    else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args))
        fprintf(stderr, "%s [ERROR][DBUS          ]:Argument is not string!\n",timestamp);
    else
        dbus_message_iter_get_basic(&args, &result);

    #if DEBUG
    printf("Decoded Result: %s\n", result);
    #endif

    // Free reply and close the connection   
    dbus_message_unref(msg);
    dbus_connection_unref(conn);
    return result;
}
