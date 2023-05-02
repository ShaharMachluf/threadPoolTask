#include <stdio.h>

int main() {
    int text[1000]; // declare a character array to hold the text to write
    int i;
    FILE *fp; // declare a file pointer
    fp = fopen("output.txt", "w"); // open the file in write mode

    if (fp == NULL) { // check if file opening was successful
        printf("Error opening file.");
        return 1;
    }

    // fill the text array with 1025 characters
    for (i = 0; i < 1000; i++) {
        text[i] = i; // fill the array with the character 'a'
    }

    // write the text array to the file
    for(int i=0; i<1000; i++){
        fprintf(fp, "%d ", text[i]);
    }

    fclose(fp); // close the file
    return 0;
}