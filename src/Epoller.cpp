#include "Epoller.h"
#include "EpollException.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <iostream>

using namespace std;

void Epoller::initEpool(int maxEvents) {
	epollfd = epoll_create(maxEvents);
	if (epollfd == -1) {
		THROW_EXCEPTION(EpollException, "Epoller create error:");
	}
}

std::string Epoller::EpollFormat(std::string fmt, ...) {
	char buff[1024];
	va_list argptr;
	va_start(argptr, fmt);
	vsprintf((char *) buff, fmt.c_str(), argptr);
	va_end(argptr);
	buff[1024 - 1] = 0;
	return buff;
}

int Epoller::socketBind(std::string hostName, int port, int socketType) {
	int listenfd;
	struct sockaddr_in servaddr;
	listenfd = socket(AF_INET, socketType, 0);
	if (listenfd == -1) {
		THROW_EXCEPTION(EpollException, "socket error:");
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	std::string ip = getIpByHost(hostName, port);
	inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr);
	servaddr.sin_port = htons(port);
	if (bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1) {
		THROW_EXCEPTION(EpollException, "bind error:");
	}
	listen(listenfd, 10);
	return listenfd;
}

void Epoller::makeNonBlocking(int sfd) {
	int flags, s;
	flags = fcntl(sfd, F_GETFL, 0);
	if (flags == -1) {
		THROW_EXCEPTION(EpollException, "fcntl F_GETFL fail");
	}

	flags |= O_NONBLOCK;
	s = fcntl(sfd, F_SETFL, flags);
	if (s == -1) {
		THROW_EXCEPTION(EpollException, "fcntl F_SETFL fail");
	}
}

/**
 * 事件监听loop ，核心部分
 */ 
void Epoller::working(int maxEvents, int timeout) {
	struct epoll_event* events = (epoll_event*) calloc(maxEvents, sizeof(struct epoll_event));
	while (true) {
		int nfds = epoll_wait(epollfd, events, maxEvents, timeout);
		if (nfds == -1) {
			THROW_EXCEPTION(EpollException, "epoll_wait error");
		}
		for (int i = 0; i < nfds; ++i) {
			if (events[i].data.ptr) {
				EpollCallback* context = (EpollCallback*) events[i].data.ptr;
				context->doEvent(&events[i]);
			}
		}
	}
	free(events);
}

int Epoller::initServer(EpollCallback* cb, std::string hostName, int port, int socketType) {
	int listen_sock = socketBind(hostName, port, socketType);
	makeNonBlocking(listen_sock);
	if (!addEvent(listen_sock, EPOLLIN | EPOLLET, cb)) {
		THROW_EXCEPTION(EpollException, EpollFormat("addEvent fail: %s", gai_strerror(errno)));
	}
	return listen_sock;
}

int Epoller::connectServer(EpollCallback* cb, std::string hostName, int port, int socketType) {
	int sockfd = socket(AF_INET, socketType, 0);
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	std::string ip = getIpByHost(hostName, port);
	inet_pton(AF_INET, ip.c_str(), &servaddr.sin_addr);
	servaddr.sin_port = htons(port);
	int rt = connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
	if (rt != 0) {
		THROW_EXCEPTION(EpollException, EpollFormat("connect: %s", gai_strerror(errno)));
	}
	addEvent(sockfd, EPOLLIN, cb);
	return sockfd;
}

bool Epoller::addEvent(int fd, int state, EpollCallback* cb) {
	struct epoll_event event;
	event.events = state;
	event.data.ptr = cb;
	int rt = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
	return rt == 0;
}

bool Epoller::removeEvent(int fd, int state) {
	struct epoll_event ev;
	ev.events = state; //EPOLLIN | EPOLLET; //EPOLLOUT
	return epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev) == 0;
}

bool Epoller::modifyEvent(int fd, int state, EpollCallback* cb) {
	struct epoll_event event;
	event.events = state;
	event.data.ptr = cb;
	return (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event) == 0);
}

void* get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET)
		return &(((struct sockaddr_in*) sa)->sin_addr);
	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

std::string Epoller::getIpByHost(std::string hostName, int port) {
	struct addrinfo hints;
	struct addrinfo *result;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int rt = getaddrinfo(hostName.c_str(), EpollFormat("%d", port).c_str(), &hints, &result);
	if (rt != 0) {
		THROW_EXCEPTION(EpollException, EpollFormat("getaddrinfo: %s", gai_strerror(errno)));
	}
	if (result == NULL) {
		THROW_EXCEPTION(EpollException, "Could not bind");
	}

	char ip[INET6_ADDRSTRLEN];
	inet_ntop(result->ai_family, get_in_addr(result->ai_addr), ip, sizeof ip);
	freeaddrinfo(result);

	return ip;
}

