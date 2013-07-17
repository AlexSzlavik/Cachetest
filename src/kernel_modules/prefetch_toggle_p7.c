#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>


MODULE_AUTHOR("Alex Szlavik <aszlavik@uwaterloo.ca>");
MODULE_DESCRIPTION("When this module loads, the DSCR is set to disable Cache prefetching");
MODULE_LICENSE("GPL");

/*
 *  Initilize module
 *  We want to create the proc_fs export
 *  The export should allow either L3 or all of the Cache to be toggled
 */
int init_module() {
    unsigned long long var = 33;
    asm("mtspr 17,%0\n"::"r"(var));
    printk("DSCR set to 33\n");
    return 0;
}

/*
 * Cleanup module
 */
void cleanup_module(){
    asm("mtspr 17,%0"::"r"(0));
    printk("DSCR set to 0\n");
}
