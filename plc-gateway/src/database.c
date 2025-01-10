/*
 * database.c
 *
 *  Created on: 16-Dec-2024
 *      Author: Srikanth Jogi
 */

#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include "database.h"
#include <time.h>
#include <timestamp.h>
#include "stdlib.h"
#include <ctype.h>
// Function to initialize the database and create the table if it doesn't exist
int initialize_database() {
    sqlite3 *db;
    char *err_msg = NULL;

    char timestamp[30];
    get_timestamp(timestamp);
    printf("%s [INFO][DATABASE-INIT]: Initialized database\n\r", timestamp);
    
    int rc = sqlite3_open("NC_data_store.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: Cannot open NC database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1; // Return -1 for failure
    }

    // SQL query to create the table
    const char *sql = "CREATE TABLE IF NOT EXISTS NC_data_info ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "type TEXT NOT NULL,"
                      "value TEXT NOT NULL,"
		      "timestamp TEXT NOT NULL);";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
	sqlite3_close(db);
	return -1; // Return -1 for SQL error
    }

     // Clear the table to overwrite old data
    const char *sql_clear = "DELETE FROM NC_data_info;";
    rc = sqlite3_exec(db, sql_clear, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: Failed to clear table: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return -1; // Return -1 for failure
    }
    // Reset the AUTOINCREMENT sequence
    const char *sql_reset_sequence = "DELETE FROM SQLITE_SEQUENCE WHERE name='NC_data_info';";
    rc = sqlite3_exec(db, sql_reset_sequence, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: Failed to reset sequence: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return -1;
    }

    printf("%s [DATABASE][INFO]: Cleared old data from 'NC_data_info' table.\n", timestamp);
    printf("[DATABASE][INFO]: Table 'NC_data_info' is ready.\n");
    sqlite3_close(db);
    return 0; // Return 0 for success
}

//network sync table initialize
int initialize_network_syncDB() {
    sqlite3 *db;
    char *err_msg = NULL;

    // Check if the database file exists
    FILE *file = fopen("Network_sync_data.db", "r");
    if (file) {
        fclose(file);
        // Database file exists, check if the table exists
        int rc = sqlite3_open("Network_sync_data.db", &db);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "[DATABASE][ERROR]: Cannot open Network sync database: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return -1; // Return -1 for failure
        }

        const char *check_table_query =
            "SELECT name FROM sqlite_master WHERE type='table' AND name='Network_sync_info';";

        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(db, check_table_query, -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "[DATABASE][ERROR]: Failed to check table existence: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return -1;
        }

        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);

        if (rc == SQLITE_ROW) {
            // Table exists, no need to initialize
            printf("[DATABASE][INFO]: Database and table already initialized.\n");
            return 0;
        }
    }

    // Initialize the database and create the table
    char timestamp[30];
    get_timestamp(timestamp);
    printf("%s [INFO][DATABASE-INIT]: Initializing database\n\r", timestamp);

    int rc = sqlite3_open("Network_sync_data.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: Cannot open Network sync database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return -1; // Return -1 for failure
    }

    // SQL query to create the table
    const char *sql = "CREATE TABLE IF NOT EXISTS Network_sync_info ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "node_id TEXT NOT NULL,"
                      "parent_id TEXT NOT NULL,"
                      "modem_serial_number TEXT NOT NULL,"
                      "connectivity_status TEXT NOT NULL,"
                      "timestamp TEXT NOT NULL,"
                      "meter_serial_number TEXT," // Add this new column
                      "UNIQUE(node_id, parent_id, modem_serial_number, connectivity_status));";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return -1; // Return -1 for SQL error
    }

    printf("[DATABASE][INFO]: Table 'Network_sync_info' is ready.\n");
    sqlite3_close(db);
    return 0; // Return 0 for success
}

