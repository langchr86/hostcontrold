hostcontrold
============

[![Build Status](https://travis-ci.com/langchr86/hostcontrold.svg?branch=master)](https://travis-ci.com/langchr86/hostcontrold)

Host Control Daemon.
Daemon which monitors other hosts with ping requests (ICMP)
and controls some of them with WOL (wake on LAN) packets.



Features
--------

Main purpose is to control one/multiple linux hosts (e.g. SMB server) by some central linux host.
This is to save power by shutting down not needed servers automatically.

The central host where `hostcontrold` is running does monitor not only the to control host (server)
but also the clients of this server.
If one of the configured client hosts is detected by a ping
the central controller starts the server host by using a magic WOL packet.
This allows to only run servers if some clients are up and running.
If all clients shutdown the server will shutdown too after a specified timeout period.

All the features:

* Core:
  * Monitoring of client/server hosts with ICMP ping requests.
  * Every host that can be pinged can work as a client (Windows, Linux, OSX).
  * Servers are started by WOL and shutdown via SSH.
  * Only Linux server hosts are supported.
* System integration:
  * Single JSON file to configure the daemon.
  * Prepared unit file for systemd integration.
  * Control multiple servers with dependencies to multiple clients.
* Server directories:
  * The state of each server can be observed by simple files in a control-directory.
  * Servers can also manually be controlled by creating some simple files in the control-directory.



Quick start guide
-----------------

### Installation

* Clone the repository on your central control linux host.
* Install build dependencies with:

  ~~~
  sudo apt install cmake g++ pkg-config libtool libsystemd-dev liboping-dev
  ~~~

* Build and install:

  ~~~
  cd hostcontrold && mkdir build && cd build
  cmake ..
  sudo make -j4 install
  ~~~


### Configuration

The configuration file is located at `/etc/hostcontrold.conf`.
If you start the daemon without a created config file a default one will be generated.
You can add arbitrary more server blocks and arbitrary clients per server.


### Prepare client hosts

* Make host pingable.
  * Linux should already respond to ICMP requests.
  * In Windows the firewall has to be configured to allow incoming IPv4 ICMP echo requests.

You can test if a host responses to ICMP requests by using ping:

~~~
ping 192.168.0.6
~~~


### Prepare server hosts

* Allow to startup from WOL packet.
  * Configure BIOS/UEFI and if needed the operating system.
  * Ensure that the network card has standby-power supply.
  * WOL works only when the host was booted at least one time after complete power loss.
* Allow central control host to shutdown server host.
  * Control host need to be able to ssh to the server host without using a password.
    Usually use an rsa-key. Ensure that the host is already in `~/.ssh/known_hosts`.
  * Ensure the used user is correctly configured on the server host.
  * The user needs to be able to call sudo. This is used to call the shutdown command.

An example of commands that the root user could use to prepare a server host
where the user `clang` should used to login via ssh:

~~~
sudo su
ssh-keygen
ssh-copy-id clang@192.168.0.6
~~~

You can test if everything is ok if the following command works without any user input:

~~~
sudo su
ssh clang@192.168.0.6
~~~



### Start daemon

Let systemd know the new daemon, enable autostart at system startup and start the daemon.

~~~
sudo systemctl daemon-reload
sudo systemctl enable hostcontrold
sudo systemctl start hostcontrold
~~~

Use `sudo systemctl restart hostcontrold` after changing the configuration.

To view the log of the daemon use:

~~~
journalctl -e -u hostcontrold
~~~



Usage / Control-Directory
-------------------------

After installation and startup the daemon will run and monitor all configured hosts constantly.
You can see the state of a server host visualized by the `on`/`off` file in the configured control-directory.
By creating simple files in the directory you can force some specific behavior:

| file name      | content      | feature |
| ---------      | -------      | ------- |
| `force_on`     | empty        | will keep the server powered on |
| `force_off`    | empty        | will keep the server powered down |



Development
-----------

See the `Installation` steps in the `Quick start guide` section for information how to build.

The code itself is organized as pure CMake project with C++14 code.
The prefered IDE to work on the code is CLion.


### Docker

The whole development steps can also be done in the prepared docker container.
Those are mainly used to build and test the code with travis on different distributions/toolchains
but can also be used for local development.
Ubuntu with GCC is used by default.

~~~ {.bash}
# Build the docker image.
./development/1_create.sh

# Create docker container and run it with the correct mounted volumes.
./development/2_run.sh

# Connect into the container.
./development/3_connect.sh

# Now you are logged-in the docker container.

cd /tmp/hostcontrold/

# Now you are in the source directory where you can compile and run tests e.g.
mkdir build
cd build
cmake ..
make -j4

# Leave the container with the following command when you are finished developing/testing.
exit

# Remove all signs of the docker container.
./development/5_remove.sh
~~~


### Future features

* Extend the shutdown timeout by file in control-directory.
* Automate installation of build dependencies. Maybe use Conan or Ansible.
* User which executed the shutdown command may not need `sudo`. Allow to configure this.
* Maybe allow to configure whole shutdown command. This would allow to use non-linux servers.
  And some special shutdown handling could be executed.
* Introduce interfaces and write unit tests for control logic.



License
-------

MIT



Versioning
----------

There exists no version numbers, releases, tags or branches.
The master should be considered the current stable release.
All other existing branches are feature/development branches and are considered unstable.



Author Information
------------------

Christian Lang
[lang.chr86@gmail.com](mailto:lang.chr86@gmail.com)
