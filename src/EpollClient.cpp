#include "Epoller.h"
#include "EpollException.h"
#include <iostream>
#include<vector>
#include <arpa/inet.h>

using namespace std;

vector<EpollCallback*> clients;

class TcpClient: public EpollCallback {
public:
	typedef std::shared_ptr<TcpClient> Ptr;
	
	TcpClient(Epoller& epoll) : EpollCallback(),
			epoll(epoll) {

	}

	virtual void doEvent(struct epoll_event* event) {
		cout << "do-----" << endl;
		if (event->events & EPOLLIN) {
			int n = read(clientfd, buff, 1024);
			if (n <= 0) {
				cout << "close " << clientfd << endl;
				close(clientfd);
				epoll.removeEvent(clientfd, EPOLLIN | EPOLLET);
				event->data.ptr=NULL;
				return;
			}
			buff[n] = '\0';
			cout << "fd:" << clientfd << ", read " << buff << endl;
			sleep(3);
			write(clientfd, buff, n);
		}
	}

	void setFd(int fd) { clientfd = fd; }
private:
	Epoller& epoll;
	int clientfd;
	char buff[1024];
	std::string clientIp;
};



int main() {
	try {
		Epoller epoll;
		epoll.initEpool(1024);
		TcpClient::Ptr clientCb = std::make_shared<TcpClient>(epoll);
		int fd = epoll.connectServer(clientCb.get(), "127.0.0.1", 8887);
		clientCb->setFd(fd);
		cout << fd << std::endl;
		char buffer[] = "hello";
		write(fd, buffer, 6);
		epoll.working(10);
		close(fd);
	} catch (EpollException&e) {
		cout << e.displayText() << endl;
	}
	return 0;
}
