#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *file;
    char buffer[BUFFER_SIZE];
    size_t bytesRead;

    // Open the file in binary mode
    file = fopen(argv[1], "rb");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    // Read bytes from the file into the buffer
    bytesRead = fread(buffer, sizeof(char), BUFFER_SIZE, file);
    if (bytesRead < 0) {
        perror("Error reading file");
        fclose(file);
        return EXIT_FAILURE;
    }
    size_t address = 0;
    while((bytesRead = fread(buffer, sizeof(char), BUFFER_SIZE, file)) > 0) {
        // Process the read bytes (for demonstration, we will just print them)
        printf("Bytes read: %zu\n", bytesRead);
        for (size_t i = 0; i < bytesRead; i++) {
            if (i % 16 == 0) {
                // printf("%p:", (void *)(&buffer[i])); // Off set in memory
                printf("0x%08lx: ", (unsigned long)address); // Offset in file
            }
            printf("%02x ", (unsigned char)buffer[i]);
            if ((i + 1) % 16 == 0) {
                printf("\n");
            }
            address++;
        }
        printf("\n");
        printf("Reading next chunk...\n");

    }

    // Process the read bytes (for demonstration, we will just print them)
    printf("Bytes read: %zu\n", bytesRead);
    for (size_t i = 0; i < bytesRead; i++) {
        if (i % 16 == 0) {
            // printf("%p:", (void *)(&buffer[i])); // Off set in memory
            printf("0x%08lx: ", (unsigned long)i); // Offset in file
        }
        printf("%02x ", (unsigned char)buffer[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");

    // Close the file
    fclose(file);
    return EXIT_SUCCESS;
}