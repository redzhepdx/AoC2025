
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX

#include "../header/nob.h"

typedef struct {
    char** items;
    size_t capacity;
    size_t count;
} InputData;

#define LINE_MAX_LEN 1024

InputData read_lines(const char* filename) {
    InputData input_data = {0};
    FILE* fp = fopen(filename, "r");

    if (!fp) return input_data;

    char buffer[LINE_MAX_LEN];

    while (fgets(buffer, LINE_MAX_LEN, fp)) {
        // Trim newline
        buffer[strcspn(buffer, "\n")] = '\0';
        da_append(&input_data, nob_temp_strdup(buffer));
    }

    fclose(fp);
    return input_data;
}

int solve(InputData dials) {
    int current_dial = 50;
    int click = 0;

    for (size_t i = 0; i < dials.count; ++i) {
        char* dial = dials.items[i];
        char dial_type = dial[0];
        char* dial_value_str = dial + 1;
        int dial_value = atoi(dial_value_str);

        if (dial_type == 'R') {
            current_dial += dial_value;
        } else {
            current_dial -= dial_value;
        }

        current_dial %= 100;

        if (current_dial == 0) {
            click++;
        }
    }

    return click;
}

int solve_part_2(InputData dials) {
    int current_dial = 50;
    int click = 0;

    for (size_t i = 0; i < dials.count; ++i) {
        char* dial = dials.items[i];
        char dial_type = dial[0];
        char* dial_value_str = dial + 1;

        int dial_value = atoi(dial_value_str);
        int prev_dial = current_dial;

        if (dial_type == 'R') {
            current_dial += dial_value;
        } else {
            current_dial -= dial_value;
        }

        if (current_dial >= 100) {
            click += (int)(current_dial / 100);
        }

        if (current_dial <= 0) {
            if (!(prev_dial == 0 && current_dial > -100)) {
                click += (int)abs(current_dial / 100) + (int)(prev_dial != 0);
            }
        }

        current_dial %= 100;
        if (current_dial < 0) {
            current_dial += 100;
        }
    }

    return click;
}

int main() {
    char* input_file = "inputs/q1_input.txt";
    InputData input_lines = read_lines(input_file);
    
    int password = solve(input_lines);
    int password = solve_part_2(input_lines);

    printf("Password : %d\n", password);

    da_free(input_lines);
    return 0;
}