#include <stdio.h>
#include <stdlib.h>
#define MAXLEN 4
int file_size(char* filename)  {
FILE *fp=fopen(filename,"r");
if(!fp) return -1;
fseek(fp,0L,SEEK_END);
int size=ftell(fp);
fclose(fp);
return size;
}
int main(int argc, char *argv[]){
if( argc < 3 )        {
printf("usage: %s %s/n", argv[0], "infile outfile");
exit(1);
}
    FILE * outfile;
    FILE * infile;
    unsigned char *buf; 
    int size = file_size(argv[1]);
    printf(" File: %s  Size: %d bytes \n",argv[1],size);
    //buf = aligned_malloc(size, 8);
    buf = (unsigned char *)(int *)malloc(size + 8);
    int times = (size+3)/4;
    outfile = fopen(argv[2],"w");
    infile  = fopen(argv[1],"rb");
    fread(buf, 1, size, infile);
    printf("+total %d times, %d bytes\n", times, size);
    fclose(infile);
    unsigned int *pp = (unsigned int *)buf;
    for(int i=0; i<times; i++){
    fprintf(outfile, "%08x ", pp[i]);
        if( (i+1)%16 == 0)
        fprintf(outfile, "\n");
        }    
    fclose(outfile);
    return 0;
}
