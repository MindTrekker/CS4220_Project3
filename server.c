//As mentioned in the client server I used the book code from section 6.1.4
//I have annotated it with my explanation of what it does to the best of my ability
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <openssl/aes.h>   

//this was the pid of the first server I ran. 
#define SERVER_PORT 20423
#define BUF_SIZE 4096
#define QUEUE_SIZE 10
#define SUBSTRING_LENGTH 16   
#define MAX_SUBSTRINGS (BUF_SIZE / SUBSTRING_LENGTH) 


//set up the 
static const unsigned char key[] = {
  0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
  0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
  0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f
};

//same as client 
void fatal(char *string);
int createSubstrings(char buf[], char substrings [][SUBSTRING_LENGTH + 1]);

int main(int argc, char *argv[]) {

  //lot more ints this time
  //s is still the socket
  //b checks the bind
  //l checks if the server can listen
  //sa checks if the server can accept a socket connection
  //fd checks if the file can be opened
  //and on is set to one to be used in opening the socket
  int s, b, l, fd, sa, bytes, numStrings, on = 1, i;
  char buff[BUF_SIZE];
  char encryptedSend[BUF_SIZE];
  char encryptedSubstring[SUBSTRING_LENGTH + 1];
  char substrings[MAX_SUBSTRINGS][SUBSTRING_LENGTH + 1];
  struct sockaddr_in channel;


  AES_KEY wctx;
  AES_set_encrypt_key(key, 256, &wctx);
  
  // build address structure to bind to socket, initially similar to client
  //but this time we are not coppying the information as we are the host
  memset(&channel, 0, sizeof(channel));
  channel.sin_family = AF_INET;
  channel.sin_addr.s_addr = htonl(INADDR_ANY);
  channel.sin_port = htons(SERVER_PORT);

  // passive open wait for connection
  s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  //tests socket failure 
  if (s < 0)
    fatal("socket failed");
  //trys to bind
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
  b = bind(s, (struct sockaddr *)&channel, sizeof(channel));
  //tests bind failure
  if (b < 0)
    fatal("bind failed");
  
  //listens 
  l = listen(s, QUEUE_SIZE);
  if (l < 0)
    fatal("listen failed");

  // accept connection
  while (1) {
    sa = accept(s, 0, 0);
    if (sa < 0)
      fatal("accept failed");

    // read from socket 
    read(sa, buff, BUF_SIZE);

    //open the requested file
    fd = open(buff, O_RDONLY);


    if (fd < 0)
      fatal("open failed");

    printf("\nEntering the read loop");
    while (1) {

      printf("\nReading from file");
      bytes = read(fd, buff, BUF_SIZE);


      //clear the 2d array 

      memset(substrings, 0, sizeof(substrings));
      memset(encryptedSend,0,sizeof(encryptedSend));
      //break into substrings here
      numStrings = createSubstrings(buff, substrings);


      for (i = 0; i < numStrings; i++){

        //encrypt the substring
        printf("\nEncrypting...");
        AES_encrypt(substrings[i], encryptedSubstring, &wctx);
        
        //make sure the substring is well terminated
        encryptedSubstring[SUBSTRING_LENGTH] = '\0';

        //add the substring to the encrypted send string to be sent
        strcat(encryptedSend,encryptedSubstring);
      }
       
      printf("\nContents of encryptedSend: %s", encryptedSend);

    
      
      if (bytes <= 0)
        break;
      
      //writes info from requested file to the socket
      write(sa, encryptedSend, bytes);
    }
    close(fd);
    close(sa);
  }
}

void fatal(char *string) {
  printf("%s\n", string);
  exit(1);
}

int createSubstrings(char buf[], char substrings [][SUBSTRING_LENGTH + 1]) {
  int length = strlen(buf);
    int numSubstrings = length / SUBSTRING_LENGTH;

    // Ensure there's space in the array to store all substrings
    if (numSubstrings > MAX_SUBSTRINGS) {
        printf("Error: Exceeded maximum number of substrings\n");
        return 1;
    }
    
    for (int i = 0; i < numSubstrings; i++) {
        strncpy(substrings[i], &buf[i * SUBSTRING_LENGTH], SUBSTRING_LENGTH);
        substrings[i][SUBSTRING_LENGTH] = '\0'; // Null-terminate the substring
    }

    // If there's a leftover part of the string less than 16 characters, store it as the last substring
    if (length % SUBSTRING_LENGTH != 0) {
        strncpy(substrings[numSubstrings], &buf[numSubstrings * SUBSTRING_LENGTH], length % SUBSTRING_LENGTH);

        for (int i = 0; i + (length % SUBSTRING_LENGTH) < SUBSTRING_LENGTH; i ++) {
          substrings[numSubstrings][(length % SUBSTRING_LENGTH) + i] = ' ';
        }
         
        substrings[numSubstrings][SUBSTRING_LENGTH]= '\0';
        numSubstrings++;
    }

    printf("Substrings:\n");
    for (int i = 0; i < numSubstrings; i++) {
        printf("%s\n", substrings[i]);
    }

    return numSubstrings;
}