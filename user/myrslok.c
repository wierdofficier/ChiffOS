#include <stdio.h>
#define MAX_LEN 128
 
void print_image(FILE *fptr);
 int what_picture;
int image(void)
{
   char *filename;
if(what_picture == 1)
{
    filename = "image.txt";
   what_picture = 0;
}
else
    {
  filename = "image2.txt";
what_picture = 1;
	}		
    FILE *fptr = NULL;
 	
    if((fptr = fopen(filename,"r")) == NULL)
    {
        fprintf(stderr,"error opening %s\n",filename);
        return 1;
    }
 
    print_image(fptr);
 
    fclose(fptr);
 
    return 0;
}
 
void print_image(FILE *fptr)
{
    char read_string[MAX_LEN];
 
    while(fgets(read_string,sizeof(read_string),fptr) != NULL)
	{
        	printf("%s",read_string);
		
	}
 
}

int main()
{
while(1)
{
	//image();
 //memset((void *)(unsigned int )0xfd000000 ,0, (0x20) * (0x20) * 0x20*444);
      /// sleep(10);
}
}
