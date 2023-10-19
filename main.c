/**
 * @file main.c
 * @author Luke Foster - fost4403@vandals.uidaho.edu
 * @brief This program is designed to mimic a simple shell. It will display a $ prompt to the user and will then accept commands through 
 *        the terminal. The input will then use forks to execute these commands via execvp(). The user can close the shell by typing in
 *        the work "exit".
 * @version 0.1
 * @date 2023-03-06
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

#define EXIT  "exit"
#define MAX   256

int makeArg(char s[], char **args[], int *type);

int findLength(char *scanPtr);

int main(void)
{
    char input[MAX];
    char **argv;

    int argc, type = 0;

    pid_t pid, status, pidW;

    while(1)
    {
        printf("$ ");

        if(fgets(input, MAX, stdin) != NULL) {  
            
            input[strcspn(input, "\n")] = 0; /* Remove the \n added by fgets */

            if(strcmp(EXIT, input) == 0) { /* If exit, close shell*/
                break;
            }

            do {
                argc = makeArg(input, &argv, &type);
                
                pid = fork();

                if(pid == 0) { /* Child process */
                    if(execvp(argv[0], argv) < 0) {
                        fprintf(stderr, "Command failed to execute!\n");
                        exit(1);
                    }
                }

                if(type != -2) { /* Wait if not & modifier */
                    wait(&status);
                }

                if(argc != -1) {
                    int i;
                    
                    for(i = 0; i < argc; i++) {
                        free(argv[i]);
                    }

                    free(argv);
                }
                
            } while(type > 0);   

            type = 0;  
        }
    }

    while((pidW = wait(&status)) > 0 ); /* Wait for all children to finish */

    return 0;
}

/**
 * @brief This function creates tokens in memory using an pointer to an array of char pointers
 * 
 * @param s String to be parsed and broken into tokens
 * @param args pointer to array of char pointers. Used to store the arguments found
 * @return int Number of arguments found
 */
int makeArg(char s[], char ***args, int *type)
{
    int index = *type, numTokens = 0, count = 0;
    int semi = 0;

    char *scanPtr = &s[index];

    while(*scanPtr != '\0') /* Find number of tokens passed with s[] */
    {
        if(*scanPtr != ' ' && *scanPtr != '\t')
        {
            numTokens++;

            while((*scanPtr != ' ' && *scanPtr != '\t') && *scanPtr != '\0')
            {
                scanPtr++;
                count++;
                
                if(*scanPtr == ';' && *(scanPtr + 1) != '\0') { /* Move index forward */
                    *type = *type + count + 1;
                    break;
                }
            }

            if(*type > index) { /* Found a multi argument */
                semi = 1;
                break;
            }
        }
        else
        {
            scanPtr++;
            count++;
        }
    }

    if(numTokens == 0) {
        return -1;
    }

    scanPtr = &s[index]; /* Reset scanner */

    *args = (char **) calloc(numTokens + 1, sizeof(char *)); /* Allocate space for char * and NULL */

    int lenToken, argNum = 0, amp = 0;

    while(argNum < numTokens) /* Create c-strings */
    {
        if(*scanPtr != ' ' && *scanPtr != '\t')
        {
            lenToken = findLength(scanPtr);

            (*args)[argNum] = malloc(sizeof(char) * (lenToken));

            strncpy((*args)[argNum], scanPtr, lenToken);
            (*args)[argNum][lenToken - 1] = '\0';

            argNum++;
        }

        while((*scanPtr != ' ' && *scanPtr != '\t') && *scanPtr != '\0') {
            scanPtr++;

            if(*scanPtr == '&') { /* Determine if & found */
                amp = 1;
            }
        }

        if(*scanPtr != '\0') /* Move one more space*/
            scanPtr++;
    }

    if(semi != 1) { /* Determine what type of execution to occur */
        *type = -1;
    }
    else if(amp) {
        *type = -2;
    }

    return numTokens;
}

/**
 * @brief Determines the length of the found token
 * 
 * @param scanPtr Scanner pointing at current element in the string 
 * @return int Length of the token found
 */
int findLength(char *scanPtr)
{   
    int lenToken = 1;

    char *lengthPtr = scanPtr;

    while((*lengthPtr != ' ' && *lengthPtr != '\t') && *lengthPtr != '\0' && *lengthPtr != ';' && *lengthPtr != '&')
    {
        lengthPtr++;
        lenToken++;
    }

    return lenToken;
}