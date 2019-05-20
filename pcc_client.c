/* this program contains the client side. It gets the server ip, the server port and the length of the stream as arguements */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

//TODO threads
//TODO SIGINT

int main(int argc, char *argv[]) {
  int nsent, sockfd, fd, nread;
  int counter = 0, bytes_read = 0;
  unsigned int bytesToRead, numOfRandomBytes, numOfPrintable;
  char * send_buff;
  struct sockaddr_in serv_addr; 
  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(atoi(argv[2])); // Note: htons for endiannes
  serv_addr.sin_addr.s_addr = inet_addr(argv[1]); 
  numOfRandomBytes = bytesToRead = atoi(argv[3]); 
  /* missing arguments error */
  if (argc < 4) {
    fprintf(stderr, "ERROR missing arguments \n");
    exit(-1);
  }
  /* open a socket */
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    fprintf(stderr, "\nError : Socket Failed. %s \n", strerror(errno));
    return -1;
  }
  /* connects the socket referred to by the file descriptor sockfd to the address specified by addr */
  if (connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
    printf("\nError : Connect Failed. %s \n", strerror(errno));
    return -1;
  }
  fd = open("/dev/urandom",O_RDONLY); 
  //fd = open("/home/alon/Desktop/eyalon",O_RDONLY); 

  if (fd < 0) {
    printf("\nError : Open /dev/urandom Failed. %s \n", strerror(errno));
    return -1;
  }
  /* malloc space for random chars with a length given from the user */
  send_buff = (char*) malloc ((numOfRandomBytes+1)*sizeof(char)); 
  if (send_buff == NULL) {
    printf("\nError : Malloc Failed. %s \n", strerror(errno));
    return -1;
  }
  /* we start reading random chars from /dev/urandom */
  counter = 0; 
  while (counter < numOfRandomBytes) {
        nread = read(fd, send_buff,bytesToRead); 
        counter = counter + nread;
        if (nread < 0) {
          printf("\n Error : Read Failed. %s \n", strerror(errno)); 
          return -1;
        }
        if (nread == 0) {
            close(fd);
        }
  }
  send_buff[numOfRandomBytes] = '\0';
  /* we will now send the number of Random chars we are going to send to the server */
  counter = 0;
  unsigned int convertedN = htonl(numOfRandomBytes); 
  char * buff = (char*)&convertedN;  
  while (counter < sizeof(convertedN)) {
      nsent = write(sockfd, buff+counter, sizeof(convertedN));
      counter = counter + nsent;
      if (nsent < 0) {
        printf("\nError : Write Failed. %s \n", strerror(errno));
        return -1;
      }
  }

  /* we will now send the chars to the server */
  counter = 0;
  while (counter < numOfRandomBytes) {
    nsent = write(sockfd, send_buff, numOfRandomBytes);
    counter = counter + nsent;
    if (nsent < 0) {
      printf("\nError : Write Failed. %s \n", strerror(errno));
      return -1;
    }
  }
  send_buff[counter] = '\0';
  //printf("\n%s\n", send_buff);

  /* we read the answer with the counter of printable chars from the server */
  buff = (char*)&numOfPrintable;
  counter = 0;
  while (counter < sizeof(numOfPrintable)) {
    bytes_read = read(sockfd, buff+counter, sizeof(numOfPrintable));        
    counter = counter + bytes_read;
    if (bytes_read < 0) {
      fprintf(stderr, "\nError : Read Failed. %s \n", strerror(errno)); 
      return -1;
    }
  }
  if (send_buff != NULL) {
    free(send_buff);
  }
  unsigned int ret_val = ntohl(numOfPrintable);
  printf("# of printable characters: %u\n", ret_val);
  close(sockfd); 
  return 0;
}
