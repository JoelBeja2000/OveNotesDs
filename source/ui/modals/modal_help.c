#define NO_UI_COMPAT_MACROS
#include "modals.h"
#include "ui.h"
#include "render.h"
#include "widget_buttons.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define app_theme_color app->ui.app_theme_color
#define active_theme_idx app->ui.active_theme_idx
#define current_lang app->ui.current_lang

static void drawTopPixel(int x, int y, uint16_t color) {
    if (wizard_buffer && x >= 0 && x < 256 && y >= 0 && y < 192)
        wizard_buffer[y * 256 + x] = color;
}

static void drawTopLine(int x0, int y0, int x1, int y1, uint16_t color) {
    int dx = abs(x1-x0), sx = x0<x1?1:-1;
    int dy = -abs(y1-y0), sy = y0<y1?1:-1;
    int err = dx+dy, e2;
    while(1) {
        drawTopPixel(x0,y0,color);
        if(x0==x1 && y0==y1) break;
        e2=2*err;
        if(e2>=dy){err+=dy; x0+=sx;}
        if(e2<=dx){err+=dx; y0+=sy;}
    }
}

static void drawTopRectOutline(int x0, int y0, int x1, int y1, uint16_t color) {
    drawTopLine(x0,y0,x1,y0,color);
    drawTopLine(x1,y0,x1,y1,color);
    drawTopLine(x1,y1,x0,y1,color);
    drawTopLine(x0,y1,x0,y0,color);
}

static void drawTopRect(int x0, int y0, int x1, int y1, uint16_t color) {
    for(int y=y0;y<=y1;y++)
        for(int x=x0;x<=x1;x++)
            drawTopPixel(x,y,color);
}

static void drawTopCircle(int xc, int yc, int r, uint16_t color, bool filled) {
    int x=0,y=r,d=3-2*r;
    while(y>=x) {
        if(filled) {
            drawTopLine(xc-x,yc-y,xc+x,yc-y,color);
            drawTopLine(xc-x,yc+y,xc+x,yc+y,color);
            drawTopLine(xc-y,yc-x,xc+y,yc-x,color);
            drawTopLine(xc-y,yc+x,xc+y,yc+x,color);
        } else {
            drawTopPixel(xc+x,yc+y,color); drawTopPixel(xc-x,yc+y,color);
            drawTopPixel(xc+x,yc-y,color); drawTopPixel(xc-x,yc-y,color);
            drawTopPixel(xc+y,yc+x,color); drawTopPixel(xc-y,yc+x,color);
            drawTopPixel(xc+y,yc-x,color); drawTopPixel(xc-y,yc-x,color);
        }
        if(d<0) d=d+4*x+6; else { d=d+4*(x-y)+10; y--; }
        x++;
    }
}

