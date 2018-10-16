#include <stdio.h>
 
  /* Internet address. */
           struct in_addr {
               unsigned int       s_addr;     /* address in network byte order */
           };
  struct sockaddr_in {
               short    sin_family; /* address family: AF_INET */
               short      sin_port;   /* port in network byte order */
               struct in_addr sin_addr;   /* internet address */
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
int socketdemo( )
{
    int socket_desc;
    struct sockaddr_in server;
    char *message , server_reply[6000];
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printk("Could not create socket");
    }
 
    //ip address of www.msn.com (get by doing a ping www.msn.com at terminal)
	// struct hostent *hlist = gethostbyname("exscape.org");
	//printf("addr = %s \n", hlist->h_addr_list[0]);
    server.sin_addr.s_addr = 0xF135B197;;// hlist->h_addr_list[0];
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
     udelay(3);
    return 0;
}

int main()
{
	socketdemo();
}
