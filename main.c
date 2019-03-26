#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <ctype.h>
#include <unistd.h>

// The array will store the arguments
static char* arguments[512];
pid_t p;
int pipeline[2];

#define READ  0
#define WRITE 1

/*
 * This will handle the commands separatly
 */

static int handleCommand(int userInput, int fCommand, int lCommand)
{
    int pipeArr[2]; // will hold the pipes
    
    // will start the pipe
    pipe(pipeArr);
    p = fork();
    
    
    if (p == 0) {
        if (fCommand == 1 && lCommand == 0 && userInput == 0) {
            // This will handle the first command
            dup2(pipeline[WRITE], STDOUT_FILENO);
        } else if (fCommand == 0 && lCommand == 0 && userInput != 0) {
            // This will handle the middle command
            dup2(userInput, STDIN_FILENO);
            dup2(pipeArr[WRITE], STDOUT_FILENO);
        } else {
            // This will handle the last command
            dup2(userInput, STDIN_FILENO );
        }
        
        if (execvp(arguments[0], arguments) == -1)
            _exit(EXIT_FAILURE); // This will occur if the child happens to fail
    }
    
    if (userInput != 0)
        close(userInput);
    

    close(pipeArr[WRITE]);
    
    // Nothing should be read if this is the last command
    if (lCommand == 1)
        close(pipeArr[READ]);
    
    return pipeArr[READ];
}

//This will clean the processes
static void clean(int n)
{
    for (int i = 0; i < n; ++i)
        wait(NULL);
}

static int runProg(char* command, int userInput, int fCommand, int lCommand);
static char lineArr[1024];
static int cmdX = 0; // This is the number of calls the command initiates

int main()
{
    while (1) {
        // This will allow the user to see to enter a command
        printf("$> ");
        fflush(NULL);
        
        // read in user input
        if (!fgets(lineArr, 1024, stdin))
            return 0;
        
        int x = 0;
        int y = 1;
        
        char* commandAdd = lineArr;
        char* nxt = strchr(commandAdd, '|'); // this will find the first pipeline
        
        while (nxt != NULL) {
            // nxt will point to the next pipeline
            *nxt = '\0';
            x = runProg(commandAdd, x, y, 0);
            
            commandAdd = nxt + 1;
            nxt = strchr(commandAdd, '|'); // will find the next pipeline
            y = 0;
        }
        x = runProg(commandAdd, x, y, 1);
        clean(cmdX);
        cmdX = 0;
    }
    return 0;
}

static void splice(char* command);

static int runProg(char* command, int userInput, int fCommand, int lCommand)
{
    splice(command);
    if (arguments[0] != NULL) {
        if (strcmp(arguments[0], "exit") == 0)
            exit(0);
        cmdX += 1;
        return handleCommand(userInput, fCommand, lCommand);
    }
    return 0;
}

static char* ignoreWhiteSpace(char* space)
{
    while (isspace(*space)) ++space;
    return space;
}

static void splice(char* command)
{
    command = ignoreWhiteSpace(command);
    char* nxt = strchr(command, ' ');
    int i = 0;
    
    while(nxt != NULL) {
        nxt[0] = '\0';
        arguments[i] = command;
        ++i;
        command = ignoreWhiteSpace(nxt + 1);
        nxt = strchr(command, ' ');
    }
    
    if (command[0] != '\0') {
        arguments[i] = command;
        nxt = strchr(command, '\n');
        nxt[0] = '\0';
        ++i;
    }
    
    arguments[i] = NULL;
}
// This is the updated comment
