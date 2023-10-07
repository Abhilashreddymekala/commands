#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *inputFile = fopen(argv[1], "r");
    if (inputFile == NULL) {
        printf("Failed to open input file.\n");
        return 1;
    }

    FILE *outputFile = fopen("output.log", "w");
    if (outputFile == NULL) {
        printf("Failed to open output file.\n");
        fclose(inputFile);
        return 1;
    }

    char command[100];
    int allCommandsSuccessful = 1; // Flag to track command execution status
    int lastCommand = 0; // Flag to indicate the last command

    // Read the lines of commands in the input file
    for (int lineNumber = 1; fgets(command, sizeof(command), inputFile) != NULL; lineNumber++) {
        command[strcspn(command, "\n")] = '\0'; // Remove the newline character at the end of the command

        time_t startTime = time(NULL); // Start time

        pid_t pid = fork();
        if (pid < 0) {
            printf("Fork failed.\n");
            fclose(inputFile);
            fclose(outputFile);
            return 1;
        } else if (pid == 0) {
            execlp(command, command, (char *)NULL); // Child process
            // If execlp returns, an error occurred
            printf("Failed to execute command on line %d: %s\n", lineNumber, command);
            exit(1);
        } else {
            // Parent process
            int status;
            waitpid(pid, &status, 0);

            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) == 0) {
                    time_t endTime = time(NULL); // End time

                    // Write the command, start time, and end time to the output file
                    fprintf(outputFile, "%s\t%s\t%s", command, ctime(&startTime), ctime(&endTime));
                    fflush(outputFile);
                } else {
                    printf("Command execution failed on line %d: %s\n", lineNumber, command);
                    allCommandsSuccessful = 0; // Set flag to indicate failure
                }
            } else {
                printf("Command execution failed on line %d: %s\n", lineNumber, command);
                allCommandsSuccessful = 0; // Set flag to indicate failure
            }

            // Check if this is the last command
            if (fgets(command, sizeof(command), inputFile) == NULL) {
                lastCommand = 1;
            }
        }

        if (lastCommand) {
            break; // Exit the loop after the last command execution
        }
    }

    fclose(inputFile);
    fclose(outputFile);

    if (allCommandsSuccessful) {
        printf("All commands executed successfully.\n");
    } else {
        printf("Some commands failed.\n");
    }

    return 0;
}
