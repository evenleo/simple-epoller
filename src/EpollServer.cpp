#include "Epoller.h"
#include "EpollException.h"
#include <iostream>
#include<vector>
#include <arpa/inet.h>
#include <unordered_map>

using namespace std;


class ClientCallback: public EpollCallback {
public:
	typedef std::shared_ptr<ClientCallback> Ptr;
	
	ClientCallback(Epoller& epoll, int clientfd, std::string clientIp) :
			epoll(epoll), clientfd(clientfd), clientIp(clientIp) {

	}

	virtual void doEvent(struct epoll_event* event) {
		if (event->events & EPOLLIN) {
			int n = read(clientfd, buff, 1024);
			if (n <= 0) {
				cout << "close " << clientfd << endl;
				close(clientfd);
				epoll.removeEvent(clientfd, EPOLLIN | EPOLLET);
				ClientCallback* cltCallback = (ClientCallback*)event->data.ptr;
				event->data.ptr=NULL;
				delete cltCallback;
				return;
			}
			buff[n] = '\0';
			cout << "fd: "<< clientfd << ",read " << buff << endl;
			write(clientfd, buff, n);
		}
	}
private:
	Epoller& epoll;
	int clientfd;
	char buff[1024];
	std::string clientIp;
};

class TcpServer: public EpollCallback {
public:
	typedef std::shared_ptr<TcpServer> Ptr;

	TcpServer(Epoller& epoll) : epoll_(epoll), listenfd(-1) {}
	void setListenfd(int listenfd) {
		this->listenfd = listenfd;
	}

	virtual void doEvent(struct epoll_event* event) {
		struct sockaddr_in clientaddr;
		socklen_t clilen;
		int clientfd = accept(listenfd, (sockaddr *)&clientaddr, &clilen);
		if (clientfd < 0) {
			cout << clientfd << std::endl;
			return;
		}
		char *str = inet_ntoa(clientaddr.sin_addr);
		cout << "accept a connection from " << str << endl;
		std::shared_ptr<EpollCallback> client = std::make_shared<ClientCallback>(epoll_, clientfd, str);
		clients_[clientfd] = client;
		epoll_.addEvent(clientfd, EPOLLIN | EPOLLET, client.get());
	}
private:
	Epoller& epoll_;
	int listenfd;
	vector<EpollCallback*> clients;
	std::unordered_map<int, EpollCallback::Ptr> clients_; 
};

int main() {
	try {
		Epoller epoll;
		epoll.initEpool(1024);
		TcpServer::Ptr listenCb = std::make_shared<TcpServer>(epoll);
		int fd = epoll.initServer(listenCb.get(), "0.0.0.0", 8887);	//SOCK_DGRAM
		listenCb->setListenfd(fd);
		cout << fd << std::endl;

		epoll.working(10);
		close(fd);
	} catch (EpollException&e) {
		cout << e.displayText() << endl;
	}
	return 0;
}
