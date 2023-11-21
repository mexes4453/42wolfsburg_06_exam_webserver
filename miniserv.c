#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#define MAX_BUF_SIZE (120000)
#define MAX_CLIENTS (1024)
#define IP_ADDR_BIN (2130706433)
#define FATAL_ERR_MSG ("Fatal error")
#define ERR_MSG_ARG_CNT ("Wrong number of arguments")

typedef struct s_client
{
    int    id;
    char   msg[1024];
}   t_client;

t_client        clients[MAX_CLIENTS];
fd_set          rFds, wFds, aFds;
int             fdMax = 0, idNext = 0;
char            rBuf[MAX_BUF_SIZE], wBuf[MAX_BUF_SIZE];

/* FUNCTIONS */
void    ft_error(char *str)
{
    if (str) write(2, str, strlen(str));
    else write(2, FATAL_ERR_MSG, strlen(FATAL_ERR_MSG));
    write(2, "\n", 1);
    exit(EXIT_FAILURE);
}


void   ft_sendall(int connfd)
{
    for (int i=0; i<= fdMax; i++)
    {
        if (FD_ISSET(i, &wFds) && i != connfd)
            send(i, wBuf, strlen(wBuf), 0);
    }
}


int main (int argc, char *argv[])
{
    if (argc != 2) ft_error(ERR_MSG_ARG_CNT);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) ft_error(NULL);

    FD_ZERO(&aFds);FD_ZERO(&rFds);FD_ZERO(&wFds);
    bzero(&clients, sizeof(clients));
    bzero(&wBuf, sizeof(wBuf));
    bzero(&rBuf, sizeof(rBuf));

    fdMax = sockfd;
    FD_SET(sockfd, &aFds);

    struct sockaddr_in server_addr;
    socklen_t addr_len;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(IP_ADDR_BIN);
    server_addr.sin_port = htons(atoi(argv[1]));

    if ((bind(sockfd,
              (const struct sockaddr *)&server_addr,
              sizeof(server_addr))
        ) < 0) ft_error(NULL);

    if (listen(sockfd, 10) < 0) ft_error(NULL);
    while(1)
    {
        rFds = wFds = aFds;
        if (select( fdMax + 1, &rFds, &wFds, NULL, NULL) < 0)
            continue;
        for (int fd_idx=0; fd_idx <=fdMax; fd_idx++)
        {
            if (FD_ISSET(fd_idx, &rFds) && fd_idx == sockfd)
            {
                int connfd = accept(sockfd, (struct sockaddr *)&server_addr,
                                             &addr_len);
                if (connfd < 0) continue;
                fdMax = connfd > fdMax ? connfd : fdMax;
                clients[connfd].id = idNext++;
                FD_SET(connfd, &aFds);
                sprintf(wBuf, "server: client %d just arrived\n", clients[connfd].id);
                ft_sendall(connfd);
                break;
            }

            if (FD_ISSET(fd_idx, &rFds) && fd_idx != sockfd)
            {
                int res = recv(fd_idx, rBuf, 65536, 0);
                if (res <= 0)
                {
                    sprintf(wBuf, "server: client %d just left\n", clients[fd_idx].id);
                    ft_sendall(fd_idx);
                    FD_CLR(fd_idx, &aFds);
                    close(fd_idx);
                    break;
                }
                else
                {
                    for (int i = 0, j= strlen(clients[fd_idx].msg); i < res; i++, j++)
                    {
                        clients[fd_idx].msg[j] = rBuf[i];
                        if (clients[fd_idx].msg[j] == '\n')
                        {
                            clients[fd_idx].msg[j] = '\0';
                            sprintf(wBuf, "client %d: %s\n", clients[fd_idx].id, clients[fd_idx].msg );
                            ft_sendall(fd_idx);
                            bzero(&clients[fd_idx].msg, strlen(clients[fd_idx].msg));
                            j = -1;
                        }
                    }
                    break;
                }
            }
        }
    }
    return (0);
}

