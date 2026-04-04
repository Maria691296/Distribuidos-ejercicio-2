#include <unistd.h>
#include <errno.h>
#include "lines.h"

int sendMessage(int socket, char * buffer, int len)
{
	int r;
	int l = len;
		

	do {	
		r = write(socket, buffer, l);
		l = l -r;
		buffer = buffer + r;
	} while ((l>0) && (r>=0));
	
	if (r < 0)
		return (-1);
	else
		return(0);
}

int recvMessage(int socket, char *buffer, int len)
{
	int r;
	int l = len;
		

	do {	
		r = read(socket, buffer, l);
		l = l -r ;
		buffer = buffer + r;
	} while ((l>0) && (r>=0));
	
	if (r < 0)
		return (-1);
	else
		return(0);
}



ssize_t readLine(int fd, void *buffer, size_t n)
{
	ssize_t numRead;
	size_t totRead;
	char *buf;
	char ch;


	if (n <= 0 || buffer == NULL) { 
		errno = EINVAL;
		return -1; 
	}

	buf = buffer;
	totRead = 0;
	
	for (;;) {
        	numRead = read(fd, &ch, 1);

        	if (numRead == -1) {	
            		if (errno == EINTR)
                		continue;
            	else
			return -1;	
        	} else if (numRead == 0) {
            		if (totRead == 0)
                		return 0;
			else
                		break;
        	} else {	
            		if (ch == '\n')
                		break;
            		if (ch == '\0')
                		break;
            		if (totRead < n - 1) {
				totRead++;
				*buf++ = ch; 
			}
		} 
	}
	
	*buf = '\0';
    	return totRead;
}
