#include <sys/stat.h>
#include <unistd.h>

#include <thread>

#include "logger.h"
#include "server_control.h"


int main(int argc, char* argv[]) {
	
/**********************************
 * Start of Daemon
 * Has to be run in background
 *********************************/

	// setup main logger
	Logger logger("/share/ServerControl.log");

/*
	//unmasking the file mode
	umask(0);

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
	*/
	
	logger.Log("[info]\tDaemon started");


/**********************************
 * Main programm
 *********************************/

	ServerControl::Config nas16_config;
	nas16_config.ip = "192.168.0.7";
	nas16_config.mac = "40:8D:5C:B6:E6:52";
	nas16_config.ssh_user = "clang";
	nas16_config.shutdown_use_ssh = true;
	nas16_config.control_interval = std::chrono::seconds(5);
	nas16_config.shutdown_timeout = std::chrono::minutes(10);

	ServerControl::ClientList nas16_clients;
	nas16_clients.emplace_back("192.168.0.66", "tv-wohnzimmer");
	nas16_clients.emplace_back("192.168.0.25", "lang-ct2014");
	nas16_clients.emplace_back("192.168.0.152", "erdin-velin");

	ServerControl nas16("/share/lang-nas16/", nas16_config, nas16_clients);

	ServerControl::Config nas08_config;
	nas08_config.ip = "192.168.0.9";
	nas08_config.mac = "00:01:2E:31:64:FF";
	nas08_config.ssh_user = "clang";
	nas08_config.shutdown_use_ssh = true;
	nas08_config.control_interval = std::chrono::seconds(30);
	nas08_config.shutdown_timeout = std::chrono::minutes(1);

	ServerControl::ClientList nas08_clients;

	ServerControl nas08("/share/lang-nas08/", nas08_config, nas08_clients);
	
	while (true) {
		nas16.DoWork();
		nas08.DoWork();

		std::this_thread::sleep_for(seconds(1));
	}
}
