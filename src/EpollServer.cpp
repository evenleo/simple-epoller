#include "Epoller.h"
#include <iostream>
#include<vector>
#include <arpa/inet.h>
#include <unordered_map>

using namespace std;


int main() {
	ServerHandler serverhandler(8877);
	IOLoop::getInstance()->start();
	return 0;
}
