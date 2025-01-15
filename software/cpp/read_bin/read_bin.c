#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Function to convert a single hex character to its integer value
int hexCharToInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1; // Invalid hex character
}

// Function to convert a hex string to binary
size_t convertHexToBinary(const char *hexStr, unsigned char *output) {
    size_t len = strlen(hexStr);
    size_t binaryLen = 0;

    for (size_t i = 0; i < len; i++) {
        if (isspace(hexStr[i])) continue; // Skip whitespace
        if (hexStr[i] == '0' && (hexStr[i + 1] == 'x' || hexStr[i + 1] == 'X')) {
            i++; // Skip '0x'
            continue;
        }

        int high = hexCharToInt(hexStr[i]);
        if (high == -1 || i + 1 >= len) return 0; // Error: Invalid hex or odd length
        int low = hexCharToInt(hexStr[++i]);

        if (low == -1) return 0; // Error: Invalid hex
        output[binaryLen++] = (high << 4) | low;
    }
    return binaryLen;
}

int main() {
    const char *binaryFileName = "log1.bin";
    const char *asciiFileName = "log1_serial.bin";

    // Open the binary file
    FILE *binaryFile = fopen(binaryFileName, "rb");
    if (!binaryFile) {
        perror("Failed to open binary file");
        return 1;
    }

    // Get binary file size
    fseek(binaryFile, 0, SEEK_END);
    long binaryFileSize = ftell(binaryFile);
    rewind(binaryFile);

    // Read binary file into memory
    unsigned char *binaryData = (unsigned char *)malloc(binaryFileSize);
    if (!binaryData) {
        perror("Memory allocation failed");
        fclose(binaryFile);
        return 1;
    }
    fread(binaryData, 1, binaryFileSize, binaryFile);
    fclose(binaryFile);

    // Open the ASCII file
    FILE *asciiFile = fopen(asciiFileName, "r");
    if (!asciiFile) {
        perror("Failed to open ASCII file");
        free(binaryData);
        return 1;
    }

    // Read ASCII file into memory
    fseek(asciiFile, 0, SEEK_END);
    long asciiFileSize = ftell(asciiFile);
    rewind(asciiFile);

    char *asciiData = (char *)malloc(asciiFileSize + 1);
    if (!asciiData) {
        perror("Memory allocation failed");
        fclose(asciiFile);
        free(binaryData);
        return 1;
    }
    fread(asciiData, 1, asciiFileSize, asciiFile);
    asciiData[asciiFileSize] = '\0'; // Null-terminate the ASCII data
    fclose(asciiFile);

    // Convert ASCII hex to binary
    unsigned char *asciiToBinary = (unsigned char *)malloc(binaryFileSize);
    if (!asciiToBinary) {
        perror("Memory allocation failed");
        free(asciiData);
        free(binaryData);
        return 1;
    }

    size_t convertedSize = convertHexToBinary(asciiData, asciiToBinary);
    free(asciiData);

    if (convertedSize != binaryFileSize) {
        printf("Size mismatch: binary file size (%ld), converted ASCII size (%zu)\n", binaryFileSize, convertedSize);
        free(asciiToBinary);
        free(binaryData);
        return 1;
    }

    // Compare the binary data
    if (memcmp(binaryData, asciiToBinary, binaryFileSize) == 0) {
        printf("The files are identical.\n");
    } else {
        printf("The files are different.\n");
    }

    free(asciiToBinary);
    free(binaryData);
    return 0;
}
