/*
 * Simple grep for EGOS-2000
 * Usage: grep [PATTERN] [FILE]
 */

#include "app.h"
#include <string.h>

#define LINE_BUF_SIZE 1024

/* Return 1 if line contains pattern, else 0 */
static int contains_pattern(const char *line, int line_len,
                            const char *pattern, int pat_len) {
    if (pat_len == 0) return 1;
    if (line_len < pat_len) return 0;

    for (int i = 0; i + pat_len <= line_len; i++) {
        int j = 0;
        while (j < pat_len && line[i + j] == pattern[j]) {
            j++;
        }
        if (j == pat_len) {
            return 1;
        }
    }
    return 0;
}

/* Process buffer and print matching lines */
static void process_buffer(char *buf, char *line, int *line_len, 
                          const char *pattern, int pat_len) {
    for (int i = 0; i < BLOCK_SIZE && buf[i] != '\0'; i++) {
        char c = buf[i];

        if (c == '\n') {
            /* End of line */
            if (*line_len >= LINE_BUF_SIZE)
                *line_len = LINE_BUF_SIZE - 1;

            line[*line_len] = '\0';

            if (contains_pattern(line, *line_len, pattern, pat_len))
                printf("%s\n", line);

            *line_len = 0;
        } else {
            if (*line_len < LINE_BUF_SIZE - 1)
                line[(*line_len)++] = c;
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        INFO("usage: grep [PATTERN] [FILE]");
        return -1;
    }

    char *pattern = argv[1];
    char *filename = argv[2];
    int pat_len = (int)strlen(pattern);

    int file_ino = dir_lookup(workdir_ino, filename);
    if (file_ino < 0) {
        INFO("grep: file %s not found", filename);
        return -1;
    }

    /* Read entire file block by block */
    char buf[BLOCK_SIZE];
    char line[LINE_BUF_SIZE];
    int line_len = 0;
    int offset = 0;

    while (1) {
        /* Clear buffer before reading */
        for (int i = 0; i < BLOCK_SIZE; i++) {
            buf[i] = '\0';
        }
        
        file_read(file_ino, offset, buf);
        
        /* Check if this block is empty (end of file) */
        if (buf[0] == '\0') {
            break;
        }
        
        /* Process this block */
        process_buffer(buf, line, &line_len, pattern, pat_len);
        
        /* Check if we read a partial block (indicates end of file) */
        int partial_block = 0;
        for (int i = 0; i < BLOCK_SIZE; i++) {
            if (buf[i] == '\0') {
                partial_block = 1;
                break;
            }
        }
        
        if (partial_block) {
            /* Handle last line if it doesn't end with newline */
            if (line_len > 0) {
                line[line_len] = '\0';
                if (contains_pattern(line, line_len, pattern, pat_len))
                    printf("%s\n", line);
            }
            break;
        }
        
        /* Move to next block */
        offset += BLOCK_SIZE;
    }

    return 0;
}