#include <stdio.h>
#include <stdlib.h>

int global_var = 100;

void function() {
    int local_var = 200;
    printf("Address of local_var: %p\n", &local_var);
}

int main() {
    int stack_var = 20;
    int *heap_var = malloc(sizeof(int));
    *heap_var = 30;


    printf("Address of global_var: %p\n", &global_var);
    printf("Address of stack_var: %p\n", &stack_var);
    printf("Address of heap_var: %p\n", heap_var);

    function();
    
    free(heap_var);
    return 0;
}