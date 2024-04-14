#include <fcntl.h>
#include <stdlib.h>
#include "include/zfile.h"
#include <glib.h>

int last_available_pos = 0;

int get_int(char c) {
    switch (c) {
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case '0': return 0;
        // this should handle the -
        default: return -1;
    }
}

void parse_line(const char *line, int line_length, GHashTable *hash_table, Entry *entry) {
    int start = 0, splitter_visited = 0, period_visited = 0, station_name_pos = 0, temperature_is_negative = 0;
    double temperature = 0, multiplier = 1;

    char station_name[MAX_CHARACTERS_IN_STATION_NAME];
    (void) memset(station_name, '\0', sizeof(station_name));

    while ((start < line_length) && (line[start] != '\0')) {
        char c = line[start++];

        /* End of station name */
        if (c == ';') {
            splitter_visited = 1;
        } else if (c == '.') {
            // we don't want to consider the dots in the station name
            if (splitter_visited) {
                period_visited = 1;
            }
        } else {
            if (splitter_visited) {
                // temperature parsing
                if (get_int(c) != -1) {
                    if (period_visited) {
                        temperature += get_int(c) * 0.1;
                    } else {
                        temperature = temperature * multiplier + get_int(c);
                        multiplier *= 10;
                    }
                } else {
                    temperature_is_negative = 1;
                }
            } else {
                // station name parsing
                station_name[station_name_pos++] = c;
            }
        }
    }

    if (temperature_is_negative) {
        temperature = -temperature;
    }

    Entry *old_value = NULL;
    char *old_key;

    // remember one thing, our key is only until the station_name_pos since the station name var has more bytes
    char station_as_key[station_name_pos];

    strncpy(station_as_key, station_name, station_name_pos);

    if (g_hash_table_lookup_extended(hash_table, station_as_key, (gpointer *) &old_key, (gpointer *) &old_value)) {
//        (void) printf("*** Updating %s;%f;%f;%f ***\n", old_value->station_name, old_value->min, old_value->mean,
//                      old_value->max);

        if (old_value->min > temperature) {
            old_value->min = temperature;
        }

        if (old_value->max < temperature) {
            old_value->max = temperature;
        }

        old_value->sum += temperature;
        old_value->count += 1;
        old_value->mean = old_value->sum / old_value->count;
//        (void) printf("*** Updating %s;%f;%f;%f ***\n", old_value->station_name, old_value->min, old_value->mean,
//                      old_value->max);
    } else {
        strncpy(entry[last_available_pos].station_name, station_name, station_name_pos);
        entry[last_available_pos].temperature = temperature;

        if (entry[last_available_pos].min > temperature) {
            entry[last_available_pos].min = temperature;
        }

        if (entry[last_available_pos].max < temperature) {
            entry[last_available_pos].max = temperature;
        }

        entry[last_available_pos].sum += temperature;
        entry[last_available_pos].count += 1;
        entry[last_available_pos].mean = entry[last_available_pos].sum / entry[last_available_pos].count;

//        (void) printf("*** SUpdating %s;%f;%f;%f, temp is %f ***\n", entry[last_available_pos].station_name,
//                      entry[last_available_pos].min, entry[last_available_pos].mean,
//                      entry[last_available_pos].max, temperature);

        g_hash_table_insert (hash_table, station_as_key, &entry[last_available_pos++]);
    }
}

void printer(Entry *entry) {
    for (int i = 0; i < last_available_pos; i++) {
        (void) printf("%s;%f;%f;%f\n", entry[i].station_name, entry[i].min, entry[i].mean, entry[i].max);
    }
}

int main(void) {
    ZFile z_file;

    if ((z_file.fd = open(INPUT_FILE_PATH, O_RDONLY)) == -1) {
        perror("zcrypt: Error while opening the input file to decrypt");
        exit(FAILURE);
    }

    if (init_zfile(&z_file) < 0) {
        perror("zcrypt: Error while initialising the ZFile for the input file");
        exit(FAILURE);
    }

    Entry entry[MAX_STATION_NAMES];

    /** Initialising all the strings to \0 chars and floats to 0 */
    for (int i = 0; i < MAX_STATION_NAMES; i++) {
        entry_init(&entry[i]);
    }

    /** Before we read let's create a hash table to hold the data. */
    GHashTable *data = g_hash_table_new(g_int_hash, g_int_equal);

    char line[ONE_ENTRY_MAX_LENGTH];
    (void) memset(line, '\0', sizeof(line));

    int wcl = 0;

    unsigned char buff[1];
    buff[0] = '\0';

    while (zread(&z_file, buff, 1) > 0) {
        if (buff[0] == '\n') {
            parse_line(line, wcl, data, entry);
            wcl = 0;
            (void) memset(line, '\0', sizeof(line));
        } else {
            line[wcl++] = (char) buff[0];
        }
    }

    (void) close(z_file.fd);
    printer(entry);
    return EXIT_SUCCESS;
}
