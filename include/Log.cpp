#include "Log.h"

std::string currentISO8601TimeUTC() {
	auto now = std::chrono::system_clock::now();
	auto itt = std::chrono::system_clock::to_time_t(now);
	std::ostringstream ss;
	ss << std::put_time(gmtime(&itt), "%F %T");
	return ss.str();
}

std::string currentISO8601TimeUTCBlock() {
	auto now = std::chrono::system_clock::now();
	auto itt = std::chrono::system_clock::to_time_t(now);
	std::ostringstream ss;
	ss << std::put_time(gmtime(&itt), "%Y%m%d%H%M");
	return ss.str();
}
