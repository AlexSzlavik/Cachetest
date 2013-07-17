/*
 * Example of using hugepage memory in a user application using the mmap
 * system call with MAP_HUGETLB flag.  Before running this program make
 * sure the administrator has allocated enough default sized huge pages
 * to cover the 256 MB allocation.
 *
 * For ia64 architecture, Linux kernel reserves Region number 4 for hugepages.
 * That means the addresses starting with 0x800000... will need to be
 * specified.  Specifying a fixed address is not required on ppc64, i386
 * or x86_64.
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

//#define LENGTH (256UL*1024*1024)
#define LENGTH (9UL*1024*1024)
#define PROTECTION (PROT_READ | PROT_WRITE)

#ifndef MAP_HUGETLB
#define MAP_HUGETLB 0x40000 /* arch specific */
#endif

/* Only ia64 requires this */
//#ifdef __ia64__
#define ADDR (void *)(0x8000000000000000UL)
#define FLAGS (MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB | MAP_FIXED)
//#else
//#define ADDR (void *)(0x0UL)
//#define FLAGS (MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB)
//#endif

static void check_bytes(char *addr)
{
	printf("First hex is %x\n", *((unsigned int *)addr));
}

static void write_bytes(char *addr)
{
	unsigned long i;

	for (i = 0; i < LENGTH; i++)
		*(addr + i) = (char)i;
}

static void read_bytes(char *addr)
{
	unsigned long i;

	check_bytes(addr);
	for (i = 0; i < LENGTH; i++)
		if (*(addr + i) != (char)i) {
			printf("Mismatch at %lu\n", i);
			break;
		}
}

int main(void)
{
	void *addr;
    int page_size = getpagesize();
    int page_offset = 0;
    unsigned long long checkaddr = 0;
    char file[200];
    int fp,fp2,iterations = 0;
    unsigned long long data;
    unsigned long long flags;
    iterations = 10;

    snprintf(file,sizeof(char)*100,"/proc/%d/pagemap",getpid());
    printf("openeing: %s\n",file);
    if((fp = open(file,O_RDONLY,NULL)) < 0) {
        perror("Open");
        exit(1);
    }

    if((fp2 = open("/proc/kpageflags",O_RDONLY,NULL)) < 0) {
        perror("Open");
        exit(1);
    }

    system("cat /proc/meminfo | grep Huge");
    while(page_size!=1) {
        page_size /= 2;
        page_offset += 1;
    }

	addr = mmap(ADDR, LENGTH, PROTECTION, FLAGS, 0, 0);
    checkaddr = (unsigned long long)addr;
    checkaddr = checkaddr >> page_offset;
    checkaddr = checkaddr * 8;
	if (addr == MAP_FAILED) {
		perror("mmap");
		exit(1);
	}

    memset(addr,0x0,LENGTH);

	printf("Returned address is %p\n", addr);
	printf("Checkaddr: 0x%llx\n", checkaddr);
    printf("Buffer at: 0x%p\n",file);
    printf("Page Size/Offset: %d/%d\n",getpagesize(),page_offset);
    printf("Pid is: %d\n",getpid());
    printf("Seeking: 0x%08llx\n",checkaddr);

	check_bytes(addr);
	write_bytes(addr);
	read_bytes(addr);

    int i;
    if(lseek(fp,checkaddr,SEEK_SET) < 0) {
        perror("Seek:");
        exit(1);
    }
    for(i=0;i<iterations;i++){
        if(read(fp,&data,sizeof(data)) <= 0) {
            perror("read pagemap:");
            exit(1);
        }
        /*if(lseek(fp2,data & (unsigned long long)0xFFFFFFFFFFFFF,SEEK_SET) < 0) {
            perror("Seek:");
            exit(1);
        }*/
        /*if(read(fp2,&flags,sizeof(flags)) < 0) {
            perror("read kpageflags:");
            exit(1);
        }*/
        printf("Final: 0x%016llx\n",data);
        //printf("Flags: 0x%016llx\n",flags);
    }

    close(fp);
    close(fp2);

	munmap(addr, LENGTH);

    system("cat /proc/meminfo | grep Huge");

	return 0;
}
