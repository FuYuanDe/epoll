/*
 *  Description : linux IO多路复用epoll实例
 *  Date        ：20180605
 *  Author      ：mason
 *  Mail        : mrsonko@126.com
 *
 */

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BUFFER_SIZE 512
#define log(fmt, arg...) printf("[udptest] %s:%d "fmt, __FUNCTION__, __LINE__, ##arg)

void main(){
    int fd1, fd2, efd, fds, i, fd;
    int ret, addr_len;
    struct epoll_event g_event;  // epoll事件
    struct epoll_event *epoll_events_ptr; 
    char buffer[BUFFER_SIZE] = {0};
    struct sockaddr_in addr1, addr2;

    // 创建套接字1
    fd1 = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd1 == -1) {
        log("create socket fail \r\n");
        return ;
    }  
    
    // 创建套接字2   
    fd2 = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd2 == -1) {
        log("create socket fail \r\n");
        close(fd1);
        return ;
    }   

    // 设置监听地址,不同套接字监听不同的地址
    addr1.sin_family = AF_INET;
    addr1.sin_addr.s_addr = INADDR_ANY; 
    addr1.sin_port = htons(3500);

    addr2.sin_family = AF_INET;
    addr2.sin_addr.s_addr = INADDR_ANY; 
    addr2.sin_port = htons(3501);
    
    addr_len = sizeof(struct sockaddr_in);
    // 套接字绑定地址
    if (0 != bind(fd1, (struct sockaddr *)&addr1, sizeof(struct sockaddr_in))) {
        log("bind local listening addr fail，errno : %d \r\n", errno);
        goto err;
    }

    if (0 != bind(fd2, (struct sockaddr *)&addr2, sizeof(struct sockaddr_in))) {
        log("bind local listening addr fail，errno : %d \r\n", errno);
        goto err;
    }

    //创建epoll实例
    efd = epoll_create1(0);
    if (efd == -1) {
        log("create epoll fail \r\n");
        goto err;

    }
    log("create epoll instance success \r\n");
    
    epoll_events_ptr = (struct epoll_event *)calloc(2, sizeof(struct epoll_event));
    if (epoll_events_ptr == NULL) {
        log("calloc fail \r\n");
        goto err;
    }

    //添加套接字到epoll中，并监控读事件
    //注意这里传给epoll的参数中可以是指针
    g_event.data.fd = fd1; 
    g_event.events = EPOLLIN;
    epoll_ctl(efd, EPOLL_CTL_ADD, fd1, &g_event);          
    
    g_event.data.fd = fd2; 
    g_event.events = EPOLLIN;
    epoll_ctl(efd, EPOLL_CTL_ADD, fd2, &g_event);  

    //监听epoll事件
    while(1) {
        log("Starting waiting epoll event \n");
        fds = epoll_wait(efd, epoll_events_ptr, 2, -1); //阻塞
        for (i = 0; i<fds; i++)
        {    
            fd = epoll_events_ptr[i].data.fd;
            if (epoll_events_ptr[i].events & EPOLLIN)
            {   
                ret = read(fd, buffer, BUFFER_SIZE);
                if(ret != -1)
                    log("recv msg : %s \n", buffer);
                    
            }     
            memset(buffer, 0, BUFFER_SIZE);
        }        
    }   

    
err:
    close(fd1);
    close(fd2);
    if(epoll_events_ptr) 
        free(epoll_events_ptr);

    return ;

}

