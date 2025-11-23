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
    int offset = 0;

    /* Read all blocks of the file */
    while (1) {
        /* Clear buffer before reading */
        for (int i = 0; i < BLOCK_SIZE; i++) {
            buf[i] = '\0';
        }
        
        file_read(file_ino, offset, buf);
        
        /* Check if this block is empty (end of file) */
        int block_empty = 1;
        for (int i = 0; i < BLOCK_SIZE; i++) {
            if (buf[i] != '\0') {
                block_empty = 0;
                break;
            }
        }
        
        if (block_empty) {
            break;  /* End of file reached */
        }
        
        /* Count lines in this block */
        for (int i = 0; i < BLOCK_SIZE && buf[i] != '\0'; i++) {
            char c = buf[i];
            if (c == '\n') {
                total_lines++;
                seen_char_since_newline = 0;
            } else {
                seen_char_since_newline = 1;
            }
        }
        
        /* Check if we read a partial block (indicates end of file) */
        int partial_block = 0;
        for (int i = 0; i < BLOCK_SIZE; i++) {
            if (buf[i] == '\0') {
                partial_block = 1;
                break;
            }
        }
        
        if (partial_block) {
            break;  /* This was the last block */
        }
        
        /* Move to next block */
        offset += BLOCK_SIZE;
    }

    /* If file ends without newline but has content, count it as a line */
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