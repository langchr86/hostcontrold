#include "logger.h"

#include <time.h>

#include <fstream>
#include <sstream>

using std::ofstream;
using std::stringstream;
using std::endl;

Logger::Logger(const string& path)
	: path_(path) {
}

Logger::~Logger() {
}

void Logger::Log(const string& text) {
	ofstream log_file(path_.c_str(), std::ios_base::out | std::ios_base::app);
	log_file << GetTimeString() << "\t\t";
	log_file << text << endl;
}

void Logger::LogWithInt(const string& text, const int& num) {
	ofstream log_file(path_.c_str(), std::ios_base::out | std::ios_base::app);
	log_file << GetTimeString() << "\t\t";
	log_file << text << " (" << num << ")" << endl;
}

string Logger::GetTimeString() const {
	time_t timer;
	time(&timer);
	struct tm* timeinfo = localtime(&timer);

	stringstream stream;

	stream << timeinfo->tm_mday << "." << timeinfo->tm_mon+1 << "." << timeinfo->tm_year+1900 << " ";
	stream << timeinfo->tm_hour << ":" << timeinfo->tm_min << ":" << timeinfo->tm_sec;

	return stream.str();
}

