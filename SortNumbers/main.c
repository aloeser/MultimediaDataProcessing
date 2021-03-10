#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

void write_sorted_numbers(const char *filename, int *sorted_numbers, int num_numbers) {
    FILE *fp;
    fp = fopen(filename, "w");
    for (int i = 0; i < num_numbers ; i++) {
        int sorted_number = sorted_numbers[i];
        fprintf(fp, "%d\n", sorted_number);
    }
    fclose(fp);
}

void read_numbers(const char *filename, int *numbers, int *num_numbers) {
    FILE *fp = fopen(filename, "r");
    bool done = false;
    bool negative = false;

    const int state_empty = 0;
    const int state_sign = 1;
    const int state_digit = 2;
    const int state_done = 3;
    int number_state = state_empty;

    int number = 0;
    while (!done) {
        char c = fgetc(fp);
        if (isspace(c)) {
            if (number_state == state_sign) {
                done = true;
            } else if (number_state == state_digit) {
                number_state = state_done;
            }
        } else if (c == '-') {
            if (number_state == state_empty || number_state == state_sign) {
                negative = !negative;
                number_state = state_sign;
            } else {
                number_state = state_done;
                done = true;
            }
        } else if (isdigit(c)) {
            number *= 10;
            number += c - '0';
            number_state = state_digit;
        } else {
            done = true;
            if (number_state == state_digit) {
                number_state = state_done;
            }
        }

        if (number_state == state_done) {
            if (negative) {
                number *= -1;
            }
            *numbers = number;
            numbers++;
            (*num_numbers)++;
            number_state = state_empty;
            negative = false;
            number = 0;
        }
    }
    fclose(fp);
}

int compare_numbers(const void *a, const void *b) {
    return *(int*)a - *(int*)b;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s infile outfile", argv[0]);
        exit(1);
    }

    int *numbers = malloc(sizeof(int) * (1 << 21));
    int num_numbers = 0;
    read_numbers(argv[1], numbers, &num_numbers);
    qsort(numbers, num_numbers, sizeof(int), compare_numbers);
    write_sorted_numbers(argv[2], numbers, num_numbers);
    return 0;
}
