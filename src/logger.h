#ifndef SRC_LOGGER_H_
#define SRC_LOGGER_H_

#include <string>

using std::string;

/**
 * \brief	A simple logger that opens, writes to the file and closes it each time a log happens.
 */
class Logger {
public:
	/**
	 * \brief	Create a log instance that write to the file specified.
	 */
	Logger(const string& path);
	virtual ~Logger();

	/**
	 * \brief	Writes new line into log-file. Each line starts with the current time stamp.
	 *
	 * \param	text	The custom log text.
	 */
	void Log(const string& text);

	/**
	 * \brief	Writes new line into log-file. Each line starts with the current time stamp.
	 *
	 * \param	text	The custom log text.
	 * \param	num		A custom integer that is added at the end of the log text.
	 */
	void LogWithInt(const string& text, const int& num);

private:
	const string path_;

	/**
	 * \brief	Get a string representation of the current time.
	 */
	string GetTimeString() const;
};

#endif /* SRC_LOGGER_H_ */
