#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#include "display.h"

#define PORT 55934
#define IP "192.168.7.2"
// Write to display over TCP
// See file main in liquidcrysal folder


static void LINUX_runCommand(char* command)
{
    // Execute the shell command (output into pipe)
    FILE *pipe = popen(command, "r");
    // Ignore output of the command; but consume it
    // so we don't get an error when closing the pipe.
    char buffer[1024];
    while (!feof(pipe) && !ferror(pipe)) {
        if (fgets(buffer, sizeof(buffer), pipe) == NULL)
            break;
        // printf("--> %s", buffer);  // Uncomment for debugging
    }
    // Get the exit code from the pipe; non-zero is an error:
    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0) {
        perror("Unable to execute command:");
        printf("  command:   %s\n", command);
        printf("  exit code: %d\n", exitCode);
    } 
}

void DISPLAY_write(char* inputStr){
    
    struct sockaddr_in serverAddr = {0};

    //Create socket file descriptor
    int socketID = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketID < 0){
        perror("ERROR: socket creation failed\n");
        exit(EXIT_FAILURE);
    }

//
    serverAddr.sin_family = AF_INET; 
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT); 

    int len = sendto(socketID,inputStr, strlen(inputStr), 0,(const struct sockaddr *) & serverAddr, sizeof(serverAddr));

    if(len < 0){
        perror("ERROR: Failed to send to server\n");
    }   

    close(socketID);
}

void DISPLAY_writeAll(char inputStr[DISPLAY_ROWS][DISPLAY_COLUMNS]){
    //clear previous strings
    DISPLAY_write("clear");

    sleep(1);

    char inputStrJoined[(DISPLAY_COLUMNS*2)+1];

    strcpy(inputStrJoined,inputStr[0]);
    strcat(inputStrJoined, "+");
    strcat(inputStrJoined, inputStr[1]);

    DISPLAY_write(inputStrJoined);
    
}

void DISPLAY_initialize(){
    LINUX_runCommand("config-pin p9.17 i2c");
    LINUX_runCommand("config-pin p9.18 i2c");
}
void DISPLAY_cleanUp(){

}