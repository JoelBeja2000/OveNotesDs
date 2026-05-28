#ifndef GAME_H
#define GAME_H

#include <nds.h>

typedef enum {
    STATE_START_MENU,
    STATE_NOTES_GALLERY,
    STATE_DRAW,
    STATE_WIZARD,
    STATE_UPLOAD,
    STATE_RENAME_LAYER,
} GameState;

extern GameState current_state;

void gameInit(void);
void gameUpdate(void);

#endif // GAME_H