// Page 0 diagram: two canvas states (HUD on / HUD off) + dpad
static void drawPage0Diagram(uint16_t theme) {
    uint16_t canvas_bg  = RGB15(2,2,3);
    uint16_t toolbar_bg = RGB15(5,5,7);
    uint16_t stroke_col = RGB15(22,22,22);
    uint16_t dim_col    = RGB15(10,10,12);

    // === Left box: canvas WITH toolbar (HUD visible) ===
    int lx0=16, ly0=28, lx1=106, ly1=148;
    drawTopRect(lx0,ly0,lx1,ly1,canvas_bg);
    drawTopRectOutline(lx0,ly0,lx1,ly1,dim_col);
    // mock strokes
    drawTopLine(lx0+8,ly0+30,lx0+50,ly0+55,stroke_col);
    drawTopLine(lx0+50,ly0+55,lx0+70,ly0+40,stroke_col);
    drawTopLine(lx0+20,ly0+65,lx0+60,ly0+70,stroke_col);
    // toolbar strip at bottom of left box
    int tb_y0=ly1-14, tb_y1=ly1-2;
    drawTopRect(lx0+2,tb_y0,lx1-2,tb_y1,toolbar_bg);
    drawTopRectOutline(lx0+2,tb_y0,lx1-2,tb_y1,theme);
    // label
    renderDrawTextOnBuffer(wizard_buffer,"CON HUD",lx0+12,ly1+4,theme,0);

    // === Right box: canvas WITHOUT toolbar (HUD hidden) ===
    int rx0=150, ry0=28, rx1=240, ry1=148;
    drawTopRect(rx0,ry0,rx1,ry1,canvas_bg);
    drawTopRectOutline(rx0,ry0,rx1,ry1,theme);
    // mock strokes extending to full bottom
    drawTopLine(rx0+8,ry0+30,rx0+50,ry0+55,stroke_col);
    drawTopLine(rx0+50,ry0+55,rx0+70,ry0+40,stroke_col);
    drawTopLine(rx0+20,ry0+65,rx0+60,ry0+70,stroke_col);
    drawTopLine(rx0+10,ry0+90,rx0+75,ry0+95,stroke_col); // extra strokes in reclaimed area
    drawTopLine(rx0+15,ry0+108,rx0+65,ry0+112,stroke_col);
    // label
    renderDrawTextOnBuffer(wizard_buffer,"SIN HUD",rx0+8,ry1+4,theme,0);

    // === Center: D-pad with only UP and DOWN highlighted ===
    int cx=128, cy=88;
    // cross arms
    drawTopRect(cx-4,cy-14,cx+4,cy+14,RGB15(12,12,14));
    drawTopRect(cx-14,cy-4,cx+14,cy+4,RGB15(12,12,14));
    // UP highlighted
    drawTopRect(cx-4,cy-14,cx+4,cy-4,theme);
    drawTopLine(cx,cy-12,cx-3,cy-8,theme);
    drawTopLine(cx,cy-12,cx+3,cy-8,theme);
    // DOWN highlighted
    drawTopRect(cx-4,cy+4,cx+4,cy+14,theme);
    drawTopLine(cx,cy+12,cx-3,cy+8,theme);
    drawTopLine(cx,cy+12,cx+3,cy+8,theme);

    // Arrows connecting boxes to dpad
    drawTopLine(rx0-2,cy,cx+16,cy,RGB15(18,18,18)); // right box -> dpad
    drawTopLine(lx1+2,cy,cx-16,cy,RGB15(18,18,18)); // left box <- dpad
    drawTopPixel(cx+14,cy-2,RGB15(18,18,18)); drawTopPixel(cx+14,cy+2,RGB15(18,18,18)); // arrowhead right
    drawTopPixel(cx-14,cy-2,RGB15(18,18,18)); drawTopPixel(cx-14,cy+2,RGB15(18,18,18)); // arrowhead left
}

