#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>

#include "wol.h"
#include "pinglib.h"
#include "filehandling.h"
#include "logger.h"


using namespace std;


// #define DEBUG


// server settings
static const string mPath("/share/");
static const string mServerName("lang-nas16");
static const string mServerIP("192.168.0.7");
static const string mServerMAC("40:8D:5C:B6:E6:52");
static const int	mServerPort(5555);
static const bool   mIsSshServer(true);
static const string mSshUser("root");
static const string mSshPassword("oaJl|5)Offs6");

// client settings
static const int mNumClients = 3;
static const string mClientIpList[] = {"192.168.0.21", "192.168.0.25", "192.168.0.12", "\0"};
static const string mClientNameList[] = {"tv-wohnzimmer", "lang-ct2014", "erdin-velin"};

// behaviour settings
static const int mInterval = 1;
static const int mWolTimeout = 10;
static const int mShutdownTimeout = 100;

// global constants
static const string mSep("/");
static const string mDirectory = mPath + mServerName + mSep;
static const string mLogName("log");
static const string mOn("on");
static const string mOff("off");
static const string mStart("start");
static const string mStop("stop");
static const string mKeepStarted("force_on");
static const string mKeepStopped("force_off");

// global variables
int pingRes = 0;					//!< Result of the ping. Positive value means the round-trip time. Negative values means error in ping.
int mWolTimer = 0;					//!< Indicates how long we have to wait for another WOL. Can send WOL if <= 0.
int serverRunning = -1;				//!< Indicator if server is running. -1 mean not initialized.
int mShutdownTimer = 0;				//!< Indicates how long we have to wait until we can shutdown server. Can shutdown if <= 0.
bool someClientsRunning = false;	//!< Indicator if at least one client is running.
Logger logger(mDirectory + mLogName);

/*
 *	\brief	Method uses a WOL-package to start the server.
 */
static void startServerIfNotRunning() {
	// only send WOL if server not running and WOL timeout has been expired
	if (mWolTimer <= 0 && serverRunning != 1) {
		int wolRes = sendWol(mServerMAC.c_str());
		if (wolRes != 0) {
			logger.LogWithInt("[error]\tWOL failed!", wolRes);
		} else {
			mWolTimer = mWolTimeout;
			logger.Log("[info]\tWOL sent");
		}
	}
}


/*
 *	\brief	Method uses a UDP-package to indicate the ShutdownDaemon on the server to shutdown the host.
 *
 * \param	test	Indicates if only a test packet will be sent to the server.
 */
static void shutdownServerIfRunning(bool test = false) {
	// only send shutdown packet if server running
	if (serverRunning != 1) return;

	// do use SSH if available
	// This needs a correct hash in ~/.ssh/known_hosts. This can be ensured by
	// once manually connect via ssh as root to the remote host.
	if (mIsSshServer) {
		const string ssh_login = string("sshpass -p \"") + mSshPassword + string("\" ssh ") + mSshUser + string("@") + mServerIP;
		#ifdef DEBUG
			logger.Log("[debug]\tSSH login command: " + ssh_login);
		#endif
		FILE *ssh = popen(ssh_login.c_str(), "w");
		if (ssh == NULL) {
			logger.Log("[error]\tpopen failed!");
			return;
		}

		fputs("shutdown -h now", ssh);

		pclose(ssh);
		logger.Log("[info]\tshutdown command via SSH executed");
		return;
	}

	// try to open socket
	struct sockaddr_in si_other;
    int s, slen=sizeof(si_other);
    string message;
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        logger.Log("[error]\tsocket could not be opened!");
    }

	// try to start listening on socket
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(mServerPort);
    if (inet_aton(mServerIP.c_str(), &si_other.sin_addr) == 0) {
		logger.Log("[error]\tlistening could not be started!");
    }

	// create message
	if (test) {
		message = "test";
	} else {
		message = "shutdown";
	}

	//send the message
	if (sendto(s, message.c_str(), strlen(message.c_str()) , 0 , (struct sockaddr *) &si_other, slen)==-1) {
		logger.Log("[error]\tpacket could not be sent!");
	} else {
		logger.Log("[info]\tshutdown packet sent: " + message);
	}
}


/*
 *	\brief	Checks if state of server has changed and handles the
 *			indicator files and logging of the server state.
 *
 *	\param	newState	The current state of the server at call time.
 */
