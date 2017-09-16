#include <stdio.h>
#include <string.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
    if (argc < 4 || strcmp(argv[2], "-o"))
    {
        fprintf(stderr, "%s <in> -o <out>\n", argv[0]);
        return 2;
    }

    FILE *w = fopen(argv[3], "w");
    if (!w)
    {
        fprintf(stderr, "failed to open %s\n", argv[3]);
        return 3;
    }

    FILE *r = fopen(argv[1], "rb");
    if (!r)
    {
        fprintf(stderr, "failed to open %s\n", argv[1]);
        return 4;
    }

    fprintf(w, "extern unsigned long g_datasz;\n");
    fprintf(w, "extern const char g_data[];\n");
    fprintf(w, "const char g_data[] = \"\"");

    unsigned totalsz = 0;
    uint8_t buf[16];
    while (!feof(r))
    {
        int sz = fread(buf, 1, sizeof(buf), r);
        if (sz <= 0)
        {
            break;
        }

        fprintf(w, "\n\"");
        for (int i = 0; i < sz; i++)
        {
            fprintf(w, "\\x%02x", buf[i]);
        }
        fprintf(w, "\"");
        totalsz += sz;

        fprintf(w, "/* ");
        for (int i = 0; i < sz; i++) {
            fprintf(w, "%c", (' ' <= buf[i] && buf[i] < 0x7F) ? buf[i] : '.');
        }
        fprintf(w, " */");
    }
    fprintf(w, ";\n\n");
    fprintf(w, "unsigned long g_datasz = %u;\n", totalsz);

    fclose(w);
    fclose(r);
    return 0;
}