static void drawHelpTopScreenDiagram(const AppState* app) {
    if (wizard_buffer == NULL) return;

    uint16_t bg_color  = RGB15(4,4,5);
    uint16_t panel_bg  = RGB15(6,6,8);
    uint16_t line_col  = RGB15(15,15,15);
    uint16_t active    = app_theme_color;

    // Clear top screen (192 rows, 256 cols)
    for(int y=0;y<192;y++)
        for(int x=0;x<256;x++)
            wizard_buffer[y*256+x] = bg_color;

    drawTopRectOutline(4,4,251,187,line_col);

    int page = app->ui.help_page;

    if(page == 0) {
        // Two canvas states + dpad
        drawPage0Diagram(active);
        if(current_lang==0) {
            renderDrawTextOnBuffer(wizard_buffer,"ARRIBA: muestra controles",16,12,active,0);
            renderDrawTextOnBuffer(wizard_buffer,"ABAJO: oculta interfaz",16,168,active,0);
        } else if(current_lang==2) {
            renderDrawTextOnBuffer(wizard_buffer,"HAUT: affiche l'interface",16,12,active,0);
            renderDrawTextOnBuffer(wizard_buffer,"BAS: cache l'interface",16,168,active,0);
        } else {
            renderDrawTextOnBuffer(wizard_buffer,"UP: show HUD interface",16,12,active,0);
            renderDrawTextOnBuffer(wizard_buffer,"DOWN: hide HUD (full canvas)",16,168,active,0);
        }
    }
    else if(page == 1) {
        // Layer management mockup
        int sx=40,sy=24;
        for(int i=0;i<3;i++) {
            int ypos=sy+i*38;
            drawTopRect(sx,ypos,sx+176,ypos+28,panel_bg);
            drawTopRectOutline(sx,ypos,sx+176,ypos+28,(i==1)?active:line_col);
            drawTopCircle(sx+12,ypos+14,6,(i==1)?active:line_col,false);
            drawTopCircle(sx+12,ypos+14,2,active,true);
            char lbl[16]; sprintf(lbl,"LAYER %d",2-i);
            renderDrawTextOnBuffer(wizard_buffer,lbl,sx+28,ypos+10,RGB15(31,31,31),0);
            renderDrawTextOnBuffer(wizard_buffer,"V",sx+162,ypos+10,active,0);
        }
        drawTopLine(sx-12,sy+28,sx-12,sy+76,active);
        drawTopLine(sx-15,sy+33,sx-12,sy+28,active);
        drawTopLine(sx-9,sy+33,sx-12,sy+28,active);
        drawTopLine(sx-15,sy+71,sx-12,sy+76,active);
        drawTopLine(sx-9,sy+71,sx-12,sy+76,active);
        if(current_lang==0) {
            renderDrawTextOnBuffer(wizard_buffer,"CIRCULO: arrastrar para ordenar",12,148,active,0);
            renderDrawTextOnBuffer(wizard_buffer,"MRG DN/UP: fusionar capas",12,164,RGB15(28,28,28),0);
        } else if(current_lang==2) {
            renderDrawTextOnBuffer(wizard_buffer,"CERCLE: glisser pour reordonner",12,148,active,0);
            renderDrawTextOnBuffer(wizard_buffer,"F.BAS/F.HAUT: fusionner calques",12,164,RGB15(28,28,28),0);
        } else {
            renderDrawTextOnBuffer(wizard_buffer,"LEFT CIRCLE: drag to reorder",12,148,active,0);
            renderDrawTextOnBuffer(wizard_buffer,"MRG DN/UP: merge layers",12,164,RGB15(28,28,28),0);
        }
    }
    else if(page == 2) {
        int horizon=80, vpx=128, vpy=horizon;
        drawTopLine(10,horizon,246,horizon,line_col);
        drawTopCircle(vpx,vpy,4,active,true);
        renderDrawTextOnBuffer(wizard_buffer,"VP",vpx-6,vpy-14,active,0);
        for(int x=20;x<256;x+=32) drawTopLine(vpx,vpy,x,180,line_col);
        drawTopRectOutline(180,20,240,60,line_col);
        drawTopCircle(210,40,16,line_col,false);
        drawTopLine(210-11,40+11,210+11,40-11,active);
        if(current_lang==0) {
            renderDrawTextOnBuffer(wizard_buffer,"PERSPECTIVA: arrastra los puntos",12,148,active,0);
            renderDrawTextOnBuffer(wizard_buffer,"Angulo pluma: rueda en BG>TRAZO",12,164,RGB15(28,28,28),0);
        } else if(current_lang==2) {
            renderDrawTextOnBuffer(wizard_buffer,"PERSPECTIVE: glisser les points",12,148,active,0);
            renderDrawTextOnBuffer(wizard_buffer,"Angle plume: roue dans BG>TRAIT",12,164,RGB15(28,28,28),0);
        } else {
            renderDrawTextOnBuffer(wizard_buffer,"PERSPECTIVE: drag the points",12,148,active,0);
            renderDrawTextOnBuffer(wizard_buffer,"Nib angle: wheel in BG>STROKE",12,164,RGB15(28,28,28),0);
        }
    }
    else if(page == 3) {
        // NDS <-> SERVER + SD card (repositioned: top at y=22 to avoid clipping)
        drawTopRect(24,58,84,108,panel_bg);
        drawTopRectOutline(24,58,84,108,line_col);
        renderDrawTextOnBuffer(wizard_buffer,"NDS",42,80,RGB15(31,31,31),0);

        drawTopRect(172,58,232,108,panel_bg);
        drawTopRectOutline(172,58,232,108,line_col);
        renderDrawTextOnBuffer(wizard_buffer,"SERVER",178,80,RGB15(31,31,31),0);

        drawTopLine(100,83,156,83,active);
        drawTopLine(150,78,156,83,active);
        drawTopLine(150,88,156,83,active);
        renderDrawTextOnBuffer(wizard_buffer,"WIFI FTP",104,70,active,0);
        renderDrawTextOnBuffer(wizard_buffer,"CODE: AB12",100,90,RGB15(28,28,28),0);

        // SD card icon: moved down so top border is visible
        drawTopRectOutline(116,22,136,48,line_col);
        drawTopLine(116,28,122,22,line_col); // notch
        renderDrawTextOnBuffer(wizard_buffer,"SD",120,32,active,0);

        if(current_lang==0) {
            renderDrawTextOnBuffer(wizard_buffer,"SELECT: guarda nota en SD (PNG)",12,148,active,0);
            renderDrawTextOnBuffer(wizard_buffer,"WIFI: usa el codigo del PC",12,164,RGB15(28,28,28),0);
        } else if(current_lang==2) {
            renderDrawTextOnBuffer(wizard_buffer,"SELECT: sauvegarder sur SD",12,148,active,0);
            renderDrawTextOnBuffer(wizard_buffer,"WIFI: utiliser le code du PC",12,164,RGB15(28,28,28),0);
        } else {
            renderDrawTextOnBuffer(wizard_buffer,"SELECT: save note to SD (PNG)",12,148,active,0);
            renderDrawTextOnBuffer(wizard_buffer,"WIFI: use the pairing code",12,164,RGB15(28,28,28),0);
        }
    }
}

