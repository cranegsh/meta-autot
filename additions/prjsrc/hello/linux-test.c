#include <stdio.h>

int main(int argc, char *argv[])
{
    FILE *fp;

    printf("Hello World!\n");
    
    printf("Doing basic file write test...\n");
    fp = fopen("/tmp/test.txt", "w+");
    fprintf(fp, "This is testing for fprintf...\n");
    fputs("This is testing for fputs...\n", fp);
    fclose(fp);
    
    printf("All done!\n");
    
    return 0;
}