void checkAndSignalServerState(int newState) {
	if (newState < 0 || newState > 1) {
		logger.LogWithInt("[error]\tWrong newState!", newState);

	} else if (newState == 1) {
		// server changed state to running
		if (serverRunning != 1) {
			// write log
			logger.Log("[info]\tserver started");
			// write on-file and delete off-file
			createFile((mDirectory + mOn).c_str());
			remove((mDirectory + mOff).c_str());
			// set server as running
			serverRunning = 1;
			// reset WOL timeout to ensure that next start approach executes immediately
			mWolTimer = 0;
		}
	} else {
		// server changed state to stopped
		if (serverRunning != 0) {
			// write log
			logger.Log("[info]\tserver stopped");
			// write off-file and delete on-file
			createFile((mDirectory + mOff).c_str());
			remove((mDirectory + mOn).c_str());
			// set server as not running
			serverRunning = 0;
		}
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
		logger.Log("[error]\tsession id failed!");
		exit(1);
	}
	
	// Change the current working directory to root.
	if (chdir("/") < 0) {
		logger.Log("[error]\tchange dir failed!");
		exit(1);
	}

	// Close stdin. stdout and stderr
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	
	logger.Log("[info]\tDaemon started");

	
	while (1) {	
		sleep(mInterval);
		
		//****************************
		// check if server is running
		//****************************
		
		// check if server is running
		pingRes = ping(mServerIP);
		
		// server running
		if (pingRes > 0) {
			#ifdef DEBUG
				logger.Log("[debug]\tserver-ping > 0:\t" + mServerIP);
			#endif
			checkAndSignalServerState(1);

		// server not running
		} else if (pingRes == 0) {
			#ifdef DEBUG
				logger.Log("[debug]\tserver-ping == 0:\t" + mServerIP);
			#endif
			checkAndSignalServerState(0);

		// log failed ping
		} else {
			logger.LogWithInt("[error]\tserver-ping failed!:\t" + mServerIP, pingRes);
		}


		//*****************************
		// check if force-files around
		//*****************************

		static const string mKeepStarted("force_on");
		static const string mKeepStopped("force_off");

		// check force_on-file and start server if not already running
		// force_on has higher priority the force_off
		if (checkFile((mDirectory + mKeepStarted).c_str())) {
			#ifdef DEBUG
				logger.Log("[debug]\tforce_on file available");
			#endif
			startServerIfNotRunning();
			continue;
		}

		// check force_off-file and stop server if running
		if (checkFile((mDirectory + mKeepStopped).c_str())) {
			#ifdef DEBUG
				logger.Log("[debug]\tforce_off file available");
			#endif
			shutdownServerIfRunning();
			continue;
		}


		//************************
		// check if client around
		//************************

		// concern that no clients are running
		someClientsRunning = false;

		// iterate over all possible clients
		for (int c=0; c < mNumClients; ++c) {

			// check if mNumClients too high
			if (mClientIpList[c].compare("\0") == 0) {
				logger.LogWithInt("[error]\tmNumClients too long!:\t", mNumClients);
				break;
			}

			// check if any client is running that needs the server
			pingRes = ping(mClientIpList[c]);

			// start server if a new running client has been found
			if (pingRes > 0) {
				#ifdef DEBUG
					logger.Log("[debug]\tclient-ping > 0:\t" + mClientNameList[c]);
				#endif

				// initialize shutdown time, signal that clients are running and start server
				someClientsRunning = true;
				mShutdownTimer = mShutdownTimeout;
				startServerIfNotRunning();

				// not needed to check other clients
				#ifdef DEBUG
					logger.LogWithInt("[debug]\tskipped client-pings:\t", mNumClients - c - 1);
				#endif
				break;

			// do nothing
			} else if (pingRes == 0) {
				#ifdef DEBUG
					logger.Log("[debug]\tclient-ping == 0:\t" + mClientNameList[c]);
				#endif

			// log failed ping
			} else {
				logger.LogWithInt("[error]\tclient-ping failed!:\t" + mClientNameList[c], pingRes);
			}
		}

		// evaluate if server has to be shutdown
		if (someClientsRunning == false && serverRunning == 1) {

			// shutdown the server
			if (mShutdownTimer <= 0) {
				shutdownServerIfRunning();
				mShutdownTimer = mShutdownTimeout;

			} else {		// wait for timeout run out
				mShutdownTimer -= mInterval;
			}
		}


		//*******************************
		// check if command-files around
		//*******************************

		// remove start-file and start server if not already running
		if (checkAndRemoveFile((mDirectory + mStart).c_str()) && serverRunning != 1) {
			startServerIfNotRunning();
		}

		// remove stop-file and stop server if running
		if (checkAndRemoveFile((mDirectory + mStop).c_str()) && serverRunning == 1) {
			shutdownServerIfRunning();
		}


		//********************
		// handle WOL timeout
		//********************

		if (mWolTimer > 0) {
			mWolTimer -= mInterval;
		}
	}


	return (0);
}
