#include <stdio.h>
 int wait_loop0 = 10000;
int wait_loop1 = 6000;
  /* Internet address. */
           struct in_addr {
               unsigned int       s_addr;     /* address in network byte order */
           };
 struct sockaddr {
  unsigned char        sa_len;
  unsigned char sa_family;
  char        sa_data[14];
};
void
udelay_____( int seconds )
{   // this function needs to be finetuned for the specific microprocessor
    int i, j, k;
    for(i = 0; i < seconds; i++)
    {
        for(j = 0; j < wait_loop0; j++)
        {
            for(k = 0; k < wait_loop1; k++)
            {   // waste function, volatile makes sure it is not being optimized out by compiler
                int volatile t = 120 * j * i + k;
                t = t + 5;
            }
        }
    }
}
struct sockaddr_in {
  unsigned char            sin_len;
  unsigned char      sin_family;
  unsigned short       sin_port;
  struct in_addr  sin_addr;
#define SIN_ZERO_LEN 8
  char            sin_zero[8];
};

 struct hostent {
               char  *h_name;            /* official name of host */
               char **h_aliases;         /* alias list */
               int    h_addrtype;        /* host address type */
               int    h_length;          /* length of address */
               char **h_addr_list;       /* list of addresses */
           };
#define SOCK_STREAM	1
#define AF_INET		2	
int wait() {}
unsigned int inet_addr(char *str)
{																	
    int a, b, c, d;
    char arr[4];
    sscanf(str, "%d.%d.%d.%d", &a, &b, &c, &d);
    arr[0] = a; arr[1] = b; arr[2] = c; arr[3] = d;
    return *(unsigned int *)arr;
}
int socketdemo(int argc, char **argv)
{
    int socket_desc;
    struct sockaddr_in server;
    char *message , server_reply[6000];
     char test[100] = {"151.177.53.241"};
	printf("test = %s \n", *test);
printf("test = %x \n", test);
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printk("Could not create socket");
    }
 printf("argv[1] = %s \n", argv[1]); 
    //ip address of www.msn.com (get by doing a ping www.msn.com at terminal)
	//   struct hostent *hlist = gethostbyname(argv[1]);
	// printf("addr   \n" );
    server.sin_addr.s_addr =   inet_addr("151.177.53.241");
    server.sin_family = AF_INET;
    server.sin_port = htons( 80 );
 
    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("connect error");
        return 1;
    }
     
    //Send some data
    message = "GET / HTTP/1.1\r\nHost: exscape.org\r\n\r\n";
 
    if( send(socket_desc , message , strlen(message) , 0) < 0)
    {
        puts("Send failed");
        return 1;
    }
        
    //Receive a reply from the server
    if( recv(socket_desc, server_reply , 6000, 0) < 0)
    {
        puts("recv failed");
    }
    puts("Reply received\n");
 
	
    printk("%s\n",server_reply);
     udelay_____(3);
    return 0;
}

int main(int argc, char **argv)
{
	socketdemo(argc,argv);
}
