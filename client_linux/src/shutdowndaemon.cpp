#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>


// client settings
static const int	mServerPort(5555);
static const int	mBufLen(256);


// global variables



int main(int argc, char* argv[]) {

/**********************************
 * Start of Daemon
 * Has to be run in background
 *********************************/
/*
	//unmasking the file mode
	umask(0);

	//set new session
	pid_t sid = setsid();
	if(sid < 0) {
		exit(1);
	}
	
	// Change the current working directory to root.
	if (chdir("/") < 0) {
		log("[error]\tchange dir failed!");
		exit(1);
	}

	// Close stdin. stdout and stderr
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
*/
	

/**********************************
 * Create Socket
 *********************************/

	struct sockaddr_in si_me, si_other;
    unsigned int slen = sizeof(si_other);
    int s, recv_len;
    char buf[mBufLen];
     
    // create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		printf("could not create socket");
        return 1;
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(mServerPort);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	
	// bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) {
        printf("could not bind");
        return 1;
    }
	
	printf("started listening on port 5555");
	
	while (true) {
         
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s, buf, mBufLen, 0, (struct sockaddr *) &si_other, &slen)) == -1) {
			printf("Fehler: recvfrom, fehler code:");
			return 1;
			
        } else {
			printf("%d bytes received!\n", recv_len);
			buf[recv_len] = '\0';

			if (strcmp(buf, "shutdown") == 0) {
				printf("shutdown PC now");
				system("shutdown -h now");
				return 0;
			}
			else if (strcmp(buf, "test") == 0) {
				printf("test packet received");
			}
			else {
				printf("wrong UDP packet received: ");
			}
		}
	}
}
