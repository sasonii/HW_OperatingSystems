#include <stdlib.h>

int main(int argc, char **argv) {
    void *ptr = malloc(atoi(argv[1]));
    if (!ptr){
            return 1;
    }
    free(ptr);
    return 0;
}
