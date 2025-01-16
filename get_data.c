#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <output_file.txt>\n", argv[0]);
        return 1;
    }

    // Get the output file name from the command-line argument
    char *output_file = argv[1];

    // Construct the shell command
    char command[256];
    snprintf(command, sizeof(command), "top -n 100 -d 1 -b > %s", output_file);

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
