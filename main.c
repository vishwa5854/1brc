#include "include/zfile.h"
#include <fcntl.h>
#include <glib.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

int last_available_pos = 0;

int
get_int(char c) {
    switch (c) {
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
        case '0':
            return 0;
            // this should handle the -
        default:
            return -1;
    }
}

void
parse_line(String line, GHashTable* hash_table, Entry* entry) {
    if (line.start_ptr == NULL) {
        (void) printf("line.start_ptr is NULL here\n");
        return;
    }

    int start = 0, splitter_visited = 0, period_visited = 0,
            temperature_is_negative = 0;
    double temperature = 0, multiplier = 1;

    // replace the separator with a \0 char so that the string parser thinks it is only until then
    String station_name = { .start_ptr = NULL, .length = 0 };

    while ((start < line.length) && (*(line.start_ptr + start) != '\0')) {
        char c = line.start_ptr[start++];

        /* End of station name */
        if (c == ';') {
            splitter_visited = 1;
            /* Putting a null char so that the key will be parsed until here */
            line.start_ptr[start - 1] = '\0';
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
                if (station_name.length == 0) {
                    station_name.start_ptr = line.start_ptr;
                }
                station_name.length++;
                // station name parsing
            }
        }
    }

    if (temperature_is_negative) {
        temperature = -temperature;
    }

    Entry* old_value = NULL;
    char* old_key;

    // remember one thing, our key is only until the station_name_pos since the station name var has
    // more bytes
//    char station_as_key[station_name_pos];
//
//    strncpy(station_as_key, station_name, station_name_pos);

    if (g_hash_table_lookup_extended(
            hash_table, station_name.start_ptr, (gpointer*)&old_key, (gpointer*)&old_value)) {
        if (old_value->min > temperature) {
            old_value->min = temperature;
        }

        if (old_value->max < temperature) {
            old_value->max = temperature;
        }

        old_value->sum += temperature;
        old_value->count += 1;
        old_value->mean = old_value->sum / old_value->count;
        //        (void) printf("*** Updating %s;%f;%f;%f ***\n", old_value->station_name,
        //        old_value->min, old_value->mean,
        //                      old_value->max);
    } else {
        entry[last_available_pos].station_name.start_ptr = station_name.start_ptr;
        entry[last_available_pos].station_name.length = station_name.length;

        entry[last_available_pos].temperature = temperature;

        if (entry[last_available_pos].min > temperature) {
            entry[last_available_pos].min = temperature;
        }

        if (entry[last_available_pos].max < temperature) {
            entry[last_available_pos].max = temperature;
        }

        entry[last_available_pos].sum += temperature;
        entry[last_available_pos].count += 1;
        entry[last_available_pos].mean =
                (entry[last_available_pos].sum / entry[last_available_pos].count);

        g_hash_table_insert(hash_table, station_name.start_ptr, &entry[last_available_pos++]);
    }
}

void
printer(Entry* entry) {
    for (int i = 0; i < last_available_pos; i++) {
        (void)printf("%s;%f;%f;%f\n", entry[i].station_name.start_ptr, entry[i].min, entry[i].mean, entry[i].max);
    }
}

int
main(void) {
    int fd;
    off_t pos = 0;

    if ((fd = open(INPUT_FILE_PATH, O_RDONLY)) == -1) {
        perror("zcrypt: Error while opening the input file to decrypt");
        exit(FAILURE);
    }

    struct stat file_stat;

    if (fstat(fd, &file_stat) == -1) {
        perror("Error while trying to stat the input file.");
        (void)close(fd);
        return EXIT_FAILURE;
    }

    void* file_in_memory = mmap(NULL, file_stat.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

    if (file_in_memory == MAP_FAILED) {
        perror("Error while mmap file to memory");
        (void)close(fd);
        return EXIT_FAILURE;
    }

    Entry entry[MAX_STATION_NAMES];

    for (int i = 0; i < MAX_STATION_NAMES; i++) {
        entry_init(&entry[i]);
    }

    /** Before we read let's create a hash table to hold the data. */
    GHashTable* data = g_hash_table_new(g_int_hash, g_int_equal);
    String line = { .start_ptr = NULL, .length = 0 };

    while (pos < file_stat.st_size) {
        char *c = ((char *)file_in_memory + pos);

        if (*c == '\n') {
            parse_line(line, data, entry);
            line.start_ptr = NULL;
            line.length = 0;
        } else {
            if (line.length == 0) {
                line.start_ptr = c;
            }
            line.length++;
        }
        pos++;
    }
    printer(entry);

    if (munmap(file_in_memory, file_stat.st_size) == -1) {
        perror("Error while munmap");
        (void)close(fd);
        return EXIT_FAILURE;
    }
    (void)close(fd);
    return EXIT_SUCCESS;
}
