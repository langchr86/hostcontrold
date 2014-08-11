#include <string>
#include <oping.h>


int ping(std::string ip) {
	// create object
	pingobj_t * obj = ping_construct();

	// add host to object
	if (ping_host_add(obj, ip.c_str()) < 0) {
		return -1;	// error
	}

	// send ICMP
	int res = ping_send(obj);
	if (res < 0) {
		return -2;	// error
	} else if (res == 0) {
		// no echo replies
	}
	
	// receive info
	pingobj_iter_t *iter = ping_iterator_get(obj);
	double latency = -1.0;
	size_t buffer_len = sizeof(latency);

	if (iter == NULL) {
		return -4;	// error
	}
	if (ping_iterator_get_info (iter, PING_INFO_LATENCY, &latency, &buffer_len) < 0) {
		return -5;	// error
	}
	
	// delete ressources
	ping_destroy(obj);
	
	// return result
	if (latency < 0.0) {
		return 0;	// timeout
	} else {
		return 1;	// ping reply recieved
	}
}