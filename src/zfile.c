/****************************************************************************************/
/***************** This file has file utils for better IO performance. ******************/
/*************** Created By: Kashi Vishwanath Bondugula on June 24 2023 *****************/
/****************************************************************************************/

#include "../include/zfile.h"

ssize_t zread(ZFile *z_file, unsigned char buff[], ssize_t n_bytes) {
    ssize_t n_read;
    ssize_t available = z_file->n_data - z_file->cursor;
    ssize_t remaining = 0;

    if (n_bytes > available) {
        remaining = n_bytes - available;
    }

    if (remaining == 0) {
        (void) memcpy(buff, z_file->data + z_file->cursor, n_bytes);
        z_file->cursor += n_bytes;
    } else {
        (void) memcpy(buff, z_file->data + z_file->cursor, available);

        /** Resetting the buffer to be available for further use. */
        (void) memset(z_file->data, '\0', BUFSIZE);
        z_file->cursor = 0;

        if ((n_read = read(z_file->fd, z_file->data, BUFSIZE)) == -1) {
            perror("zcrypt: zfile: Error while reading data from the file.");
            return FAILURE;
        }
        z_file->n_data = n_read;

        if (n_read == 0) return available;

        (void) memcpy(buff + available, z_file->data, remaining);
        z_file->cursor = remaining;
    }
    return n_bytes;
}

ssize_t init_zfile(ZFile *z_file) {
    (void) memset(z_file->data, '\0', BUFSIZE);
    z_file->cursor = 0;
    ssize_t n_bytes;

    if ((n_bytes = read(z_file->fd, z_file->data, BUFSIZE)) == -1) {
        perror("zcrypt: Failed to initialise ZFile");
        return FAILURE;
    }
    z_file->n_data = n_bytes;
    return n_bytes;
}
