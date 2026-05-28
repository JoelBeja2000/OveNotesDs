#include "io.h"
#include <fat.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>

bool ioInitFAT(void) {
    return fatInitDefault();
}

bool ioSaveNote(const uint16_t* buffer) {
    if (buffer == NULL) return false;
    
    // Ensure directory exists
    mkdir("/ovenotes", 0777);
    
    // Find next available filename
    char filename[64];
    FILE* f = NULL;
    int index = 1;
    while (index < 1000) {
        sprintf(filename, "/ovenotes/nota_%03d.bin", index);
        f = fopen(filename, "rb");
        if (f == NULL) {
            // Found unused index
            break;
        }
        fclose(f);
        index++;
    }
    
    sprintf(filename, "/ovenotes/nota_%03d.bin", index);
    f = fopen(filename, "wb");
    if (f == NULL) {
        printf("[IO] Error creating file: %s\n", filename);
        return false;
    }
    
    size_t written = fwrite(buffer, sizeof(uint16_t), 256 * 192, f);
    fclose(f);
    
    printf("[IO] Saved: %s (%d pixels written)\n", filename, (int)written);
    return (written == 256 * 192);
}

int ioGetNoteList(char filenames[][32], int max_files) {
    DIR* dir = opendir("/ovenotes");
    if (dir == NULL) {
        return 0;
    }
    
    int count = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        // Look for .bin files
        int len = strlen(entry->d_name);
        if (len > 4 && strcmp(entry->d_name + len - 4, ".bin") == 0) {
            strncpy(filenames[count], entry->d_name, 31);
            filenames[count][31] = '\0';
            count++;
            if (count >= max_files) {
                break;
            }
        }
    }
    closedir(dir);
    
    // Bubble sort filenames alphabetically
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (strcmp(filenames[i], filenames[j]) > 0) {
                char temp[32];
                strcpy(temp, filenames[i]);
                strcpy(filenames[i], filenames[j]);
                strcpy(filenames[j], temp);
            }
        }
    }
    
    return count;
}

bool ioLoadNote(const char* filename, uint16_t* dest_buffer) {
    if (filename == NULL || dest_buffer == NULL) return false;
    
    char path[64];
    sprintf(path, "/ovenotes/%s", filename);
    
    FILE* f = fopen(path, "rb");
    if (f == NULL) {
        printf("[IO] Error opening file: %s\n", path);
        return false;
    }
    
    size_t read = fread(dest_buffer, sizeof(uint16_t), 256 * 192, f);
    fclose(f);
    
    printf("[IO] Loaded: %s (%d pixels read)\n", path, (int)read);
    return (read == 256 * 192);
}
