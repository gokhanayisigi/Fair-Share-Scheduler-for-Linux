#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    // Get the input and output file names from command-line arguments
    char *input_file = argv[1];
    char *output_file = argv[2];

    // Construct the shell command
    char command[512];
    snprintf(command, sizeof(command), "grep 'u1p' %s | awk '{print $9}' > %s", input_file, output_file);

    // Execute the command
    int ret = system(command);

    // Check for errors during execution
    if (ret == -1) {
        perror("Error executing command");
        return 1;
    }

    printf("Command executed successfully. Results saved in %s\n", output_file);
    return 0;
}
