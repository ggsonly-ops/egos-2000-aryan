/*
 * Simple wcl (wc -l) for EGOS-2000
 * Usage: wcl [FILE1] [FILE2] ...
 */

#include "app.h"
#include <string.h>

static int count_lines_in_file(int file_ino) {
    char buf[BLOCK_SIZE];
    int total_lines = 0;
    int seen_char_since_newline = 0;

    /* EGOS file_read: return value is not the byte count.
       It fills buf and returns 0 on success (like in cat.c).
       So we ignore the return value and treat buf as a C string. */
    file_read(file_ino, 0, buf);

    for (int i = 0; buf[i] != '\0'; i++) {
        char c = buf[i];
        if (c == '\n') {
            total_lines++;
            seen_char_since_newline = 0;
        } else {
            seen_char_since_newline = 1;
        }
    }

    if (seen_char_since_newline)
        total_lines++;

    return total_lines;
}

int main(int argc, char **argv) {
    if (argc < 2) {
        INFO("usage: wcl [FILE1] [FILE2] ...");
        return -1;
    }

    int grand_total = 0;

    for (int i = 1; i < argc; i++) {
        int file_ino = dir_lookup(workdir_ino, argv[i]);
        if (file_ino < 0) {
            INFO("wcl: file %s not found", argv[i]);
            continue;
        }

        grand_total += count_lines_in_file(file_ino);
    }

    printf("%d\n", grand_total);
    return 0;
}
