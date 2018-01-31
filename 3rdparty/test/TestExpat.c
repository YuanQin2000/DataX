#include <stdio.h>
#include <assert.h>

#include "expat/expat.h"

#ifdef XML_LARGE_SIZE
#if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#define XML_FMT_INT_MOD "I64"
#else
#define XML_FMT_INT_MOD "ll"
#endif
#else
#define XML_FMT_INT_MOD "l"
#endif

#define BUFFSIZE        8192

char Buff[BUFFSIZE];

int Depth;

static void XMLCALL
start(void *data, const char *el, const char **attr)
{
  int i;

    printf("start <IN>\n");
    for (i = 0; i < Depth; i++)
        printf("  ");

    printf("%s", el);

    for (i = 0; attr[i]; i += 2) {
        printf(" %s='%s'", attr[i], attr[i + 1]);
    }

    printf("\n");
    Depth++;
    printf("start <OUT>\n");
}

static void XMLCALL
end(void *data, const char *el)
{
    printf("end <IN>\n");
    Depth--;
    printf("end <OUT>\n");
}

static void XMLCALL
CharacterHandler(void *pData, const char *pBuf, int len)
{
    int i;
    printf("CharacterHandler <IN>\n");
    for (i = 0; i < len; i++) {
        printf("%d ", pBuf[i]);
    }
    printf("\nCharacterHandler <OUT>\n");
}

int
main(int argc, char *argv[])
{
    assert(argc > 1);

    FILE* pFile = fopen(argv[1], "r");
    if (!pFile) {
        fprintf(stderr, "Can't open the file: %s\n", argv[1]);
        exit(-1);
    }

    XML_Parser p = XML_ParserCreate(NULL);
    if (!p) {
        fprintf(stderr, "Couldn't allocate memory for parser\n");
        exit(-1);
    }

    XML_SetElementHandler(p, start, end);
    XML_SetCharacterDataHandler(p, CharacterHandler);

    for (;;) {
        int done;
        int len;

        len = (int)fread(Buff, 1, BUFFSIZE, pFile);
        if (ferror(stdin)) {
            fprintf(stderr, "Read error\n");
            exit(-1);
        }
        done = feof(pFile);

        printf("XML_Parse <IN>\n");
        if (XML_Parse(p, Buff, len, done) == XML_STATUS_ERROR) {
            fprintf(stderr, "Parse error at line %" XML_FMT_INT_MOD "u:\n%s\n",
                    XML_GetCurrentLineNumber(p),
                    XML_ErrorString(XML_GetErrorCode(p)));
            exit(-1);
        }
        printf("XML_Parse <OUT>\n");

        if (done)
            break;
    }
    XML_ParserFree(p);
    fclose(pFile);
    return 0;
}
