#ifndef IO_H
#define IO_H

#include <stdbool.h>
#include <stdint.h>

bool ioInitFAT(void);
bool ioSaveNote(const uint16_t* buffer);
int ioGetNoteList(char filenames[][32], int max_files);
bool ioLoadNote(const char* filename, uint16_t* dest_buffer);

#endif // IO_H
