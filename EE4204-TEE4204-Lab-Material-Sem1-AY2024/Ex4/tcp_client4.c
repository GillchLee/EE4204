/*******************************
tcp_client.c: the source file of the client in tcp transmission 
********************************/

#include "headsock.h"
#include "time.h"	//for random

float str_cli(FILE *fp, int sockfd, long *len);                       //transmission function
void tv_sub(struct  timeval *out, struct timeval *in);	    //calcu the time interval between out and in

int main(int argc, char **argv)
{
	int sockfd, ret;
	float ti, rt;
	long len;
	struct sockaddr_in ser_addr;
	char ** pptr;
	struct hostent *sh;
	struct in_addr **addrs;
	FILE *fp;


	if (argc != 2) {
		printf("parameters not match");
	}

	sh = gethostbyname(argv[1]);	                                       //get host's information
	if (sh == NULL) {
		printf("error when gethostby name");
		exit(0);
	}

	printf("canonical name: %s\n", sh->h_name);					//print the remote host's information
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++)
		printf("the aliases name is: %s\n", *pptr);
	switch(sh->h_addrtype)
	{
		case AF_INET:
			printf("AF_INET\n");
		break;
		default:
			printf("unknown addrtype\n");
		break;
	}
        
	addrs = (struct in_addr **)sh->h_addr_list;
	sockfd = socket(AF_INET, SOCK_STREAM, 0);                           //create the socket
	if (sockfd <0)
	{
		printf("error in socket");
		exit(1);
	}
	ser_addr.sin_family = AF_INET;                                                      
	ser_addr.sin_port = htons(MYTCP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr.sin_zero), 8);
	ret = connect(sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr));         //connect the socket with the host
	if (ret != 0) {
		printf ("connection failed\n"); 
		close(sockfd); 
		exit(1);
	}
	

	ti = str_cli(fp, sockfd, &len);                       //perform the transmission and receiving
	rt = (len/(float)ti);                                         //caculate the average transmission rate
	printf("Time(ms) : %.3f, Data sent(byte): %d\nData rate: %f (Kbytes/s)\n", ti, (int)len, rt);

	close(sockfd);
	fclose(fp);
//}
	exit(0);
}

float str_cli(FILE *fp, int sockfd, long *len)
{
	char *buf;
	long lsize, ci;
	char sends[DATALEN];
	struct ack_so ack;
	int n, slen;
	float time_inv = 0.0;
	struct timeval sendt, recvt;
	ci = 0;

// allocate memory to contain the whole file.
	buf = (char *) malloc (MAXSIZE);
	if (buf == NULL) {printf("Malloc error"); exit (2);}

  // copy the message into the buffer.
	if (fgets(sends, MAXSIZE, fp) ==NULL ) {
		pritnf("error input\n");
	}
	
  /*** the whole file is loaded in the buffer. ***/
	// fgets can add null. buf[lsize] ='\0';									//append the end byte
	gettimeofday(&sendt, NULL);							//get the current time
	
	lsize = strlen(buf);
	
	while(ci<= lsize)
	{
		if ((lsize+1-ci) <= DATALEN)	//datalen is segmenet size
			slen = lsize+1-ci;
		else 
			slen = DATALEN;
		memcpy(sends, (buf+ci), slen);
		n = send(sockfd, &sends, slen, 0);
		if(n == -1) {
			printf("send error!");								//send the data
			exit(1);
		}
		n = recv(sockfd, &ack, 2, 0);
		if ( n > 0 && ack.num == 1 && ack.len == 0 ) // ACK received success                                   //receive the ack
		{
			printf(" ACK received\n");
			ci += slen;
		}
		else {
			printf("error in transmission\n");
			sleep(1);	
		}
	}
		gettimeofday(&recvt, NULL);
	*len= ci;                                                         //get current time
	tv_sub(&recvt, &sendt);                                                                 // get the whole trans time
	time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;
	return(time_inv);
}

void tv_sub(struct  timeval *out, struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) <0)
	{
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}
