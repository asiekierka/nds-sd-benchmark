// SPDX-License-Identifier: MIT
//
// SPDX-FileContributor: Adrian "asie" Siekierka, 2023

//#include <errno.h>
#include <stdio.h>

#ifdef BLOCKSDS
#include <fatfs.h>
#else
#include <fat.h>
#endif
#include <nds.h>

#include <nds/arm9/dldi.h>

static inline uint32_t my_rand(void)
{
    static uint32_t seed = 0;

    seed = seed * 0xFDB97531 + 0x2468ACE;

    return seed;
}

static inline uint32_t get_ticks(void) {
    return TIMER0_DATA | (TIMER1_DATA << 16);
}

int main(int argc, char **argv) {
    FILE *file = NULL;
    consoleDemoInit();

    printf("\x1b[2J"); // Clear console

    //      012345678901234567890123456sssss78901
    printf("  \x1b[43m*\x1b[39m SD file system benchmark \x1b[43m*\x1b[39m\n");
    printf("\x1b[37mDLDI: %s\x1b[39m\n\n", io_dldi_data->friendlyName);

    if (argc <= 0 || argv[0][0] == 0) {
        printf("\x1b[41mCould not find argv!\n");
        goto exit;
    }

    int buf_size = 2*1024*1024;
    char *buffer = malloc(buf_size);
    if (buffer == NULL) {
        printf("\x1b[41mOut of memory!\n");
        goto exit;
    }

    fatInitDefault();
    file = fopen(argv[0], "rb");
    if (file == NULL) {
        printf("\x1b[41mCould not open '%s'!\n", argv[0]);
        goto exit;
    }

/*    printf("Main RAM: (A) ARM7, (B) ARM9");

    while (1)
    {
        swiWaitForVBlank();

        scanKeys();

        uint32_t keys_down = keysDown();
        if (keys_down & KEY_A) {
            REG_EXMEMCNT |= EXMEMCNT_MAIN_RAM_PRIORITY_ARM7;
            printf(" \x1b[43mA\x1b[39m\n");
            break;
        }
        if (keys_down & KEY_B) {
            REG_EXMEMCNT &= ~EXMEMCNT_MAIN_RAM_PRIORITY_ARM7;
            printf(" \x1b[43mB\x1b[39m\n");
            break;
        }
    } */

    TIMER0_DATA = 0;
    TIMER1_DATA = 0;
    TIMER0_CR = TIMER_ENABLE | TIMER_DIV_256;
    TIMER1_CR = TIMER_ENABLE | TIMER_CASCADE;

    char msg_buffer[33];
    int reads_count = 4;
    for (int curr_size = buf_size; curr_size >= 512; curr_size >>= 1) {
        if (curr_size >= 1024*1024) {
            printf(" %3d MiB", curr_size >> 20);
        } else if (curr_size >= 1024) {
            printf(" %3d KiB", curr_size >> 10);
            reads_count <<= 1;
        } else {
            printf(" 0.5 KiB");
        }

        uint32_t ticks_start = get_ticks();
        uint32_t reads = 0;
        while (reads < reads_count) {
            fseek(file, (my_rand() & (~0x1FF)) & 0x7FFFFF, SEEK_SET);
            fread(buffer, curr_size, 1, file);
            reads++;
        }
        double read_kilobytes = (curr_size / 1024.0) * reads_count;
        uint32_t ticks_end = get_ticks();
        double seconds_time = (ticks_end - ticks_start) / (double)(BUS_CLOCK >> 8);
        double kilobytes_per_second = read_kilobytes / seconds_time;
        if (kilobytes_per_second >= 1024.0) {
            sprintf(msg_buffer, "%.3f MB/s", kilobytes_per_second / 1024.0);
        } else {
            sprintf(msg_buffer, "%.3f KB/s", kilobytes_per_second);
        }

        printf("\x1b[32D\x1b[%dC\x1b[42m%s\x1b[39m\n", 32 - strlen(msg_buffer)-1, msg_buffer);
    }

exit:
    if (file != NULL)
        fclose(file);
    printf("\x1b[39m\n");
    printf("Press START to exit to loader\n");

    while (1)
    {
        swiWaitForVBlank();

        scanKeys();

        uint32_t keys_down = keysDown();
        if (keys_down & KEY_START)
            break;
    }

    return 0;
}
