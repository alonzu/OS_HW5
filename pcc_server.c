/* this program contains the server side. It gets the server port as an arguement */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define numberOfPrintableChars 127

int stopped = 0;

void handler () {
	stopped = 1; /* Program Got a SIGINT */
}

void sigintPrint (unsigned int * pcc_total) {
	printf("\n");
	for (int ascii=32; ascii<=126; ascii++) {
		printf("char '%c' : %u times\n",ascii,pcc_total[ascii]);
	}
	exit(0);
}

int main(int argc, char *argv[]) {
	struct sigaction Stopper;
	memset(&Stopper, '\0',sizeof(Stopper));
	Stopper.sa_handler = handler;
	sigaction(SIGINT,&Stopper,NULL);

	int connfd, listenfd, nsent, counter, asciiNum;
	int bytes_read = 0, printableCounter = 0;
	unsigned int pcc_total[numberOfPrintableChars];
	memset(pcc_total,0,sizeof(pcc_total));
	char * recv_buff;
	unsigned int length;
	struct sockaddr_in serv_addr;
	struct sockaddr_in peer_addr;
	socklen_t addrsize = sizeof(struct sockaddr_in);
	memset( &serv_addr, 0, addrsize );
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 	// INADDR_ANY = any local machine address
  	serv_addr.sin_port = htons(atoi(argv[1])); // sets the port number from the user
	/* missing arguments error */
	if (argc < 2) {
		fprintf(stderr, "ERROR missing arguments \n");
		exit(-1);
	}
	if (stopped == 1) { /* program got a SIGINT signal */
		sigintPrint(pcc_total);
	}
	/* open a socket */
	listenfd = socket( AF_INET, SOCK_STREAM, 0 );
	if (listenfd < 0) {
		fprintf(stderr, "\n Error : Socket Failed. %s \n", strerror(errno));
  		return -1;
	}
	/* assigning a name to a socket */
  	if(bind(listenfd, (struct sockaddr*) &serv_addr, addrsize) != 0) {
  		fprintf(stderr, "\n Error : Bind Failed. %s \n", strerror(errno));
  		return -1;
  	}
  	/* marking the socket as a passive socket for up to 10 connections */
  	if(listen(listenfd, 10) != 0) {
  		fprintf(stderr, "\n Error : Listen Failed. %s \n", strerror(errno));
  		return -1;
  	}

  	while( 1 ) {
	    /* Accept a connection */
	  	connfd = accept(listenfd, (struct sockaddr*) &peer_addr, &addrsize);
	  	if (stopped == 1) { /* program got a SIGINT signal */
	  		sigintPrint(pcc_total);
	  	}
	  	if(connfd < 0) {
	  		fprintf(stderr, "\n Error : Accept Failed. %s \n", strerror(errno));
	  		return -1;
	  	}
	  	/* get the number of chars to be sent to the server by the client */
	  	counter = 0;
	  	char * buff = (char*)&length;
	  	while (counter < sizeof(length)) {
	  		bytes_read = read(connfd, buff+counter, sizeof(length)); 
	  		counter = counter + bytes_read;
	  		if (bytes_read < 0) {
	  			fprintf(stderr, "\n Error : Read Failed. %s \n", strerror(errno)); 
	  			return -1;
	  		}
	  		if (bytes_read == 0) { /* finished reading from user - user closed the stream */
	  			close(connfd);
	  			break;
	  		}
	  	}
	  	unsigned int length2 = ntohl(length);
	  	buff[counter]='\0';
	  	//printf("\nlength=%u\n", length2);
	  	/* malloc space for "length" number of chars */
	  	recv_buff = (char*)malloc(length2*sizeof(char));
	  	if(recv_buff == NULL) {
	  		fprintf(stderr, "\n Error : malloc Failed. %s \n", strerror(errno));
	  		return -1;
	  	}
	  	/* get the random chars from the client and store them at recv_buff */
		counter = 0;
	  	while (counter < length2) {
	  		bytes_read = read(connfd, recv_buff + counter, sizeof(recv_buff) - 1); 
	  		counter = counter + bytes_read;
		    if (bytes_read < 0) { 
	  			close(connfd);
	  			fprintf(stderr, "\n Error : Read Failed. %s \n", strerror(errno)); 
	  			return -1;
	  		}
	  		if (bytes_read == 0) { /* finished reading from user - user closed the stream */
	  			close(connfd);
	  			recv_buff[counter]='\0';
	  			break;
	  		}
	  	}
	  	//printf("\ngot this chars:\n");
		//printf("\n%s\n",recv_buff);
		/* count printable chars */
	  	printableCounter = 0;
	  	for (int i=0; i < counter; i++) {
	  		asciiNum = (int)recv_buff[i];
	  		//printf("\n ascii=%d \n",asciiNum);
		    if (asciiNum >= 32 && asciiNum <= 126) { /* not a printable char */
	  			//printf("\ncount this\n");
	  			printableCounter++;
	  			pcc_total[asciiNum]++;
	  		}
	  	}
	  	if (stopped == 1) { /* program got a SIGINT signal */
	  		sigintPrint(pcc_total);
	  	}
	  	/* return to client number of printable chars found */		
  		counter = 0;
  		unsigned int printableCounter2 = htonl(printableCounter);
  		buff = (char*)&printableCounter2;
  		while (counter < sizeof(printableCounter2)) {
  			nsent = write(connfd, buff+counter, sizeof(printableCounter2));
      		counter = counter + nsent;
      		if (nsent < 0) {
        		printf("\n Error : Write Failed2. %s \n", strerror(errno));
        		return -1;
      		}
  		}
  		//printf("I sent %u\n", printableCounter2);
  		//printf("I sent %u\n", printableCounter);

		close(connfd);
		if (recv_buff != NULL) {
			free(recv_buff);
		}
	}
}