// Render bottom screen modal
void uiDrawHelpModal(const AppState* app) {
    uint16_t bg_modal  = RGB15(4,4,5);
    uint16_t border    = app_theme_color;
    uint16_t text_col  = RGB15(31,31,31);
    uint16_t label_col = blendRGB555_int(border,RGB15(31,31,31),16);

    renderDrawRect(8,12,247,180,bg_modal);
    renderDrawRectOutline(8,12,247,180,border);

    char title[48];
    if(current_lang==0)      sprintf(title,"GUIA DE CONTROLES (%d/4)",app->ui.help_page+1);
    else if(current_lang==2) sprintf(title,"GUIDE DE CONTROLES (%d/4)",app->ui.help_page+1);
    else                     sprintf(title,"CONTROLS GUIDE (%d/4)",app->ui.help_page+1);
    renderDrawText(title,16,18,label_col,0);

    renderDrawRect(230,14,244,26,RGB15(12,4,4));
    renderDrawRectOutline(230,14,244,26,RGB15(31,8,8));
    renderDrawText("X",235,16,RGB15(31,10,10),0);

    for(int x=12;x<244;x++)
        renderSetPixel(x,30,blendRGB555_int(border,RGB15(4,4,5),8));

    int page = app->ui.help_page;
    // Max ~36 chars per line from x=16 to x=232
    if(page==0) {
        if(current_lang==0) {
            renderDrawText("1. LIENZO Y DIBUJO:",16,40,border,0);
            renderDrawText("- Barra inferior: tipo pincel y size",16,54,text_col,0);
            renderDrawText("- Deshacer < y Rehacer > arriba izq.",16,66,text_col,0);
            renderDrawText("- MODO PANTALLA COMPLETA:",16,82,label_col,0);
            renderDrawText("  ABAJO: Oculta toda la interfaz.",16,94,text_col,0);
            renderDrawText("  ARRIBA: Restaura los controles.",16,106,text_col,0);
        } else if(current_lang==2) {
            renderDrawText("1. DESSIN & PIXELS :",16,40,border,0);
            renderDrawText("- Barre du bas: reglage pinceaux.",16,54,text_col,0);
            renderDrawText("- Annuler < Refaire > haut gauche.",16,66,text_col,0);
            renderDrawText("- MODE SANS DISTRACTION :",16,82,label_col,0);
            renderDrawText("  BAS : Cache l'interface.",16,94,text_col,0);
            renderDrawText("  HAUT : Restaure les menus.",16,106,text_col,0);
        } else {
            renderDrawText("1. CANVAS & SKETCHING:",16,40,border,0);
            renderDrawText("- Bottom bar: brush type and size.",16,54,text_col,0);
            renderDrawText("- Undo < & Redo > at top-left.",16,66,text_col,0);
            renderDrawText("- DISTRACTION-FREE MODE:",16,82,label_col,0);
            renderDrawText("  DPAD DOWN: Hides all UI panels.",16,94,text_col,0);
            renderDrawText("  DPAD UP: Restores UI back.",16,106,text_col,0);
        }
    }
    else if(page==1) {
        if(current_lang==0) {
            renderDrawText("2. GESTION DE CAPAS:",16,40,border,0);
            renderDrawText("- Panel de capas: lado derecho.",16,54,text_col,0);
            renderDrawText("- REORDENAR:",16,68,label_col,0);
            renderDrawText("  Arrastra el circulo izquierdo",16,80,text_col,0);
            renderDrawText("  de cada ranura para moverla.",16,92,text_col,0);
            renderDrawText("- FUSIONAR:",16,106,label_col,0);
            renderDrawText("  MRG DN/UP une capas arriba/abajo.",16,118,text_col,0);
            renderDrawText("- OPACIDAD: deslizador del panel.",16,132,text_col,0);
        } else if(current_lang==2) {
            renderDrawText("2. GESTION DES CALQUES :",16,40,border,0);
            renderDrawText("- Panel lateral des calques.",16,54,text_col,0);
            renderDrawText("- REORDONNER :",16,68,label_col,0);
            renderDrawText("  Glissez le cercle de gauche.",16,80,text_col,0);
            renderDrawText("- FUSION :",16,94,label_col,0);
            renderDrawText("  F.BAS/F.HAUT unit les calques.",16,106,text_col,0);
            renderDrawText("- OPACITE: barre glissante.",16,120,text_col,0);
        } else {
            renderDrawText("2. LAYER MANAGEMENT:",16,40,border,0);
            renderDrawText("- Open layers sidebar (right).",16,54,text_col,0);
            renderDrawText("- REORDER (DRAG & DROP):",16,68,label_col,0);
            renderDrawText("  Drag the left circle handle",16,80,text_col,0);
            renderDrawText("  on any slot to reorder layers.",16,92,text_col,0);
            renderDrawText("- MERGE LAYERS:",16,106,label_col,0);
            renderDrawText("  MRG DN/UP combines layers.",16,118,text_col,0);
            renderDrawText("- OPACITY: use the sidebar slider.",16,132,text_col,0);
        }
    }
    else if(page==2) {
        if(current_lang==0) {
            renderDrawText("3. FONDOS Y PERSPECTIVA:",16,40,border,0);
            renderDrawText("- Menu 'BG' en la barra inferior.",16,54,text_col,0);
            renderDrawText("- PERSPECTIVA:",16,68,label_col,0);
            renderDrawText("  Elige 1, 2 o 3 puntos de fuga.",16,80,text_col,0);
            renderDrawText("  Activa EDIT:SI y arrastra los",16,92,text_col,0);
            renderDrawText("  puntos sobre el lienzo.",16,104,text_col,0);
            renderDrawText("- COLORES DE GUIA:",16,118,label_col,0);
            renderDrawText("  Primaria(P) Secundaria(S) en BG.",16,130,text_col,0);
        } else if(current_lang==2) {
            renderDrawText("3. FONDS & PERSPECTIVE :",16,40,border,0);
            renderDrawText("- Menu 'BG' dans la barre du bas.",16,54,text_col,0);
            renderDrawText("- PERSPECTIVE :",16,68,label_col,0);
            renderDrawText("  1, 2 ou 3 points de fuite.",16,80,text_col,0);
            renderDrawText("  Clic EDIT:OUI et glissez les pts",16,92,text_col,0);
            renderDrawText("  sur le canvas.",16,104,text_col,0);
            renderDrawText("- COULEURS DE GRILLE :",16,118,label_col,0);
            renderDrawText("  Choisir couleur P ou S dans BG.",16,130,text_col,0);
        } else {
            renderDrawText("3. BACKGROUNDS & PERSPECTIVE:",16,40,border,0);
            renderDrawText("- Open the 'BG' modal (bottom bar).",16,54,text_col,0);
            renderDrawText("- PERSPECTIVE GRID:",16,68,label_col,0);
            renderDrawText("  Select 1, 2 or 3 Vanishing Pts.",16,80,text_col,0);
            renderDrawText("  Toggle EDIT:YES, drag to place",16,92,text_col,0);
            renderDrawText("  guide points on the canvas.",16,104,text_col,0);
            renderDrawText("- GUIDE GRID COLORS:",16,118,label_col,0);
            renderDrawText("  Set P & S colors in BG panel.",16,130,text_col,0);
        }
    }
    else if(page==3) {
        if(current_lang==0) {
            renderDrawText("4. GUARDADO Y COMPANION APP:",16,40,border,0);
            renderDrawText("- GUARDAR EN SD:",16,54,label_col,0);
            renderDrawText("  Presiona SELECT para guardar",16,66,text_col,0);
            renderDrawText("  la nota como PNG en la SD.",16,78,text_col,0);
            renderDrawText("- ENVIAR AL PC (WIFI):",16,92,label_col,0);
            renderDrawText("  Abre OveNotes Companion en PC.",16,104,text_col,0);
            renderDrawText("  Escribe el codigo en la DS.",16,116,text_col,0);
            renderDrawText("  Pulsa ENVIAR para exportar.",16,128,text_col,0);
        } else if(current_lang==2) {
            renderDrawText("4. SAUVEGARDE & COMPANION :",16,40,border,0);
            renderDrawText("- SUR CARTE SD :",16,54,label_col,0);
            renderDrawText("  SELECT: sauvegarde en PNG.",16,66,text_col,0);
            renderDrawText("- ENVOI AU PC (WIFI) :",16,80,label_col,0);
            renderDrawText("  Lancez OveNotes Companion PC.",16,92,text_col,0);
            renderDrawText("  Saisissez le code sur la DS.",16,104,text_col,0);
            renderDrawText("  Clic ENVOI pour transmettre.",16,116,text_col,0);
        } else {
            renderDrawText("4. SAVING & COMPANION APP:",16,40,border,0);
            renderDrawText("- SAVE TO SD CARD:",16,54,label_col,0);
            renderDrawText("  Press SELECT to save note",16,66,text_col,0);
            renderDrawText("  locally as PNG on SD card.",16,78,text_col,0);
            renderDrawText("- TRANSMIT TO PC (WIFI):",16,92,label_col,0);
            renderDrawText("  Launch OveNotes Companion on PC.",16,104,text_col,0);
            renderDrawText("  Type the pairing code on NDS.",16,116,text_col,0);
            renderDrawText("  Tap SEND to export sketch.",16,128,text_col,0);
        }
    }

    // Nav buttons
    if(page>0) {
        renderDrawRect(16,158,74,174,RGB15(6,6,8));
        renderDrawRectOutline(16,158,74,174,border);
        renderDrawText(uiTxt(TXT_HELP_PREV),20,162,text_col,0);
    }
    if(page<3) {
        renderDrawRect(182,158,240,174,RGB15(6,6,8));
        renderDrawRectOutline(182,158,240,174,border);
        renderDrawText(uiTxt(TXT_HELP_NEXT),186,162,text_col,0);
    }

    drawHelpTopScreenDiagram(app);
}

// Refresh only the modal area (no full menu redraw -> no flickering)
void uiRefreshHelpModal(const AppState* app) {
    // Clear only the modal region on the bottom screen
    uint16_t bg = RGB15(4,4,5);
    for(int y=12;y<=180;y++)
        for(int x=8;x<=247;x++)
            canvas_buffer[y*256+x] = bg;
    uiDrawHelpModal(app);
}
