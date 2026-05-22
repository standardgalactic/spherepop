/* repl.c — Extended REPL utilities (history, completion stubs).
 * The main REPL loop lives in main.c; this file provides helpers
 * for multi-line input, history recall, and future readline integration.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Multi-line input: accumulate until braces balance */
char *repl_read_balanced(FILE *in, const char *prompt) {
    size_t cap = 4096;
    char  *buf = malloc(cap);
    size_t len = 0;
    int    depth = 0;
    char   line[1024];

    printf("%s", prompt);
    fflush(stdout);

    while (fgets(line, sizeof(line), in)) {
        size_t ll = strlen(line);
        if (len + ll + 1 >= cap) {
            cap *= 2;
            buf = realloc(buf, cap);
        }
        memcpy(buf + len, line, ll);
        len += ll;

        for (size_t i = 0; i < ll; i++) {
            if (line[i] == '{') depth++;
            if (line[i] == '}') depth--;
        }

        if (depth <= 0) break;
        printf("...  ");
        fflush(stdout);
    }
    buf[len] = '\0';
    return buf;
}
