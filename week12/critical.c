#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h> // sleep 을 하기 위해

int count = 0;
pthread_mutex_t mutex;  //pthread_mutex_init() 쓰기 싫으면 
                // pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 라고 선언 해도 됨

void *thread_main(void (*arg)) 
{
    int i;
    int id = *(int *)arg;
    pthread_mutex_lock(&mutex);
    for(i = 0; i < 10000000; i++){
        //pthread_mutex_lock(&mutex); // lock 은 최대한 좁은 공간을 하는 것이 좋음
                                    // for 문 밖에 lock을 걸면 i 가 다 돌 때까지 하나의 thread만 count에 접근 가능
                                    // for 문 안에 lock을 걸면 lock을 걸고 푸는 작업을 반복하여 다른 thread 와 교대로 적절히 접근 가능
                                    // 대신 for 문 안에 있을 때는 매우 오래 걸림(lock도 10000000번 걸고 푸니까). for 문 밖에 있을 때는 순식간에 계산
        count++;    // critical section 임
        printf("id : %d, count : %d\n", id, count);
        //pthread_mutex_unlock(&mutex);
    }
    pthread_mutex_unlock(&mutex);
    return NULL;
}
int main()
{
    pthread_t t_id1, t_id2, t_id3;
    int ret;
    int a = 1, b = 2, c = 3;
    pthread_mutex_init(&mutex, NULL); 

    // arg (a, b, c 를 넣지 않을 때는 NULL 넣으면 됨)
    ret = pthread_create(&t_id1, NULL, thread_main, &a);
    ret = pthread_create(&t_id2, NULL, thread_main, &b);
    ret = pthread_create(&t_id3, NULL, thread_main, &c);

    
    pthread_join(t_id1, NULL);
    pthread_join(t_id2, NULL);
    pthread_join(t_id3, NULL); 

    printf("count : %d\n", count);  // 이론대로라면 count 값이 30000000이 되어야 함
                                    // 그러나 그렇지 않음. 왜냐하면 공유자원에 접근하기 때문
    
    pthread_mutex_destroy(&mutex);

    return 0;
}