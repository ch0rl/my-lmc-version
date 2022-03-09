/*
    my-lmc - a 'replica' of Little Man Computer
    Copyright (C) 2022 reddersc022

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// VS things
#pragma warning(disable : 4996)

/* Config */
#define MEM_SIZE 100    // Number of addressable parts of "memory"
#define LABEL_SIZE 10   // Max size of any labels
#define OP_SIZE 3       // Size of operators (ie., ADD)
#define DATA_SIZE 10    // Max size of the data (after ops)
#define EXTRA_ROOM 20   // Max amount of whitespace in a line
#define REG_NUM 10      // Number of registers
#define LABEL_NUM 10    // Max number of labels
char OPERATORS[][3] = {  // See reference.md
    "HLT", "ADD", "SUB", "STA", "LDA", "BRA", "BRZ", "BRP", "INP", "OUT"
};
size_t OP_NUM = 10;  // Remember to update
size_t LINE_SIZE = LABEL_SIZE + OP_SIZE + DATA_SIZE + EXTRA_ROOM + 3;

// Struct for labels
typedef struct {
    char *name;
    int line_no;
} Label;


/*
    next_line
    ---------
    char *line: Pointer to the line of characters to re-write
    FILE *f: File stream to read from

    returns int: "length" of line read
*/
int next_line(char *line, FILE *f) {
    // Clear line
    for (int i = 0; i < strlen(line); i++) { line[i] = 0; }
    // Get char until newline, eof, or null
    char c = fgetc(f); int i = 0;
    while (c != '\n' && c != EOF && c != 0) {
        strncat(line, &c, 1);
        c = fgetc(f); i++;
    }
    
    return i;
}


/*
    compile
    -------
    int *RAM: Pointer to the simulated RAM
    FILE *file: File stream to read from

    returns int: Exit state (EXIT_FAILURE | EXIT_SUCCESS)
 */
int compile(int *RAM, FILE *file) {
    int ram_i = 0;
    char *line = (char *)malloc(sizeof(char) * LINE_SIZE);
    char *op = (char *)malloc(sizeof(char) * OP_SIZE);
    char *data = (char *)malloc(sizeof(char) * DATA_SIZE);
    Label *labels = (Label *)malloc(sizeof(Label) * LABEL_NUM);
    int line_i;

    // First, loop for labels
    int label_i = 0; int line_n = 0;
    while (next_line(line, file)) {
        line_i = 0;
        // Skip whitespace
        if (line[line_i] == ' ') {
            while (line[line_i++] == ' ') {}
        }

        // If starts with lowercase, is label
        if (line[line_i] >= 'a' && line[line_i] <= 'z') {
            labels[label_i].name = (char *)malloc(sizeof(char) * LABEL_SIZE);
            labels[label_i].line_no = line_n;
            for (int i = 0; i < LABEL_SIZE && line[line_i + i] != ' '; i++) {
                strncat(labels[label_i].name, &line[line_i + i], 1);
            }

            label_i++;
        }

        line_n++;
    }

    // Now compile code
    rewind(file);
    while (next_line(line, file)) {
        // Clear strings
        for (int i = 0; i < DATA_SIZE; i++) { data[i] = 0; }
        for (int i = 0; i < OP_SIZE; i++) { op[i] = 0; }
        line_i = 0;

        // Skip whitespace and labels
        if (line[0] == ' ' || (line[0] >= 'a' && line[0] <= 'z')) {
            while (line[line_i] == ' ' || (line[line_i] >= 'a' && line[line_i] <= 'z')) {
                line_i++;
            }
        }

        // Form op
        for (int i = 0; i < 3; i++) {
            op[i] = line[line_i++];
        }

        // Skip whitespace
        if (line[line_i] == ' ') {
            while (line[line_i++] == ' ') {}
        } line_i--;

        // Form data
        for (int i = 0; line[line_i] != 0 && i < DATA_SIZE; i++) {
            data[i] = line[line_i++];
        }

        // Interpret and save to 'ram'
        // Operator
        bool done = false; int op_n = 0;
        for (; op_n < OP_NUM; op_n++) {
            if (strncmp(op, OPERATORS[op_n], 3) == 0) {
                RAM[ram_i] = op_n * 100000;
                done = true; break;
            }
        }

        if (!done) {
            puts("Unrecognized op:");
            printf("    %s\n", op);
            return EXIT_FAILURE;
        }

        // Data (not if HLT, INP, OUT)
        if (op_n != 0 && op_n != 8 && op_n != 9) {
            // $ and & will always be numeric
            if (data[0] == '$') {
                char *_data = (char *)malloc(sizeof(char) * (strlen(data) - 1));
                for (int i = 1; data[i] != 0; i++) { _data[i-1] = data[i]; }
                RAM[ram_i] += atoi(_data) * 10;
                RAM[ram_i]++;
                free(_data);
            } else if (data[0] == '&') {
                char *_data = (char *)malloc(sizeof(char) * (strlen(data) - 1));
                for (int i = 1; data[i] != 0; i++) { _data[i - 1] = data[i]; }
                RAM[ram_i] += atoi(_data) * 10;
                RAM[ram_i] += 2;
                free(_data);
            } else {
                // Check if label or int
                bool done = false;
                if (data[0] >= 'a' && data[0] <= 'z') {
                    for (int i = 0; i < label_i; i++) {
                        if (strcmp(data, labels[i].name) == 0) {
                            RAM[ram_i] += labels[i].line_no * 10;
                            done = true; break;
                        }
                    }

                    if (!done) {
                        puts("Unrecognized label:");
                        printf("    %s", data);
                        return EXIT_FAILURE;
                    }
                } else {
                    RAM[ram_i] += atoi(data) * 10;
                }
            }
        }

        ram_i++;
    }

    return EXIT_SUCCESS;
}


