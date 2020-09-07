#ifndef __EPOLLER_H__
#define __EPOLLER_H__

// https://www.cnblogs.com/Anker/archive/2013/08/17/3263780.html

#include <string>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <memory>
#include <unordered_map>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>

using namespace std;

#define MAX_PENDING 1024
#define BUFFER_SIZE 1024

class Handler
{
public:
	virtual ~Handler() {}
	virtual int handle(epoll_event e) = 0;
};

class IOLoop
{
public:
	static IOLoop *getInstance()
	{
		static IOLoop instance;
		return &instance;
	}

	void start()
	{
		const uint64_t MAX_EVENTS = 10;
		epoll_event events[MAX_EVENTS];
		for (;;)
		{
			// -1 只没有事件一直阻塞
			int nfds = epoll_wait(this->epfd, events, MAX_EVENTS, -1/*Timeout*/);

			for (int i = 0; i < nfds; ++i)
			{
				int fd = events[i].data.fd;
				Handler *h = handlers[fd];
				if (h)
					h->handle(events[i]);
			}
		}
	}

	void addHandler(int fd, Handler *handler, unsigned int events)
	{
		handlers[fd] = handler;
		epoll_event e;
		e.data.fd = fd;
		e.events = events;

		if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, fd, &e) < 0)
		{
			printf("Failed to insert handler to epoll");
		}
	}

	void modifyHandler(int fd, unsigned int events) 
	{
		struct epoll_event event;
		event.events = events;
		epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
	}

	void removeHandler(int fd) {}

private:
	IOLoop()
	{
		this->epfd = epoll_create(this->EPOLL_EVENTS);

		if (this->epfd < 0)
		{
			printf("Failed to create epoll");
			exit(1);
		}
	}

private:
	int epfd;
	int EPOLL_EVENTS = 10;
	std::unordered_map<int, Handler *> handlers;
};

class EchoHandler : public Handler
{
public:
	EchoHandler(int clientFd, struct sockaddr_in client_addr)
	{
		fd = clientFd;
	}
	virtual int handle(epoll_event e)
	{
		cout << "e.events=" << e.events << endl;
		if (e.events & EPOLLHUP)
		{
			IOLoop::getInstance()->removeHandler(fd);
			return -1;
		}

		if (e.events & EPOLLERR)
		{
			return -1;
		}

		if (e.events & EPOLLOUT)
		{
			if (received > 0)
			{
				cout << "Writing: " << buffer << endl;
				if (send(fd, buffer, received, 0) != received)
				{
					printf("Error writing to socket");
				}
			}

			IOLoop::getInstance()->modifyHandler(fd, EPOLLIN);
		}

		if (e.events & EPOLLIN)
		{
			if ((received = recv(fd, buffer, BUFFER_SIZE, 0)) < 0)
			{
				printf("Error reading from socket");
			}
			else if (received > 0)
			{
				buffer[received] = 0;
				cout << "Reading: " << buffer << endl;
			}

			if (received > 0)
			{
				IOLoop::getInstance()->modifyHandler(fd, EPOLLOUT);
			}
			else
			{
				IOLoop::getInstance()->removeHandler(fd);
			}
		}

		return 0;
	}

private:
	int fd;
	int received = 0;
	char buffer[BUFFER_SIZE];
};

class ServerHandler : Handler
{
public:
	void setnonblocking(int fd)
	{
		int flags = fcntl(fd, F_GETFL, 0);
		fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	}

	ServerHandler(int port)
	{
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));

		if ((fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
		{
			printf("Failed to create server socket");
			exit(1);
		}

		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(port);

		if (bind(fd, (struct sockaddr *)&addr,
				 sizeof(addr)) < 0)
		{
			printf("Failed to bind server socket");
			exit(1);
		}

		if (listen(fd, MAX_PENDING) < 0)
		{
			printf("Failed to listen on server socket");
			exit(1);
		}
		setnonblocking(fd);

		IOLoop::getInstance()->addHandler(fd, this, EPOLLIN);
	}

	virtual int handle(epoll_event e)
	{
		cout << "server events=" << e.events << endl;
		struct sockaddr_in client_addr;
		socklen_t ca_len = sizeof(client_addr);

		int client = accept(fd, (struct sockaddr *)&client_addr,
							&ca_len);

		if (client < 0)
		{
			printf("Error accepting connection");
			return -1;
		}

		cout << "Client connected: " << inet_ntoa(client_addr.sin_addr) << endl;
		Handler* clientHandler = new EchoHandler(client, client_addr);
		IOLoop::getInstance()->addHandler(client, clientHandler, EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLERR);
		return 0;
	}

private:
	int fd;
};



#endif /* __EPOLLER_H__ */
