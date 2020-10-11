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
static const char MESSAGE[] = "Hello, World!\n";

//static void listen_cb(struct evconnlistener *, evutil_socket_t ,struct sockaddr *,int socklen,void *);
static void
listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
            struct sockaddr *sa, int socklen, void *user_data);
static  void signal_cb(evutil_socket_t,short ,void *);
static void conn_readcb(struct bufferevent *bev, void *user_data);
static void
conn_writecb(struct bufferevent *bev, void *user_data);

static void
conn_eventcb(struct bufferevent *bev, short events, void *user_data);
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

    struct evconnlistener *lis =  evconnlistener_new_bind(base,listener_cb,(void *)base,LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE,-1,(struct sockaddr *)&sin,sizeof(sin));
    if (!lis){
        perror("");
        return -1;
    }
    // 创建信号触发节点
  struct event *signal_event = evsignal_new(base,SIGINT,signal_cb,(void *)base);
    // 循环监听
    event_base_dispatch(base);
    event_free(signal_event);
    event_base_free(base);


    return 0;
}

static void
listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
            struct sockaddr *sa, int socklen, void *user_data)
{
    struct event_base *base = (struct event_base *)user_data;
    struct bufferevent *bev;

    //将fd上树
    //新建一个buffervent节点
    bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (!bev) {
        fprintf(stderr, "Error constructing bufferevent!");
        event_base_loopbreak(base);
        return;
    }
    //设置回调
    bufferevent_setcb(bev, conn_readcb, conn_writecb, conn_eventcb, NULL);
    bufferevent_enable(bev, EV_WRITE | EV_READ);//设置写事件使能
    //bufferevent_disable(bev, EV_READ);//设置读事件非使能

//    bufferevent_write(bev, MESSAGE, strlen(MESSAGE));//给cfd发送消息 helloworld
}
static void conn_readcb(struct bufferevent *bev, void *user_data)
{
    char buf[1500]="";
    int n = bufferevent_read(bev,buf,sizeof(buf));
    printf("%s\n",buf);
    bufferevent_write(bev, buf,n);//给cfd发送消息



}

static void
conn_writecb(struct bufferevent *bev, void *user_data)
{
    struct evbuffer *output = bufferevent_get_output(bev);//获取缓冲区类型
    if (evbuffer_get_length(output) == 0) {

        //	printf("flushed answer\n");
        //	bufferevent_free(bev);//释放节点 自动关闭
    }
}

static void
conn_eventcb(struct bufferevent *bev, short events, void *user_data)
{
    if (events & BEV_EVENT_EOF) {
        printf("Connection closed.\n");
    } else if (events & BEV_EVENT_ERROR) {
        printf("Got an error on the connection: %s\n",
//               strerror(errno));/*XXX win32*/
    }
    /* None of the other events can happen here, since we haven't enabled
     * timeouts */
    bufferevent_free(bev);
}

static void
signal_cb(evutil_socket_t sig, short events, void *user_data)
{
    struct event_base *base = (struct event_base *)user_data;
    struct timeval delay = { 2, 0 };

    printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

    event_base_loopexit(base, &delay);//退出循环监听
}
