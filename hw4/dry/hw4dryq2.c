#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    int size = atoi(argv[1]);
	
getpid();
    char* buffer = (char*)malloc(size);
getpid();	
    if (buffer == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    free(buffer);

    return 0;
}	