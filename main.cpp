#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include "IOhandle.h"
#include "ProcessPool.h"
using std::string;


void setnonblocking(int sockfd)
{
    int opts = fcntl(sockfd,F_GETFL);
    if(opts < 0)
    {
        perror("fcntl(sockfd,F_GETFL)\n");
        exit(1);
    }

    opts = (opts | O_NONBLOCK);
    if(fcntl(sockfd,F_SETFL,opts) < 0)
    {
        perror("fcntl(sockfd,F_SETFL)\n");
        exit(1);
    }
}
int EpollListen(uint16_t Port) {
    struct sockaddr_in serveraddr,clientaddr;
    socklen_t socklen = sizeof(clientaddr);
    struct epoll_event ev,events[100];//ev用于注册事件,数组用于回传要处理的事件
    int epollfd = epoll_create(1000);
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(0 > listenfd)
    {
        perror("socket");
        return -1;
    }
    setnonblocking(listenfd);
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port=htons(Port);
    ev.events = EPOLLIN|EPOLLET;//文件描述符可以读|ET边缘触发模式
    ev.data.fd = listenfd;
    //注册epoll事件
    epoll_ctl(epollfd,EPOLL_CTL_ADD,listenfd,&ev);
    //设置套接字选项避免地址使用错误
    int on=1;
    if((setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)))<0)
    {
        perror("setsockopt");
        return -1;
    }
    //绑定
    if(0 > bind(listenfd,(struct sockaddr *)&serveraddr, sizeof(serveraddr)))
    {
        perror("bind");
        return -1;
    }
    std::cout<<"bind success"<<std::endl;
    //监听
    if(0 > listen(listenfd,1000))
    {
        perror("listen");
        return -1;
    }
    std::cout<<"listen success"<<std::endl;
    while(1) {
        int nfds =  epoll_wait(epollfd,events,100,200);
        for(int i=0;i<nfds;++i) {
            if((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN)))
            {
                close (events[i].data.fd);
                epoll_ctl(epollfd,EPOLL_CTL_DEL,events[i].data.fd,&ev);
                continue;
            }
            else if(events[i].data.fd == listenfd)
            {
                int conn_sock = accept(listenfd,(struct sockaddr *)&clientaddr, &socklen);
                printf("======================<<have a client connect server>>=======================\n");
                setnonblocking(conn_sock);
                ev.events = EPOLLIN|EPOLLET;//设置用于读操作的事件
                ev.data.fd = conn_sock;
                epoll_ctl(epollfd,EPOLL_CTL_ADD,conn_sock,&ev);//注册ev
                continue;
            }
            else
            {
                //读取数据
                // ......
                int fd = events[i].data.fd;
                IOhandle::readMutex.lock();
                IOhandle::Readque.push(fd);
                IOhandle::readMutex.unlock();
                IOhandle::Readcv.notify_one();
            }
        }




    }

}
int main() {
    std::thread t1([](){
        IOhandle::writeThread();
    });
    std::thread t2([](){
        IOhandle::readThread();
    });
    ThreadPool threadPool(10);
    EpollListen(9000);
    t1.join();
    t2.join();
    return 0;
}
