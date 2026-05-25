#ifndef INPUT_H
#define INPUT_H

#include <nds.h>
#include <stdbool.h>

void inputScan(void);
uint32_t inputGetKeysHeld(void);
uint32_t inputGetKeysDown(void);
bool inputGetTouch(touchPosition* touch);

#endif // INPUT_H
