/****************************************************************************************/
/***************** This file has file utils for better IO performance. ******************/
/*************** Created By: Kashi Vishwanath Bondugula on June 24 2023 *****************/
/****************************************************************************************/

#ifndef INC_1BRC_ZFILE_H
#define INC_1BRC_ZFILE_H

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "structures.h"

#define BUFSIZE (BUFSIZ * 10)

typedef struct zfile {
    int fd;
    ssize_t cursor;
    unsigned char data[BUFSIZE];
    ssize_t n_data;
} ZFile;

ssize_t init_zfile(ZFile *z_file);

ssize_t zread(ZFile *z_file, unsigned char buff[], ssize_t n_bytes);

#endif //INC_1BRC_ZFILE_H
