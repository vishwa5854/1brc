//
// Created by z on 3/25/24.
//
#include <string.h>
#include "../include/structures.h"

void entry_init(Entry *entry) {
    entry->station_name.start_ptr = NULL;
    entry->station_name.length = 0;

    entry->temperature = 0;
    // initialising with a greater value out of the given range
    entry->min = 200;

    entry->mean = 0;

    // initialising with a lesser value out of the given range
    entry->max = -200;
}
