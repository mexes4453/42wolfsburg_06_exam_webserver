#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <printf.h>

typedef struct s_clients
{
	int		id;
	char	msg[1024];
} t_clients;

t_clients	clients[1024];
fd_set		readfds, writefds, activefds;
int			fdMax = 0, idNext = 0;
char		bufferRead[120000], bufferWrite[120000];

void	ft_Error(char *str)
{
	if (str)
		write(2, str, strlen(str));
	else
		write(2, "Fatal error", strlen("Fatal error"));
	write(2, "\n", 1);
	exit(1);
}

void sendAll(int connfd)
{
	for (int i = 0; i <= fdMax; i++)
	{
		if (FD_ISSET(i, &writefds) && i != connfd)
			send(i, bufferWrite, strlen(bufferWrite), 0);
	}  
}

int main(int ac, char** av)
{
	if (ac != 2)
		ft_Error("Wrong number of arguments");
	
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		ft_Error(NULL);

	FD_ZERO(&activefds);

	bzero(&clients, sizeof(clients));

	fdMax = sockfd;

	FD_SET(sockfd, &activefds);

	struct sockaddr_in server_addy;
	socklen_t len;
	bzero(&server_addy, sizeof(server_addy));
	server_addy.sin_family = AF_INET;
	server_addy.sin_addr.s_addr = htonl((2130706433));
	server_addy.sin_port = htons(atoi(av[1]));

	if ((bind(sockfd, (const struct sockaddr *)&server_addy, sizeof(server_addy))) < 0)
		ft_Error(NULL);

	if (listen(sockfd, 10) < 0)
		ft_Error(NULL);

	while(1)
	{
		readfds = writefds = activefds;

		if (select(fdMax + 1, &readfds, &writefds, NULL, NULL) < 0)
			continue;
		for (int fdI = 0; fdI <= fdMax; fdI++)
		{
			if (FD_ISSET(fdI, &readfds) && fdI == sockfd)
			{
				int connfd = accept(sockfd, (struct sockaddr*)&server_addy, &len);
				if (connfd < 0)
					continue;
				fdMax = connfd > fdMax? connfd : fdMax;
				clients[connfd].id = idNext++;
				FD_SET(connfd, &activefds);
				sprintf(bufferWrite, "server: client %d just arrived\n", clients[connfd].id);
				sendAll(connfd);
				break;
			}

			if (FD_ISSET(fdI, &readfds) && fdI != sockfd)
			{
                int res = recv(fdI, bufferRead, 65536, 0);
                if (res <= 0)
                {
                    sprintf(bufferWrite, "server: client %d just left\n", clients[fdI].id);
                    sendAll(fdI);
                    FD_CLR(fdI, &activefds);
                    close(fdI);
                    break;
                }
                else
                {
                    for (int i = 0, j = strlen(clients[fdI].msg); i < res; i++, j++)
                    {
                        clients[fdI].msg[j] = bufferRead[i];
                        if (clients[fdI].msg[j] == '\n')
                        {
                            clients[fdI].msg[j] = '\0';
                            sprintf(bufferWrite, "client %d: %s\n", clients[fdI].id, clients[fdI].msg);
                            sendAll(fdI);
                            bzero(&clients[fdI].msg, strlen(clients[fdI].msg));
                            j = -1;
                        }
                    }
                    break;
                }
			}
		}
	}
}