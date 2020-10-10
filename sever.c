#include <event.h>
#include <stdio.h>
#include "wrap.h"

#define MAX_NUM 1024

typedef struct fdEventMap{
    int fd;
    struct event *ev;
}FdEvent;

FdEvent mFdEventArray[MAX_NUM];

void initFdArray(){
    int i;
    for (i =0 ; i < MAX_NUM; i++){
        mFdEventArray[i].fd = -1;
        mFdEventArray[i].ev = NULL;
    }
}

int  addEventFd(int fd, struct event *ev){
    int i ;
    for (i =0; i < MAX_NUM; i++){
        if (mFdEventArray[i].fd < 0){
            break;
        }
    }
    if (i == MAX_NUM){
        return -1;
    }
    mFdEventArray[i].fd = fd;
    mFdEventArray[i].ev = ev;
    return 0;
}

struct event * getEventFd(int fd){
    int i ;

    for (i =0; i < MAX_NUM; i++){
        if(mFdEventArray[i].fd == fd){
            return mFdEventArray[i].ev;
        }
    }
    return NULL;
}

void desmFaEventArra(){
    int i;
    for (i =0 ; i < MAX_NUM; i++){
        if (mFdEventArray[i].fd >0 && mFdEventArray[i].ev){
            close(mFdEventArray[i].fd);
            mFdEventArray[i].fd = -1;
            event_free(mFdEventArray[i].ev);
        }
    }
}

void cfdcb(int cfd,short event,void *arg){
    char buf[1024] = "";
    int n =  Read(cfd,buf,sizeof(buf));
    if (n <= 0){
        perror("Read:");
        close(cfd);
        event_del(getEventFd(cfd));
    }
        printf("-----:%s",buf);
    
}

void lfdcb(int fd , short event,void *arg){
    struct event_base *base = (struct event_base *)arg;
    int cfd = Accept(fd, NULL,NULL);
    // if(cfd > 0 ){
    struct event *ev = event_new(base,cfd,EV_READ | EV_PERSIST,cfdcb,NULL);
    event_add(ev,NULL);
    
        addEventFd(fd,ev);
    // }
}

int main (){

    // 创建
    int fd = tcp4bind(8001,NULL);
    // 监听
    Listen(fd,128);
    // 初始化树根
    struct event_base *base = event_base_new();
    // 初始化数组
    initFdArray();
    // 初始化上述节点
    struct event *ev = event_new(base,fd,EV_READ | EV_PERSIST,lfdcb,base);
    // 上树
    event_add(ev,NULL);
    // 循环监听
    event_base_dispatch(base);
    // 关闭
    desmFaEventArra();
    close(fd);
    event_base_free(base);
    

    return 0;
}