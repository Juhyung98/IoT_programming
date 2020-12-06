#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h> // sleep 을 하기 위해

int count = 0;

void *thread_main(void (*arg)) 
{
    int i;
    
    for(i = 0; i < 10000000; i++){
        count++;
    }
    return NULL;
}
int main()
{
    pthread_t t_id1, t_id2, t_id3;
    int ret;

    // int pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
    //                  pthread_attr_t (thread의 속성 지정을 위한 구조체)를 NULL로 설정 --> 디폴트 설정으로 생성
    ret = pthread_create(&t_id1, NULL, thread_main, NULL);
    ret = pthread_create(&t_id2, NULL, thread_main, NULL);
    ret = pthread_create(&t_id3, NULL, thread_main, NULL);

    
    pthread_join(t_id1, NULL);
    pthread_join(t_id2, NULL);
    pthread_join(t_id3, NULL); 

    printf("count : %d\n", count);  // 이론대로라면 count 값이 30000000이 되어야 함
                                    // 그러나 그렇지 않음. 왜냐하면 공유자원에 접근하기 때문

    return 0;
}