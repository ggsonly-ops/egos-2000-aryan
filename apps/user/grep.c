/*
 * Simple grep for EGOS-2000
 * Usage: grep [PATTERN] [FILE]
 */

#include "app.h"
#include <string.h>

#define LINE_BUF_SIZE 1024   /* big enough for README line */

/* Return 1 if line[0..line_len-1] contains pattern[0..pat_len-1], else 0. */
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

    /* Same as cat: read once, ignore return, treat buffer as a string. */
    char buf[BLOCK_SIZE];
    file_read(file_ino, 0, buf);

    /* Walk through buf, split into lines, and match each line. */
    char line[LINE_BUF_SIZE];
    int line_len = 0;

    for (int i = 0;; i++) {
        char c = buf[i];

        if (c == '\0' || c == '\n') {
            /* end of line or end of file */
            if (line_len >= LINE_BUF_SIZE)
                line_len = LINE_BUF_SIZE - 1;

            line[line_len] = '\0';

            if (contains_pattern(line, line_len, pattern, pat_len))
                printf("%s\n", line);

            line_len = 0;

            if (c == '\0')
                break;
        } else {
            if (line_len < LINE_BUF_SIZE - 1)
                line[line_len++] = c;
        }
    }

    return 0;
}
