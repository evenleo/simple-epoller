#ifndef __EPOLLEXCEPTION_H__
#define __EPOLLEXCEPTION_H__

#include "Epoller.h"
#include <exception>
#include <string>

class EpollException: public std::exception {
public:
	EpollException(std::string display);
	virtual ~EpollException() _GLIBCXX_USE_NOEXCEPT;
	std::string displayText() const;
private:
	std::string display;
};

std::string exceptionText(EpollException& exception);
std::string exceptionText(EpollException& exception, const std::string& exceptionPosition);
std::string exceptionText(const std::string &strErr, const std::string& exceptionPosition);

#define EXCEPTION_POSITION Epoller::EpollFormat("%s %d",__FILE__,int(__LINE__))
#define THROW_EXCEPTION(EXCEPTION, INFO) throw EXCEPTION(std::string(INFO)+EXCEPTION_POSITION)

#endif /* __EPOLLEXCEPTION_H__ */
