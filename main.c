#include <stdio.h>

void showUsage() {
    const char* usageText = "USAGE:\n./barmark <filename>";
    printf("%s\n", usageText);
}

int main(int argc, char **argv) {
    if (argc > 1) {
        printf("file name: %s\n", argv[1]);
    } else {
        printf("No file name given.\n");
        showUsage();
    }
    return 0;
}