/* Main */
int main(int argc, char *argv[]) {
    if (argc != 2) {
        puts("Usage: ./assembly <filename>");
        return EXIT_FAILURE;
    }

    // Program stored in file
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        puts("File must exist");
        return EXIT_FAILURE;
    }

    // Load program into 'ram'
    int *RAM = (int *)malloc(sizeof(int) * MEM_SIZE);
    for (int i = 0; i < MEM_SIZE; i++) { RAM[i] = 0; }
    if (compile(RAM, file) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    // Testing
    for (int i = 0; RAM[i] != 0; i++) {
        printf("%d | ", RAM[i]);
    } puts("");

    // Registers
    int PC = 0, MAR, MDR, CIR, ACC = 0;
    int *gp_registers = (int *)malloc(sizeof(int) * REG_NUM);
    for (int i = 0; i < REG_NUM; i++) { gp_registers[i] = 0; }
    // Helpful
    int _op, _data, _flag;
    bool run = true;
    while (run) {
        // Fetch
        MAR = PC++;
        MDR = RAM[MAR];
        CIR = MDR;

        // Decode
        _op = CIR / 100000;
        _data = (CIR % 100000) / 10;
        _flag = CIR % 100000 % 10;

        // Execute
        // Some basic validation
        if (_flag == 0) {
            if (_data >= MEM_SIZE) {
                printf("Address %d is out of bounds\n", _data);
                return EXIT_FAILURE;
            }
        } else if (_flag == 2) {
            if (_data >= REG_NUM) {
                printf("Register %d is out of range\n", _data);
                return EXIT_FAILURE;
            }
        }
        
        switch (_op) {
            case 0:
                run = false; break;
            case 1:
                if (_flag == 1) {
                    ACC += _data;
                } else if (_flag == 2) {
                    ACC += gp_registers[_data];
                } else {
                    ACC += RAM[_data];
                } break;
            case 2:
                if (_flag == 1) {
                    ACC -= _data;
                } else if (_flag == 2) {
                    ACC -= gp_registers[_data];
                } else {
                    ACC -= RAM[_data];
                } break;
            case 3:
                if (_flag == 2) {
                    gp_registers[_data] = ACC;
                } else {
                    RAM[_data] = ACC;
                } break;
            case 4:
                if (_flag == 1) {
                    ACC = _data;
                } else if (_flag == 2) {
                    ACC = gp_registers[_data];
                } else {
                    ACC = RAM[_data];
                } break;
            case 5:
                PC = _data; break;
            case 6:
                if (!ACC) {
                    PC = _data;
                } break;
            case 7:
                if (ACC > 0) {
                    PC = _data;
                } break;
            case 8:
                puts("Input:"); printf("    ");
                scanf(" %d", &ACC); break;
            case 9:
                puts("Output:"); printf("    %d (%c)\n", ACC, (char)ACC);
                break;
        }
    }

    fclose(file);
    free(RAM);
    return EXIT_SUCCESS;
}
