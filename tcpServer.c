#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define PROTOPORT       6500            /* default protocol port number */
#define QLEN            6               /* size of request queue        */

int     visits      =   0;              /* counts client connections    */
/*------------------------------------------------------------------------
 * Program:   server
 *
 * Purpose:   allocate a socket and then repeatedly execute the following:
 *              (1) wait for the next connection from a client
 *              (2) send a short message to the client
 *              (3) close the connection
 *              (4) go back to step (1)
 *
 * Syntax:    server [ port ]
 *
 *               port  - protocol port number to use
 *
 * Note:      The port argument is optional.  If no port is specified,
 *            the server uses the default given by PROTOPORT.
 *
 *------------------------------------------------------------------------
 */
int main(int argc, char *argv[])
{
        struct  message { int code; char str[132];} msg;
        struct  hostent  *ptrh;  /* pointer to a host table entry       */
        struct  protoent *ptrp;  /* pointer to a protocol table entry   */
        struct  sockaddr_in sad; /* structure to hold server's address  */
        struct  sockaddr_in cad; /* structure to hold client's address  */
        int     sd, sd2;         /* socket descriptors                  */
        int     port;            /* protocol port number                */
        int     alen;            /* length of address                   */
        char    buf[1000];       /* buffer for string the server sends  */
        int     n;               /* number of characters received       */ 
        int optval = 1;          /* options set by setsockopt           */

        int ret;
        int cnt = 1;
        char *resp = "ok!"; /* resposta padrao */
        char *bye = "bye!";

        memset((char *)&sad, 0, sizeof(sad)); /* clear sockaddr structure */
        sad.sin_family = AF_INET;             /* set family to Internet   */
        sad.sin_addr.s_addr = INADDR_ANY;     /* set the local IP address */

        /* Check command-line argument for protocol port and extract    */
        /* port number if one is specified.  Otherwise, use the default */
        /* port value given by constant PROTOPORT                       */

        if (argc > 1) {                 /* if argument specified        */
                port = atoi(argv[1]);   /* convert argument to binary   */
        } else {
                port = PROTOPORT;       /* use default port number      */
        }
        if (port > 0)                   /* test for illegal value       */
                sad.sin_port = htons((u_short)port);
        else {                          /* print error message and exit */
                fprintf(stderr,"Bad port number %s\n",argv[1]);
                exit(1);
        }

        /* Map TCP transport protocol name to protocol number */

        if ( ((int)(ptrp = getprotobyname("tcp"))) == 0) {
                fprintf(stderr, "cannot map \"tcp\" to protocol number");
                exit(1);
        }

        /* Create a TCP socket */
        sd = socket(AF_INET, SOCK_STREAM, 0 /*ptrp->p_proto*/);
        if (sd < 0) {
                fprintf(stderr, "Socket creation failed\n");
                exit(1);
        }

        /* Eliminate "Address already in use" error from bind. */
        if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,
                   (const void *)&optval , sizeof(int)) < 0)
        return -1;


        /* Bind a local address to the socket */
        if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
                fprintf(stderr,"Bind failed\n");
                exit(1);
        }

        /* Specify size of request queue */
        if (listen(sd, QLEN) < 0) {
                fprintf(stderr,"Listen failed\n");
                exit(1);
        }

        /* Main server loop - accept and handle requests */
        while (1) {
           if ( (sd2 = accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
                   fprintf(stderr, "Accept failed\n");
                        exit(1);
           }

           ret = fork (); // agora somos pai e filho


           /* sou o filho ---------------------------------------*/ 
           if (ret == 0) { 
              alen = sizeof(cad); // ?????

              /*n = recv(sd2, *buf, sizeof(msg buf), 0);*/
              while (1 /*n > 0*/)
              {
                  n = recv(sd2, &msg /*buf*/, sizeof(msg /*buf*/), 0);

                  printf ("Worker %d\n",cnt);
                  if (msg.code == 0) {
                    printf ("Apenas echo\n");
                    send(sd2, &msg /*buf*/, sizeof msg /*n*/, 0);
                  }
                  else if (msg.code == 1) {
                    printf ("Serviço\n"); 
                    memcpy((char *)&msg.str, (char *)resp, sizeof resp);
                    send(sd2, &msg /*buf*/, sizeof msg /*n*/, 0);

                  }
                  /* terminei o meu trabalho: fechando e saindo ... */
                  else if (msg.code > 1 || n == 0) {
                    printf ("Worker %d saido...\n", cnt); 
                    memcpy((char *)&msg.str, (char *)bye, sizeof bye);
                    send(sd2, &msg /*buf*/, sizeof msg /*n*/, 0);
                    close (sd2);
                    exit (2);
                  }
              }
           }
           /* fim do filho --------------------------------------*/           

           /* sou o pai */
           cnt++;
        }
}


