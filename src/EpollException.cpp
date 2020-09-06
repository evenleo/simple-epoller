#include "EpollException.h"

EpollException::EpollException(std::string display) :
		display(display) {
}

EpollException::~EpollException() _GLIBCXX_USE_NOEXCEPT{
}

std::string EpollException::displayText() const {
	return display;
}

std::string exceptionText(EpollException& exception) {
	return exception.displayText();
}

std::string exceptionText(EpollException& exception, const std::string& exceptionPosition) {
	std::string result = exception.displayText();
	result.append(":").append(exceptionPosition);
	return result;
}

std::string exceptionText(const std::string &strErr, const std::string& exceptionPosition) {
	std::string result = strErr;
	result.append(":").append(exceptionPosition);
	return result;
}
