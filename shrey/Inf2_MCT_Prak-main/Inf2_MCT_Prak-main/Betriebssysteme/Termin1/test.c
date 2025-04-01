#include <stdio.h>
#include <pthread.h>
#include <string.h>

 /* Thread -Funktion */
void *example_fct(void *args){
    size_t i;
    struct timespec sleep = { 0, 1000 };

    for(i = 0; i < 100; i++) {
        /* Schleife , die einen "zufälligen" Buchstaben ausgibt.
        Dieser wird anhand der Thread ID bestimmt. */
        putchar('a' + pthread_self () % 26);
        /* Warte ein wenig; 1 microsecunde */
        nanosleep (&sleep , NULL);
    }
    int length = strlen(*(char**)args);
    printf("String length: %d\n", length);
 return NULL;
}

int main(){
    char* name = "User";
    /* Lege zwei Thread -handle an */
    pthread_t threadA , threadB;

    /* Starte zwei Thread mit der auszuführenden Funktion example_fct.
    Konfigurations - und Übergabe -Parameter werden nicht gesetzt , daher NULL. */
    pthread_create (&threadA , NULL , &example_fct , NULL);
    pthread_create (&threadB , NULL , &example_fct , NULL);

    example_fct(&name);

 /* Warte auf Beendigung der beiden Threads */
    pthread_join(threadA , NULL);
    pthread_join(threadB , NULL);

    return 0;
}