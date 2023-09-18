#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>  // inet_pton()

#define ERR_MSG     ("Fatal error...")
#define MAX_BUFSZ   (120000)
#define MAX_CLIENTS (1024)
#define MSG_BUFSZ   (2048)
#define BACKLOGS    (10)
#define TRUE        (1)

typedef struct s_client
{
    int     id;
    char    msg[MSG_BUFSZ];
}           t_client;

/* Global variables */
int         fd_next = 0, fd_max =0;
fd_set      wFds, rFds, active;
char        wBuf[MAX_BUFSZ], rBuf[MAX_BUFSZ];
t_client    clients[MAX_CLIENTS];



/* static functions */
void    ft_error(char *err_msg)
{
    if (err_msg)
    {
        write(2, err_msg, strlen(err_msg));
    }
    else
    {
        write(2, ERR_MSG, strlen(ERR_MSG));
    }
    write(2, "\n", 1);
    exit(EXIT_FAILURE);
}


/* This function is used mainly for debug purpose 
 * It is used to write logs on the server terminal.
 */
void    ft_print(char *msg)
{
    if (msg)
    {
        write(1, msg, strlen(msg));
    }
    else
    {
        write(1, "print_error", 11);
    }
    write(1, "\n", 1);
}


void    ft_sendall(int fd)
{
    for (int fd_idx = 0; fd_idx <= fd_max; fd_idx++ )
    {
        if (FD_ISSET(fd_idx, &wFds) && fd_idx != fd)
        {
            send(fd_idx, wBuf, sizeof(wBuf), 0);
        }
    }
}

int main (int argc, char *argv[])
{
    if (argc != 2) ft_error("Wrong number of arguments");

    int sfd, nfd;
    int read_cnt = 0;
    struct sockaddr_in server_addr;
    socklen_t   addr_len;

    FD_ZERO(&wFds);
    FD_ZERO(&rFds);
    FD_ZERO(&active);
    bzero(wBuf, sizeof(wBuf));
    bzero(rBuf, sizeof(rBuf));
    bzero(clients, sizeof(clients));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(2130706433);
    server_addr.sin_port = htons(atoi(argv[1]));
    addr_len = sizeof(server_addr);


    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) ft_error(NULL);
 
    FD_SET(sfd, &active);
    fd_max = sfd;

    if (bind(sfd, (const struct sockaddr *)&server_addr, addr_len) < 0) ft_error(NULL);

    if (listen(sfd, BACKLOGS) < 0) ft_error(NULL);

    while (TRUE)
    {
        wFds = rFds = active;

        if ( select(fd_max + 1, &rFds, &wFds, NULL, NULL) <= 0) continue;
        for (int fd_idx = 0; fd_idx <= fd_max; fd_idx++ )
        {
            if (FD_ISSET(fd_idx, &rFds) && fd_idx == sfd)
            {
                if (sfd < 0) ft_error(NULL);

                nfd = accept(sfd, (struct sockaddr *)&server_addr, &addr_len);
                if (nfd < 0) continue;

                ft_print(" + New connection created.");
                FD_SET(nfd, &active);
                fd_max = nfd > fd_max ? nfd : fd_max;
                clients[nfd].id = fd_next++;
                sprintf(wBuf, "Server: client %d just arrived\n", clients[nfd].id);
                ft_sendall(nfd);
                break ;
            }
            if (FD_ISSET(fd_idx, &rFds) && fd_idx != sfd)
            {
                read_cnt = recv(fd_idx, rBuf, sizeof(rBuf), 0);
                if (read_cnt <= 0)
                {
                    ft_print(" - One connection closed.");
                    sprintf(wBuf, "Server: client %d just left\n", clients[fd_idx].id);
                    ft_sendall(fd_idx);
                    FD_CLR(fd_idx, &active);
                    close(fd_idx);
                    break ;
                }
                else
                {
                    ft_print(" * Receiving message.");
                    for (int idx = 0; idx < read_cnt; idx++)
                    {
                        clients[fd_idx].msg[idx] = rBuf[idx];
                        if (rBuf[idx] == '\n')
                        {
                            clients[fd_idx].msg[idx] = '\0';
                            break ;
                        }
                    }
                    bzero(wBuf, sizeof(wBuf));
                    sprintf(wBuf, "Client %d: %s\n", clients[fd_idx].id, clients[fd_idx].msg);
                    ft_sendall(fd_idx);
                    break ;
                }
            }
        }
    }
    return (0);
}