//fetching node id
char* fetch_node_id(unsigned char* system, unsigned short system_len) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    char *node_id = NULL;

    char system_hex[system_len * 2 + 1]; // Buffer for hex string

    // Print the binary data for debugging
    printf("[DATABASE][DEBUG]: system data is: ");
    for (int i = 0; i < system_len; i++) {
        printf("%02X ", system[i]);
    }
    printf("\n");

    if (system_len == 2) {
        // Convert the binary data to a hexadecimal string
        for (int i = 0; i < system_len; i++) {
            sprintf(&system_hex[i * 2], "%02X", system[i]);
        }
        system_hex[system_len * 2] = '\0'; // Null-terminate the string
        printf("[DATABASE][DEBUG]: system as hex string: %s\n", system_hex);
    }

    // Open the database
    rc = sqlite3_open("Network_sync_data.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: Cannot open database: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    // SQL query to fetch node_id
    const char *sql = "SELECT node_id FROM Network_sync_info WHERE meter_serial_number = ? OR node_id = ?;";

    // Prepare the SQL statement
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    // Bind system or meter_serial_number values
    if (system_len == 2) {
        rc = sqlite3_bind_text(stmt, 2, system_hex, -1, SQLITE_STATIC);
    } else if (system_len > 2) {
        rc = sqlite3_bind_text(stmt, 1, (const char*)system, -1, SQLITE_STATIC);
    }
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: Failed to bind system value: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return NULL;
    }

    // Execute the query and fetch the result
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *serial = sqlite3_column_text(stmt, 0);
        if (serial) {
            node_id = malloc(strlen((const char*)serial) + 1);
            if (node_id) {
                strcpy(node_id, (const char*)serial);
            } else {
                fprintf(stderr, "[DATABASE][ERROR]: Memory allocation failed.\n");
            }
        }
    } else {
        printf("[DATABASE][INFO]: No matching data found for provided system or meter_serial_number.\n");
    }

    // Finalize the statement and close the database
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return node_id; // Return the fetched node_id
}

// Function to fetch meter_serial_number from the database
char* fetch_meter_id(const char* global_system, unsigned short system_len) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    char *meter_id = NULL;

    // Open the database
    rc = sqlite3_open("Network_sync_data.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: Cannot open database: %s\n", sqlite3_errmsg(db));
        return NULL;
    }

    // SQL query to fetch the meter_serial_number
    const char *sql = "SELECT meter_serial_number FROM Network_sync_info WHERE meter_serial_number = ?;";

    // Prepare the SQL statement
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL;
    }

    // Bind the global_system to the SQL statement
    rc = sqlite3_bind_text(stmt, 1, global_system, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: Failed to bind system value: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return NULL;
    }

    // Execute the query and fetch the result
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *serial = sqlite3_column_text(stmt, 0);
        if (serial) {
            meter_id = malloc(strlen((const char*)serial) + 1);
            if (meter_id) {
                strcpy(meter_id, (const char*)serial);
            } else {
                fprintf(stderr, "[DATABASE][ERROR]: Memory allocation failed.\n");
            }
        }
    } else {
        printf("[DATABASE][INFO]: No matching data found for meter_serial_number: %s\n", global_system);
    }

    // Finalize the statement and close the database
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return meter_id; // Return the fetched meter_serial_number or NULL
}


//Fetch Modem serial number
char* fetch_modem_number() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;
    char *modem_serial_number = NULL;

    // Open the database
    rc = sqlite3_open("Network_sync_data.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL; // Return NULL on error
    }

    // SQL query to fetch modem_serial_number
    const char *sql = "SELECT modem_serial_number FROM Network_sync_info WHERE id = 1;";

    // Prepare the SQL statement
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return NULL; // Return NULL on error
    }

    // Execute the query and fetch the result
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *serial = sqlite3_column_text(stmt, 0);
        if (serial) {
            modem_serial_number = malloc(strlen((const char*)serial) + 1);
            if (modem_serial_number) {
                strcpy(modem_serial_number, (const char*)serial);
            } else {
                fprintf(stderr, "[DATABASE][ERROR]: Memory allocation failed.\n");
            }
        }
    } else {
        printf("[DATABASE][INFO]: No data found.\n");
    }

    // Finalize the statement and close the database
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return modem_serial_number; // Return the fetched serial number
}


