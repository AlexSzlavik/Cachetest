#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define iterations 20

int main(int argc, char** argv) {
    int fp;
    unsigned long long addr = 0x400000;
    unsigned long long data = 0;
    int page_size = getpagesize();
    int page_offset = 0;
    system("cat /proc/buddyinfo > out1");
    char *file = malloc((1<<30));
    system("cat /proc/buddyinfo > out2");
    //char file[1<<20];
    memset(file,0xFF,(1<<20));

    while(page_size!=1) {
        page_size /= 2;
        page_offset += 1;
    }

    addr = (unsigned long long)file;
    addr = addr >> page_offset;
    addr = addr * 8;

    printf("Buffer at: 0x%p\n",file);
    printf("Page Size/Offset: %d/%d\n",getpagesize(),page_offset);
    printf("Pid is: %d\n",getpid());
    printf("Seeking: 0x%08llx\n",addr);
    read(STDIN_FILENO,file,1);

    snprintf(file,sizeof(char)*100,"/proc/%d/pagemap",getpid());
    printf("openeing: %s\n",file);
    if((fp = open(file,O_RDONLY,NULL)) < 0) {
        perror("Open");
        exit(1);
    }
    if(lseek(fp,addr,SEEK_SET) < 0) {
        perror("Seek:");
        exit(1);
    }

    int i;
    for(i=0;i<iterations;i++){
        if(read(fp,&data,sizeof(data)) < 0) {
            perror("read:");
            exit(1);
        }
        printf("Final: 0x%016llx\n",data);
    }

    close(fp);
}
