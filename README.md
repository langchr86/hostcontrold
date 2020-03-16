Host Control Daemon
===================


Tools
-----

~~~
apt install cmake gcc g++ autoconf pkg-config libtool libsystemd-dev
~~~


Preparations
------------

* Make hostpingable
  * linux should already respond to ICMP requests
  * in windows the firewall has to be configured to allow incoming IPv4 ICMP echo requests.
