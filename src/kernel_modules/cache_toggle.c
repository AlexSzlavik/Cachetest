#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>


MODULE_AUTHOR("Alex Szlavik <aszlavik@uwaterloo.ca>");
MODULE_DESCRIPTION("While the module is loaded, x86 CPU Caches are disabled");
MODULE_LICENSE("GPL");

/*
 *  Initilize module
 *  We want to create the proc_fs export
 *  The export should allow either L3 or all of the Cache to be toggled
 */
int init_module() {
    long long var=0;
    asm volatile ("push %%rax \n\t" \
         "mov %%cr0, %%rax\n\t "
         "or $(1 << 30), %%rax\n\t " 
         "mov %%rax, %%cr0\n\t "
         "wbinvd\n\t "
         "mov %%cr0, %0\n\t "
         "pop %%rax\n\t "
         :"=r"(var):);
    printk("CR0: 0x%016llx\n",var);
    printk("The CPU Cache hirarchy has been disabled.\n");
    return 0;
}

/*
 * Cleanup module
 */
void cleanup_module(){
    long long var=0;
    asm volatile ("push %%rax \n\t"
         "mov %%cr0, %%rax\n\t"
         "and $~(1 << 30), %%rax\n\t"
         "mov %%rax, %%cr0\n\t"
         "mov %%cr0, %0\n\t"
         "invd\n\t"
         "pop %%rax\n\t"
         :"=r"(var):);
    printk("CR0: 0x%016llx\n",var);
    printk("The CPU Cache hirarchy has been ENABLED.\n");
}
