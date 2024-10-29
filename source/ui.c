// SPDX-License-Identifier: MIT
//
// SPDX-FileContributor: Adrian "asie" Siekierka, 2024

#include <stdio.h>
#include <nds.h>
#include <nds/arm9/dldi.h>
#include <fat.h>
#include "ui.h"

static PrintConsole bottomConsole, topConsole;

void ui_init(void) {
    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);

    vramSetPrimaryBanks(VRAM_A_LCD, VRAM_B_LCD, VRAM_C_SUB_BG, VRAM_D_MAIN_BG_0x06000000);
    setBrightness(3, 0);

    consoleInit(&bottomConsole,
        0, BgType_Text4bpp, BgSize_T_256x256, 22, 3, false, true);
    consoleInit(&topConsole,
        0, BgType_Text4bpp, BgSize_T_256x256, 22, 3, true, true);

    consoleSelect(&topConsole);
    printf("\x1b[2J");
    printf("\x1b[16;0H________________________________");
    printf("    _ _    _ _\n");
    printf(" __| | |__| (_) unofficial\n");
    printf("/ _` | / _` | | driver benchmark");
    printf("\\__,_|_\\__,_|_| ");

#ifdef BLOCKSDS
    printf("v0.3  / BlocksDS");
#else
    printf("v0.3  /    dkARM");
#endif
    printf("\n\x1b[37m%s\x1b[39m\n\n", io_dldi_data->friendlyName);

    consoleSetWindow(&topConsole, 0, 0, 32, 15);
    printf("\x1b[14;0H");
    consoleSelect(&bottomConsole);
    printf("\x1b[2J");
}

void ui_select_top(void) {
    consoleSelect(&topConsole);
}

void ui_select_bottom(void) {
    consoleSelect(&bottomConsole);
}
