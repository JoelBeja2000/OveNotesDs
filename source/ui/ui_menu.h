#ifndef UI_MENU_H
#define UI_MENU_H

#include <nds.h>

void uiDrawStartMenu(void);
void uiDrawNotesGallery(int selected_idx, int total_count, const char filenames[][32]);

#endif // UI_MENU_H
