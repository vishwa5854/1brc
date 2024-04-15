//
// Created by z on 2/25/24.
//

#ifndef INC_1BRC_STRUCTURES_H
#define INC_1BRC_STRUCTURES_H

#define FAILURE (-1)

#define INPUT_FILE_PATH "/home/z/bs/1brc/measurements.txt"

/* One char extra to hold the terminating null char */
#define MAX_CHARACTERS_IN_STATION_NAME 101

#define MAX_STATION_NAMES 10000

#define ONE_ENTRY_MAX_LENGTH (MAX_CHARACTERS_IN_STATION_NAME + 6)

typedef struct str {
    char *start_ptr;
    int length;
} String;

typedef struct entry {
    String station_name;

    /** non null double between -99.9 (inclusive) and 99.9 (inclusive), always with one fractional digit */
    double temperature;
    double min;
    double mean;
    double max;
    double sum;
    int count;
} Entry;

void entry_init(Entry *entry);

#endif //INC_1BRC_STRUCTURES_H
