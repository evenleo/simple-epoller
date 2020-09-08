#include "Epoller.h"

int main(int argc, char** argv) {
	ServerHandler serverhandler(8877);
	IOLoop::Instance()->start();
	return 0;
}
