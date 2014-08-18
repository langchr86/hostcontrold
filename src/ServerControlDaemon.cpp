/**
 * \file 		ServerControlDaemon.cpp
 *
 * \date 		06.05.2013
 * \author 		Christian Lang
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <fstream>
#include <time.h>

#include "wol.h"
#include "pinglib.h"
#include "filehandling.h"


using namespace std;


//#define DEBUG


// settings
static const string mPath("/share/");
static const string mServerName("lang-mainserver");
static const string mIP("192.168.0.9");
static const string mMAC("00:01:2E:31:64:FF");

// global constants
static const string mSep("/");
static const string mDirectory = mPath + mServerName + mSep;
static const string mLogName("log");
static const string mOn("on");
static const string mOff("off");
static const string mStart("start");

// global variables
time_t timer;
struct tm* timeinfo;
ofstream logFile;

static void log(const char* text) {
	logFile.open((mDirectory + mLogName).c_str(), ios_base::out | ios_base::app);
	if (logFile.is_open()) {
		time(&timer);
		timeinfo = localtime(&timer);
		logFile << timeinfo->tm_mday << "." << timeinfo->tm_mon+1 << "." << timeinfo->tm_year+1900 << " ";
		logFile << timeinfo->tm_hour << ":" << timeinfo->tm_min << ":" << timeinfo->tm_sec << "\t\t";
		logFile << text << endl;
		logFile.close();
	} else {
		return;
	}
}

static void logWithInt(const char* text, int num) {
	logFile.open((mDirectory + mLogName).c_str(), ios_base::out | ios_base::app);
	if (logFile.is_open()) {
		time(&timer);
		timeinfo = localtime(&timer);
		logFile << timeinfo->tm_mday << "." << timeinfo->tm_mon+1 << "." << timeinfo->tm_year+1900 << " ";
		logFile << timeinfo->tm_hour << ":" << timeinfo->tm_min << ":" << timeinfo->tm_sec << "\t\t";
		logFile << text << " (" << num << ")" << endl;
		logFile.close();
	} else {
		return;
	}
}


int main(int argc, char* argv[]) {
	
/**********************************
 * Start of Daemon
 * Has to be run in background
 *********************************/

	//unmasking the file mode
	umask(0);
	
	// create directory
	int status = mkdir(mDirectory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	if (status != 0) {
		// error or already there
	}
	
	// remove old files
	remove((mDirectory + mOff).c_str());
	remove((mDirectory + mOn).c_str());
	remove((mDirectory + mStart).c_str());

	//set new session
	pid_t sid = setsid();	
	if(sid < 0) {
		log("[error]\tsession id failed!");
		exit(1);
	}
	
	// Change the current working directory to root.
	if (chdir("/") < 0) {
		log("[error]\tchange dir failed!");
		exit(1);
	}

	// Close stdin. stdout and stderr
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	
	log("[info]\tDaemon started");

	int pingRes = 0;
	int serverRunning = -1;
	
	while (1) {		
		sleep(5);
		
		// Test if server running
		pingRes = ping(mIP);
		
		if (pingRes > 0) {	
#if DEBUG
			log("[debug]\tping > 0");
#endif			
			// server running
			if (serverRunning != 1) {
				// write log
				log("server started");
				// write On-File and delete Off-File
				createFile((mDirectory + mOn).c_str());
				remove((mDirectory + mOff).c_str());
				// Set Server as running
				serverRunning = 1;
			}
		} else if (pingRes == 0) {
#if DEBUG
			log("[debug]\tping == 0");
#endif			
			// server not running
			if (serverRunning != 0) {
				// write log
				log("server stopped");
				// write Off-File and delete On-File
				createFile((mDirectory + mOff).c_str());
				remove((mDirectory + mOn).c_str());
				// Set Server as not running
				serverRunning = 0;
			}
		} else {
			logWithInt("[error]\tping failed!", pingRes);
		}
		
		// Test if Start-File
		if (checkAndRemoveFile((mDirectory + mStart).c_str()) && serverRunning != 1) {
			// start server
			int wolRes = sendWol(mMAC.c_str());
			if (wolRes != 0) {
				logWithInt("WOL failed!", wolRes);
			} else {
				log("WOL sent");
			}
		}
	}

	
	return (0);
}