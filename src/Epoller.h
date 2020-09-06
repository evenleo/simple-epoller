#ifndef __EPOLLER_H__
#define __EPOLLER_H__

#include <string>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <memory>

class EpollCallback {
public:
	typedef std::shared_ptr<EpollCallback> Ptr;
	EpollCallback() {};
	virtual ~EpollCallback() {};
	virtual void doEvent(struct epoll_event*) {};
};

class Epoller {
public:
	typedef std::shared_ptr<Epoller> Ptr;
	Epoller() : epollfd(-1) {}
	virtual ~Epoller() { close(epollfd); };
	int initServer(EpollCallback* cb, std::string hostName, int port, int socketType = SOCK_STREAM);
	int connectServer(EpollCallback* cb, std::string hostName, int port, int socketType = SOCK_STREAM);
	int socketBind(std::string hostName, int port, int socketType = SOCK_STREAM);
	std::string getIpByHost(std::string hostName, int port);
	void initEpool(int maxEvents = 1024);

	/**
	 * state:
	 * EPOLLIN ：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；
	 * EPOLLOUT：表示对应的文件描述符可以写；
	 * EPOLLPRI：表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）；
	 * EPOLLERR：表示对应的文件描述符发生错误；
	 * EPOLLHUP：表示对应的文件描述符被挂断；
	 * EPOLLET： 将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的。
	 * EPOLLONESHOT：只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里
	 */
	bool addEvent(int fd, int state, EpollCallback* cb);
	bool removeEvent(int fd, int state);
	bool modifyEvent(int fd, int state, EpollCallback* cb);
	void makeNonBlocking(int sfd);
	void working(int maxEvents, int timeout = 100);
	static std::string EpollFormat(std::string fmt, ...);
	
private:
	int epollfd;
};

#endif /* __EPOLLER_H__ */
