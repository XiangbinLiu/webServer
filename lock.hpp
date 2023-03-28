#ifndef LOCKER_H
#define LOCKER_H

#include<pthread.h>
#include<semaphore.h>
#include<exception>
/*
此处将C语言中的sem_t封装成cpp类，名之sem
semaphore.h中的函数很少，可以在文件中了解
到有哪些可用的函数及其用法。
大致上的用法就是，用指针传递信号量，用整数返回值表示执行状态

23/3/27 @Mark
*/ 
class sem{
public:
    sem(){
        if(sem_init(&semaphore,0,0)!=0){// 地址，共享此信号量的进程号，初始值
            throw std::exception();  //excepption是所有异常类的基类
        };  
    }
    sem(int value){
        if(sem_init(&semaphore,0,value)!=0){// 地址，共享此信号量的进程号，初始值
            throw std::exception();  //excepption是所有异常类的基类
        };
    }
    ~sem(){
        sem_destroy(&semaphore);
    }
    bool wait(){  //阻塞直至信号量大于0，之后接触阻塞并且令信号量减一
        return sem_wait(&semaphore)==0;  //返回零表示执行成功
    }
    bool post(){  //令信号量加一
        return sem_post(&semaphore)==0; //返回零表示执行成功
    }

private:
    sem_t semaphore;
};

/*
此处借助pthread_mutex_t实现互斥锁。
逻辑与sem类似，将已经C库中已经有的东西封装起来，返回1表示达到预期。
23/3/27 @Mark
*/ 
class locker{
public:
    locker(){
        if(pthread_mutex_init(&mutex,nullptr)!=0){ 
            // 第二参数为null，不提供死锁检测
            throw std::exception();
        }
    }
    ~locker(){
        pthread_mutex_destroy(&mutex);
    }
    int lock(){
        return pthread_mutex_lock(&mutex)==0; 
        //给 mutex 对象加锁。如果 mutex 已经加锁，尝试加锁的线程会被堵塞，直到 mutex 解锁。该函数返回加锁的 mutex。
    }   
    int unlock(){
        return pthread_mutex_unlock(&mutex)==0;
        //如果当前有被堵塞的线程，解锁后 mutex 将可用，调度器会决定哪个线程获取 mutex。
    }
private:
    pthread_mutex_t mutex;
};


/*
此处主要封装wait, signal, broadcast这三个功能。
23/3/27 @Mark
*/ 
class conditionValue{
public:
    conditionValue(){
        if(pthread_cond_init(&condition, nullptr)!=0){  //设为默认属性
            throw std::exception();
        }
    }
    ~conditionValue(){
        pthread_cond_destroy(&condition);
    }
    bool wait(pthread_mutex_t *mutex)
    {
        int ret = 0;
        pthread_mutex_lock(mutex); //保证只有一个线程查询条件变量
        ret = pthread_cond_wait(&condition, mutex); 
        //内部会有一次解锁和加锁操作,解锁是为了在wait时不耽误别的线程
        pthread_mutex_unlock(mutex);
        return ret == 0;
    }
    bool timewait(pthread_mutex_t *mutex, struct timespec t)
    {  //超时之后，此函数将重新获取互斥量，然后返回错误ETIMEOUT
        int ret = 0;
        pthread_mutex_lock(mutex);
        ret = pthread_cond_timedwait(&condition, mutex, &t);
        pthread_mutex_unlock(mutex);
        return ret == 0;
    }
    int signal(){
        //通知等待的线程条件变量已变为真，唤醒一个
         return pthread_cond_signal(&condition)==0;
    }
    int broadcast(){
        //通知等待的线程条件变量已变为真，唤醒所有
        return pthread_cond_broadcast(&condition)==0;
    }
private:
    pthread_cond_t condition;
}; 

#endif