#ifndef GAME_H
#define GAME_H

#include <nds.h>

typedef enum {
    STATE_DRAW,
    STATE_WIZARD,
    STATE_UPLOAD,
} GameState;

extern GameState current_state;

void gameInit(void);
void gameUpdate(void);

#endif // GAME_H
