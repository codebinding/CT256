#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define PAGE_SIZE 4096
#define LWHPS2FPGA_BRIDGE_BASE 0xff200000
#define HPS2FPGA_BRIDGE_BASE 0xc0000000
#define OPERAND1_OFFSET 0x0
#define OPERAND2_OFFSET 0x8
#define OPERATOR_OFFSET 0x10
#define RESULT_OFFSET 0x18

volatile uint64_t *operand1, *operand2;
volatile uint64_t *operator;
volatile uint64_t *result;
void *bridge_map;

int main()
{
    int fd;
    off_t bridge_base = HPS2FPGA_BRIDGE_BASE;

    printf("****5/15****\n");
    getchar();

    /* open the memory device file */
    fd = open("/dev/mem", O_RDWR|O_SYNC);
    if (fd < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    /* map the LWHPS2FPGA bridge into process memory */
    bridge_map = mmap(NULL, PAGE_SIZE, PROT_WRITE, MAP_SHARED, fd, bridge_base);
    if (bridge_map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    /* get the registers base address */
    operand1 = (uint64_t *) (bridge_map + OPERAND1_OFFSET);
    operand2 = (uint64_t *) (bridge_map + OPERAND2_OFFSET);
    operator = (uint64_t *) (bridge_map + OPERATOR_OFFSET);
    result = (uint64_t *) (bridge_map + RESULT_OFFSET);

    /* write the value */

    int i = 0;
    //for(i=0; i<100000; i++){
    while(++i){

        *operand1 = (uint64_t)i;
        *operand2 = (uint64_t)i+1;

        *operator = '+';

        printf("%lld + %lld = %lld\n", *operand1, *operand2, *result);

        if( *result != (uint64_t)(i+i) ){

            //perror("incorrect result\n");
        }
    }

    if (munmap(bridge_map, PAGE_SIZE) < 0) {

        perror("munmap");
    }

    close(fd);
    return EXIT_SUCCESS;
}
