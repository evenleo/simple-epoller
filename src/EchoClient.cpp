#include "Epoller.h"
#include <iostream>
#include <unistd.h>


class TcpClient: public Callback, public std::enable_shared_from_this<TcpClient> {
public:
	typedef std::shared_ptr<TcpClient> ptr;
	TcpClient(std::string ip, int port) 
		: ip_(ip), port_(port), clientfd_(-1)
	{
		epoll_ = std::make_shared<Epoller>();
	}
	void run()
	{
		clientfd_ = epoll_->connectServer(ip_, port_, shared_from_this());
		epoll_->start(10, -1);
	}
	int send(const char* data, int size) { return ::send(clientfd_, data, size, 0);  }
	virtual void doEvent(struct epoll_event* event) 
	{
        int clientfd = event->data.fd;
        if (event->events & EPOLLIN)
        {
			char buff[1024];
            int n = ::read(clientfd, buff, 1024);
			if (n <= 0) {
				std::cout << "close " << clientfd << std::endl;
				::close(clientfd);
				epoll_->removeEvent(clientfd);
				return;
			}
			buff[n] = '\0';
			std::cout << "response: " << buff << std::endl;
			std::cout << "please input if contine to send ..." << std::endl;
        }
    }

private:
	std::string ip_;
	int port_;
	int clientfd_;
	Epoller::ptr epoll_;
};


int main(int argc, char** argv)
{
    TcpClient::ptr client = std::make_shared<TcpClient>("127.0.0.1", 7778);
	client->run();
	
	std::cout << "please input to send ..." << std::endl;
	std::string line;
	while (getline(std::cin, line))
	{
		client->send(line.c_str(), line.size());

	}

    return 0;
}