void store_network_sync_info(const char *node_id, const char *parent_id, const char *serial_number, const char *connectivity_status) {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    // Open the database
    rc = sqlite3_open("Network_sync_data.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: Cannot open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Check for existing data (exclude timestamp from duplicate check)
    const char *sql_check =
        "SELECT COUNT(*) FROM Network_sync_info WHERE "
        "node_id = ? AND parent_id = ? AND modem_serial_number = ? AND connectivity_status = ?;";

    rc = sqlite3_prepare_v2(db, sql_check, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Bind parameters to the query
    sqlite3_bind_text(stmt, 1, node_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, parent_id, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, serial_number, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, connectivity_status, -1, SQLITE_STATIC);

    // Execute the query
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        fprintf(stderr, "[DATABASE][ERROR]: Failed to execute query: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return;
    }

    // Get the count of matching records
    int count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    if (count > 0) {
        // Record exists, so we perform an UPDATE
        printf("[DATABASE][INFO]: Node ID '%s' exists. Perform update.\n", node_id);

        const char *sql_update =
            "UPDATE Network_sync_info SET parent_id = ?, modem_serial_number = ?, connectivity_status = ? "
            "WHERE node_id = ?;";

        rc = sqlite3_prepare_v2(db, sql_update, -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "[DATABASE][ERROR]: Failed to prepare update statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        // Bind parameters to the update query
        sqlite3_bind_text(stmt, 1, parent_id, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, serial_number, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, connectivity_status, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, node_id, -1, SQLITE_STATIC);

        // Execute the update query
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            fprintf(stderr, "[DATABASE][ERROR]: Failed to update data: %s\n", sqlite3_errmsg(db));
        } else {
            printf("[DATABASE][INFO]: Successfully updated Node ID: '%s'.\n", node_id);
        }

    } else {
        // Insert new data if no duplicates
        printf("[DATABASE][INFO]: No existing record found. Inserting new record for Node ID '%s'.\n", node_id);

        char timestamp[30];
        get_timestamp(timestamp);

        const char *sql_insert =
            "INSERT INTO Network_sync_info (node_id, parent_id, modem_serial_number, connectivity_status, timestamp) "
            "VALUES (?, ?, ?, ?, ?);";

        rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            fprintf(stderr, "[DATABASE][ERROR]: Failed to prepare insert statement: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return;
        }

        // Bind parameters to the insert query
        sqlite3_bind_text(stmt, 1, node_id, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, parent_id, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, serial_number, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, connectivity_status, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 5, timestamp, -1, SQLITE_STATIC);

        // Execute the insert query
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            fprintf(stderr, "[DATABASE][ERROR]: Failed to insert data: %s\n", sqlite3_errmsg(db));
        } else {
            printf("[DATABASE][INFO]: Successfully inserted Node ID: '%s', Parent ID: '%s', Modem Serial Number: '%s', Connectivity Status: '%s'.\n",
                   node_id, parent_id, serial_number, connectivity_status);
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

// General function to store NC dataBase configurations
void store_data(const char *type, const char *value) {
    sqlite3 *db;
    char *err_msg = NULL;

    char timestamp[30];
    get_timestamp(timestamp);

    int rc = sqlite3_open("NC_data_store.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Prepare the SQL query to insert the data
    char sql[256];
    snprintf(sql, sizeof(sql), "INSERT INTO NC_data_info  (type, value,timestamp) VALUES ('%s', '%s', '%s');", type, value, timestamp);

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    } else {
        printf("[DATABASE][INFO]: Successfully stored '%s' with value '%s'.\n", type, value);
    }

    sqlite3_close(db);
}


void binary_to_hex(const char *input, int length, char *output) {
    for (int i = 0; i < length; i++) {
        sprintf(output + (i * 2), "%02X", (unsigned char)input[i]);
    }
    output[length * 2] = '\0';  // Null-terminate the string
}


void clear_network_db(){

    sqlite3 *db;
    char *err_msg = NULL;
    int rc = sqlite3_open("Network_sync_data.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: Cannot open Network_sync_data database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    const char *sql_clear = "DELETE FROM Network_sync_info;";
    rc = sqlite3_exec(db, sql_clear, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "[DATABASE][ERROR]: Failed to clear table: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return ; // Return  for failure
    }
	


}


// Specialized functions to store specific types of data
void store_network_size(int network_size) {
    char value[32];
    snprintf(value, sizeof(value), "%d", network_size);
    store_data("Network Size", value);
}

void store_network_id(const char *network_id) {
    store_data("Network ID", network_id);
}

void store_node_id(const char *node_id) {
    store_data("Node ID", node_id);
}

void store_serial_number(const char *serial_number) {
    char hex_serial[33];  // 16 bytes * 2 characters per byte + 1 for null terminator
    binary_to_hex(serial_number, 16, hex_serial);
    store_data("Serial Number", hex_serial);

}

void store_nc_database_size(int nc_db_size) {
    char value[32];
    snprintf(value, sizeof(value), "%d", nc_db_size);
    store_data("NC Database Size", value);
}
