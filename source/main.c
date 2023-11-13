// SPDX-License-Identifier: MIT
//
// SPDX-FileContributor: Adrian "asie" Siekierka, 2023

#include <stdio.h>

#ifdef BLOCKSDS
#include <fatfs.h>
#else
#include <fat.h>
#endif
#include <nds.h>

#include <nds/arm9/dldi.h>

const char *nds_filename;
uint8_t *io_buffer;
int io_buffer_size;
int io_read_offset = 0;

static inline uint32_t my_rand(void)
{
    static uint32_t seed = 0;

    seed = seed * 0xFDB97531 + 0x2468ACE;

    return seed;
}

static inline uint32_t get_ticks(void) {
    return TIMER0_DATA | (TIMER1_DATA << 16);
}

static void benchmark_read(void) {
    FILE *file = fopen(nds_filename, "rb");
    if (file == NULL) {
        printf("\x1b[41mCould not open '%s'!\n", nds_filename);
        return;
    }

    printf("        \x1b[46mTesting reads...\x1b[39m\n");

    char msg_buffer[33];
    int reads_count = 4;
    for (int curr_size = io_buffer_size; curr_size >= 512; curr_size >>= 1) {
        if (curr_size >= 1024*1024) {
            printf("  %3d MiB", curr_size >> 20);
        } else if (curr_size >= 1024) {
            printf("  %3d KiB", curr_size >> 10);
            reads_count <<= 1;
        } else {
            printf("  0.5 KiB");
        }

        uint32_t ticks_start = get_ticks();
        uint32_t reads = 0;
        while (reads < reads_count) {
            fseek(file, ((my_rand() & (~0x1FF)) & 0x7FFFFF) + io_read_offset, SEEK_SET);
            fread(io_buffer, curr_size, 1, file);
            reads++;
        }
        double read_kilobytes = (curr_size / 1024.0) * reads_count;
        uint32_t ticks_end = get_ticks();
	    uint32_t ticks_diff = ticks_end - ticks_start;
        double seconds_time = (ticks_diff) / (double)(BUS_CLOCK >> 8);
        double kilobytes_per_second = read_kilobytes / seconds_time;
        if (kilobytes_per_second >= 1024.0) {
            sprintf(msg_buffer, "%.3f MB/s", kilobytes_per_second / 1024.0);
        } else {
            sprintf(msg_buffer, "%.3f KB/s", kilobytes_per_second);
        }

        printf("\x1b[32D\x1b[%dC\x1b[42m%s\x1b[39m\n", 32 - 2 - strlen(msg_buffer), msg_buffer);
        swiDelay(5000000);
    }

    fclose(file);
}

static void benchmark_write(void) {
    FILE *file = fopen(nds_filename, "r+b");
    int file_offset = 8*1024*1024;
    if (file == NULL) {
        printf("\x1b[41mCould not open '%s'!\n", nds_filename);
        return;
    }

    printf("        \x1b[46mTesting writes...\x1b[39m\n");

    char msg_buffer[33];
    int reads_count = 1024;
    for (int curr_size = 512; curr_size <= io_buffer_size; curr_size <<= 1) {
        if (curr_size > 1*1024*1024) {
            continue;
        } else if (curr_size >= 1024*1024) {
            printf("  %3d MiB", curr_size >> 20);
        } else if (curr_size >= 1024) {
            printf("  %3d KiB", curr_size >> 10);
            reads_count >>= 1;
        } else {
            printf("  0.5 KiB");
        }

        uint32_t ticks_start = get_ticks();
        uint32_t reads = 0;
        while (reads < reads_count) {
            fseek(file, file_offset + ((my_rand() & (~0x1FF)) & 0x1FFFFF) + io_read_offset, SEEK_SET);
            fwrite(io_buffer, curr_size, 1, file);
            reads++;
        }
        double read_kilobytes = (curr_size / 1024.0) * reads_count;
        uint32_t ticks_end = get_ticks();
	    uint32_t ticks_diff = ticks_end - ticks_start;
        double seconds_time = (ticks_diff) / (double)(BUS_CLOCK >> 8);
        double kilobytes_per_second = read_kilobytes / seconds_time;
        if (kilobytes_per_second >= 1024.0) {
            sprintf(msg_buffer, "%.3f MB/s", kilobytes_per_second / 1024.0);
        } else {
            sprintf(msg_buffer, "%.3f KB/s", kilobytes_per_second);
        }

        printf("\x1b[32D\x1b[%dC\x1b[42m%s\x1b[39m\n", 32 - 2 - strlen(msg_buffer), msg_buffer);
        swiDelay(5000000);
    }

    fclose(file);
}

