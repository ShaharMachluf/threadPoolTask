#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(NULL)); // seed the random number generator

    // open a file for writing
    FILE* fp = fopen("letters.txt", "w");
    if (fp == NULL) {
        printf("Failed to open file.\n");
        return 1;
    }

    // write 100 random letters to the file
    for (int i = 0; i < 2000; i++) {
        char c = 'a' + (rand() % 26); // generate a random letter
        fprintf(fp, "%c", c); // write the letter to the file
    }

    // close the file
    fclose(fp);

    return 0;
}