#ifndef UI_SHARED_H
#define UI_SHARED_H

#include <nds.h>
#include <stdbool.h>
#include <stdint.h>

#include "render.h"

// Primitive macro redirections to rendering layer
#define drawRect(x0, y0, x1, y1, color)         renderDrawRect(x0, y0, x1, y1, color)
#define drawRectOutline(x0, y0, x1, y1, color)  renderDrawRectOutline(x0, y0, x1, y1, color)
#define drawLine(x0, y0, x1, y1, color)         renderDrawLineSimple(x0, y0, x1, y1, color)
#define drawCircleOutline(xc, yc, r, color)    renderDrawCircleOutline(xc, yc, r, color)
#define drawFilledCircle(xm, ym, r, color)     renderDrawFilledCircle(xm, ym, r, color)


// Button and widget drawing components
void drawModalButtonAt(int x0, int x1, int y0, int y1, const char* text, bool selected);
void drawModalButtonDisabledAt(int x0, int x1, int y0, int y1, const char* text);
void drawToolButtonWithIconAt(int x0, int x1, int y0, int y1, const char* text, int icon_type, bool selected);
void drawPatternBrushButtonAt(int x0, int x1, int y0, int y1, int pat_idx, bool selected);
void drawPlumaButtonAt(int x0, int x1, int y0, int y1, const char* label, int pl_type, bool selected);
void drawModalColorButtonAt(int x0, int x1, int y0, int y1, uint16_t color, bool selected);
#define drawPatternPreview(x0, y0, x1, y1, pat_idx, selected)  renderDrawPatternPreview(x0, y0, x1, y1, pat_idx, selected)
#define drawLockIcon(x, y, locked)                             renderDrawLockIcon(x, y, locked)

#endif // UI_SHARED_H
