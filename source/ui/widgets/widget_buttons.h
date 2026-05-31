#ifndef WIDGET_BUTTONS_H
#define WIDGET_BUTTONS_H

#include <nds.h>
#include <stdbool.h>
#include <stdint.h>

void drawModalButtonAt(int x0, int x1, int y0, int y1, const char* text, bool selected);
void drawModalButtonDisabledAt(int x0, int x1, int y0, int y1, const char* text);
void drawToolButtonWithIconAt(int x0, int x1, int y0, int y1, const char* text, int icon_type, bool selected);
void drawPatternBrushButtonAt(int x0, int x1, int y0, int y1, int pat_idx, bool selected);
void drawPlumaButtonAt(int x0, int x1, int y0, int y1, const char* label, int pl_type, bool selected);
void drawModalColorButtonAt(int x0, int x1, int y0, int y1, uint16_t color, bool selected);

#endif // WIDGET_BUTTONS_H
