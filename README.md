# simple-epoller

## 实现目的
在linux下简单实现基于epoll echo服务器和客户端，旨在理解epoll的基本原理

## 环境
Linux version 4.15.0-117-generic
Ubuntu 5.4.0-6ubuntu1~16.04.12
gcc version 5.4.0

## build
```
git clone https://github.com/evenleo/simple-epoller.git
cd simple-epoller
mkdir build
cd build
cmake ..
make
```

## 测试

EchoServer
```
evenleo@ubuntu:~/workspace/simple-epoller/build$ ./EchoServer 
accept a connection from 127.0.0.1
recv from client fd= 5, buff=evenleo
recv from client fd= 5, buff=hello
```

EchoClient
```
evenleo@ubuntu:~/workspace/simple-epoller/build$ ./EchoClient 
please input to send ...
evenleo
response: evenleo
please input if contine to send ...
hello
response: hello
please input if contine to send ...

```
