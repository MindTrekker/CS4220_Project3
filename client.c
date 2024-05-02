//I used the code found in section 6.1.4 of the book
//below are my annotations describing line by line what the code does 
//to the best of my understanding
//as such the doccumentation could be considered excesive 
//but it was my intention to see if I could explain each part of the program
//for the sake of my own learning 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <openssl/aes.h>   

//server port is a unique identifier that the socket will be bound to
//as I understand it there are ~65k ports but ports below 1024 are reserved
#define SERVER_PORT 20423

//buffer size is the number of bytes that will be read in per call of the read or write command
#define BUF_SIZE 4096


//defining a 128 byte key
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


//fatal is a function used for desplaying error messages 
void fatal(char *string);

int main(int argc, char **argv) {

  //ints for various purposes, bytes is used to read in data
  //s is the socket, and c represents the connection status
  int c, s, bytes;

  //buff is a string used for displaying info from the recived file
  char buf[BUF_SIZE];
  char decryptBuf[BUF_SIZE];
  //h will store information about the host
  struct hostent *h;

  //and channel will hold information about the port and IP address 
  struct sockaddr_in channel;


  AES_KEY wctx;

  AES_set_decrypt_key(key, 256, &wctx);

  //a check to makes sure that the user provided the correct number of args
  if (argc != 3)
    fatal("Usage: client server-name file-name");

  //gets the hostserver from the argv and checks to see if a server was actually stored in h
  h = gethostbyname(argv[1]);
  if (!h)
    fatal("gethostbyname failed");

  //createds the socket
  s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  //checks to see if the socket was created sucsusfully
  if (s < 0)
    fatal("socket failed");

  //uses channel to get information form the host server h and coppy it 
  memset(&channel, 0, sizeof(channel));
  channel.sin_family = AF_INET;
  memcpy(&channel.sin_addr.s_addr, h->h_addr, h->h_length);
  channel.sin_port = htons(SERVER_PORT);

  //connects to the socket s using channel and checks to see if a connection was formed
  c = connect(s, (struct sockaddr *)&channel, sizeof(channel));
  if (c < 0)
    fatal("Connect failed");

  //gets the specified file
  write(s, argv[2], strlen(argv[2]) + 1);

  FILE *outputfile = fopen("outputfile.txt", "wb"); // Open output file in binary write mode

  if (!outputfile)
    fatal("failed to open output file");

  FILE *outputfileEncrypted = fopen("outputfileencrypted.txt", "wb"); // Open output file in binary write mode
  if (!outputfileEncrypted)
    fatal("failed to open output file");

  /*
  while ((bytes = read(s, buf, BUF_SIZE)) > 0) {
    fwrite(buf, sizeof(char), bytes, outputfileEncrypted); // Write received data to the file
    //decrypt the file
    AES_decrypt(buf, decryptBuf, &wctx);
    fwrite(decryptBuf,sizeof(char),bytes,outputfile);
  }
  */

  while ((bytes = read(s, buf, BUF_SIZE)) > 0) {
        // Decrypt the received data
        char decryptBuf[BUF_SIZE];
        AES_decrypt(buf, decryptBuf, &wctx);
        fwrite(decryptBuf, sizeof(char), bytes, outputfile);
        fwrite(buf,sizeof(char),bytes, outputfileEncrypted);
  }


  fclose(outputfile);
  close(s);
}

//prints a message to console and stops the program with code 1
void fatal(char *string) {
  printf("%s\n", string);
  exit(1);
}