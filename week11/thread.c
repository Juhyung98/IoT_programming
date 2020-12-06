#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h> // sleep 을 하기 위해

void *thread_main(void (*arg)) 
{
    int i;
    int n = *(int *)arg; // arg에 있는 내용을 n에 넣으려는데, 그 값이 int 형임을 알기 때문에 (int *)로 바꿔줌 
    for(i = 0; i < n; i++){
        sleep(1);
        printf("Running...\n");
    }
    return NULL;
}
int main()
{
    pthread_t t_id;
    int arg = 10;    // arg 수 많큼 위에서 Running
                     // arg가 10이어도 10번 돌지 않음. main process가 5번 돌고 sleep 하기 때문
    int ret;

    // int pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine) (void *), void *arg);
    //                  pthread_attr_t (thread의 속성 지정을 위한 구조체)를 NULL로 설정 --> 디폴트 설정으로 생성
    ret = pthread_create(&t_id, NULL, thread_main, (void *)&arg);
    if(ret != 0){   // 보통 에러가 나면 -1을 return 하지만, 여기서는 에러코드를 return
        errno = ret; // ret에 에러코드가 있으므로 여기에 그 결과를 넣어줌.
        perror("thread_create");
        return 0;
    }
    sleep(5); // 원래 있던 main thread는 5초 쉬어
    
    printf("End\n");

    return 0;
}