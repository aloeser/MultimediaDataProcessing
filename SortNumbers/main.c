#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

// Disclaimer: If done properly, there should be a .h file with declarations and in here only definitions, but I dont feel its worth the bother for a one file project

/**
 * Simple function to write the sorted numbers
 * @param filename
 * @param sorted_numbers
 * @param num_numbers
 */
void write_sorted_numbers(const char *filename, int *sorted_numbers, int num_numbers) {
    FILE *fp;
    fp = fopen(filename, "w");
    for (int i = 0; i < num_numbers ; i++) {
        int sorted_number = sorted_numbers[i]; // pointers can be used in an array-like syntax
        fprintf(fp, "%d\n", sorted_number);
    }
    fclose(fp);
}

/**
 * function to read whitespace-separated numbers from the given file name.
 * The function reads the input character wise.
 * The main loop for parsing a number is:
 *
 * num = 0
 * while (next char is number)
 *    number = number * 10 + next char
 * write num to the numbers array, num = 0
 *
 * The number of numbers is not known upfront, but tracked in num_numbers, which is given as a pointer (return value would work as well)
 * The array (== pointer) numbers has to be allocated by the functions caller, to avoid having to handle malloc/realloc/free
 * @param filename
 * @param numbers
 * @param num_numbers
 */
void read_numbers(const char *filename, int *numbers, int *num_numbers) {
    FILE *fp = fopen(filename, "r");
    // Input ends either on EOF or when encountering a format error, e.g., 34x, or a minus without a digit ("- ")
    bool done = false;
    bool negative = false;

    const int state_empty = 0; // initial state / whitespace was read, new number has not yet begun
    const int state_sign = 1; // we read a -. Multiple - are allowed, e.g., --5 == 5
    const int state_digit = 2; // we started reading digits - only digits and white spaces may follow after
    const int state_done = 3; // we found a whitespace after a digit - number is done, write it out
    int number_state = state_empty;

    int number = 0;
    while (!done) {
        char c = fgetc(fp); // read single char
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

/*
 * comparison function required for qsort()
 * -1 if a < b
 *  0 if a == b
 *  1 if a > b
 */
int compare_numbers(const void *a, const void *b) {
    return *(int*)a - *(int*)b; // cast the void pointers to int pointers before dereferencing
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s infile outfile", argv[0]);
        exit(1);
    }

    /*
     * allocate an array of sufficient size.
     * wc -l file02.txt claims the file has 1048576 == 2^20 rows
     * It doesnt seem like a line contains multiple numbers in the second file, but just to be sure, allocate twice as much memory (2^21 ints)
     *
     * I would prefer to use int numbers[2 << 21] (automatic memory management, on stack), but that causes a stack overflow as soon as the program starts.
     * Instead, I have to allocate the memory on the heap, using malloc:
     */
    int *numbers = malloc(sizeof(int) * (1 << 21));
    int num_numbers = 0;
    read_numbers(argv[1], numbers, &num_numbers);
    qsort(numbers, num_numbers, sizeof(int), compare_numbers);
    write_sorted_numbers(argv[2], numbers, num_numbers);
    // TODO: maybe I should free the allocated memory from numbers, but the program terminates now anyway, so the OS should do it for me
    return 0;
}
