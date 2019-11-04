#include "../src/co_utils.h"

int main(int argc, char const *argv[])
{
    FILE * f = fopen("LICENSE", "r");
    if (f) {
        char word[256];     // 关键字最多支持这么长
        while (fgets(word, 256, f)) {
            int len = strlen(word) - 1;
            if (word[len] == '\n')
                word[len] = '\0';
            if (len > 0)
                printf("%s\n", word);
        }
        fclose(f);
    }
    return 0;
}
