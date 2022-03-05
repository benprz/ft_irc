#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

#define IP_ADDRESS "127.0.0.1"

#define IP_PROTOCOL 0
#define TCP_PROTOCOL 6

/*

A socket name in the Internet domain is an Internet address, made up of a 32-bit IP address and a 16-bit port address.

FIONBIO to set sockets I/O as non-blocking

<<<<<<< HEAD
https://ngircd.barton.de/ serveur irc pour check le comportement avec le notre
=======
Sylvain :
voir ce que htons fait exactement
faire cast c++ dans bind(...)

Benjamin :
>>>>>>> a56d3d167d30cb688057d19e888d5516e06e23eb

*/

void    putstr(char *str)
{
    while (*str)
        putchar(*str++);
}

int main(void)
{
    setbuf(stdout, NULL); //pour éviter que printf print les lignes que quand y'a un /n, à la place il print à chaque caractère
    //struct addrinfo addr;
    //in_addr_t addr = inet_addr(IP_ADDRESS);
    protoent *proto = getprotobyname("TCP");
    if (!proto)
        return 0;

    /*  socket */
    int sock = socket(PF_INET, SOCK_STREAM, proto->p_proto);

    /*  bind */
    struct sockaddr_in addr;
    socklen_t   length;

    addr.sin_len = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9649);
    // addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_addr.s_addr = INADDR_ANY;
    bzero(addr.sin_zero, sizeof(addr.sin_zero));
    length = sizeof(addr);
    bind(sock, (const struct sockaddr *)&addr, length);
    //getsockname(sock, (struct sockaddr *)&addr, &length);
    printf("addr=%s | port=%d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

    /* listen */
    listen(sock, 1);

    /* accept */
    socklen_t   addr_len = sizeof(addr);
    int sock2, rval, pw, welcome = 0;
    char buf[1025];

    
    while (1)
    {
        if ((sock2 = accept(sock, (struct sockaddr *)&addr, &addr_len)) == -1)
            break ;
        printf("connexion réussie ! sock2=%d\n", sock2);
        while ((rval = read(sock2, buf, 1024))) // read mais faut utiliser recv(pour plus de contrôle) 
        {
            buf[1024] = 0;
            //write(sock2, buf, 1024); // write mais faut utiliser send (pour plus de contrôle)
            printf("%s", buf);
            bzero(buf, 1024);
        }
        printf("\n");

        struct sockaddr_in addr2;
        socklen_t   addr2_len = sizeof(addr2);

		getsockname(sock2, (struct sockaddr *)&addr2, &addr2_len);
        printf("addr2=%s | port2=%d <-\n", inet_ntoa(addr2.sin_addr), ntohs(addr2.sin_port));
        //connect(sock2, )
        close(sock2);
    }

    /*  connect 
    printf("%d\n", connect(sock, &address, socklen));
*/

    return 0;
}