char options[20][33];

static bool run_menu(int options_count, int *selection) {
    int last_selection = -1;
    int max_option_width = 0;
    for (int i = 0; i < options_count; i++) {
        int option_width = strlen(options[i]);
        if (max_option_width < option_width) max_option_width = option_width;
    }
    int menu_left = ((30 - max_option_width) >> 1) - 1;

    while (1) {
        if (last_selection != *selection) {
            printf("\x1b[2J"); // Clear console
            for (int i = 0; i < options_count; i++) {
                printf("\x1b[%dC\x1b[46m%c\x1b[39m %s\n", menu_left, i == *selection ? '>' : ' ', options[i]);
            }
        }

        swiWaitForVBlank();
        scanKeys();
        uint32_t keys_down = keysDown();
        if(keys_down & KEY_A) return true;
        if(keys_down & (KEY_B | KEY_START)) return false;
        if(keys_down & KEY_UP) (*selection)--;
        if(keys_down & KEY_DOWN) (*selection)++;
        if((*selection) < 0) *selection = 0;
        if((*selection) >= options_count) *selection = options_count-1;
    }
}

static void press_start_to_continue(void) {
    printf("\x1b[39m\n");
    printf("Press START to continue\n");

    while (1) {
        swiWaitForVBlank();

        scanKeys();

        uint32_t keys_down = keysDown();
        if (keys_down & KEY_START)
            break;
    }
}

int main(int argc, char **argv) {
    consoleDemoInit();

    printf("\x1b[2J"); // Clear console

    //      01234567890123456789012345678901
    printf("  \x1b[43m*\x1b[39m SD file system benchmark \x1b[43m*\x1b[39m\n");
    printf("\x1b[37mDLDI: %s\x1b[39m\n\n", io_dldi_data->friendlyName);

    // set window
    consoleSetWindow(NULL, 0, 3, 32, 25);

    if (argc <= 0 || argv[0][0] == 0) {
        printf("\x1b[41mCould not find argv!\n");
        goto exit;
    }

    io_buffer_size = 2*1024*1024;
    io_buffer = malloc(io_buffer_size);
    if (io_buffer == NULL) {
        printf("\x1b[41mOut of memory!\n");
        goto exit;
    }

    fatInitDefault();
    nds_filename = argv[0];

    TIMER0_DATA = 0;
    TIMER1_DATA = 0;
    TIMER0_CR = TIMER_ENABLE | TIMER_DIV_256;
    TIMER1_CR = TIMER_ENABLE | TIMER_CASCADE;

    int options_count;
    int selection = -1;

    do {
        switch (selection) {
            case 0: printf("\x1b[2J"); benchmark_read(); press_start_to_continue(); break;
            case 1: printf("\x1b[2J"); benchmark_write(); press_start_to_continue(); break;
            case 2: REG_EXMEMCNT ^= (1 << 15); break;
            case 3:
                if (io_read_offset == 0) io_read_offset = 1;
                else if (io_read_offset >= 256) io_read_offset = 0;
                else io_read_offset <<= 1;
                break;
        }

        snprintf(options[0], 33, "Benchmark reads");
        snprintf(options[1], 33, "Benchmark writes");
        snprintf(options[2], 33, "RAM priority: %s", (REG_EXMEMCNT & (1 << 15)) ? "ARM7" : "ARM9");
        snprintf(options[3], 33, "Byte offset: %d", io_read_offset);
        options_count = 4;
    } while (run_menu(options_count, &selection));

exit:
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
