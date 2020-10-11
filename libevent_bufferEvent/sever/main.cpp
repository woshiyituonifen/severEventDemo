#include <iostream>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/listener.h>
#include <event2/event.h>
#include <event.h>
//#include <netinet/in.h>
#include <arpa/inet.h>



static  const int PORT = 8002;
int main() {
    // 创建event_base根节点
    struct event_base *base = event_base_new();
    if (!base)
    {
        perror("");
        return 1;
    }
    struct sockaddr_in sin;
    memset(&sin,0,sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);

    struct evconnlistener *lis =  evet_connlistener_new_bind(base,listen_cb,(void *)base,LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,-1,(struct sockaddr *)&sin,sizeof(sin));
    if (!lis){
        perror("");
        return -1;
    }
    // 创建信号触发节点
    evsignal_new(base,SIGINT,signal_cb,(void *)base);
    event_base_dispatch(base);




    return 0;
}