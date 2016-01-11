# Hooli Drive
A client-server file transfer system built in C

Hooli Drive was a networking assignment given by Jeff Shantz at the University of Western Ontario, which I completed successfully with an overall grade of 102.5%.

Note: This should not be considered ready for production. However, it may be used by someone trying to solve a similar problem, or as a reference for setting up file transfer over TCP and reliable UDP connections.

## Overview
Hooli Drive works via 3 main programs - the client, a database server (HMDS), and a file upload server . It is built to run on a Linux platform, with all development and testing having been done within a Vagrant VM running Ubuntu 14.04 Trusty.

The client recursively scans a specified directory for files and computes their checksums (used to determine file modification). It then sends the list of files and their checksums to the database server (HMDS), using a TCP connection. Hmds compares the filenames and checksums against the metadata stored in a Redis server. Once HMDS has compiled a list of files that are either new or have been modified, it sends a response back to the client, listing the files to be uploaded to the file server (HFTPD). The client then sends the requested files to HFTPD over a reliable UDP connection, which operates on a Stop-and-Wait protocol.

## How to use

### Vagrant Setup
Note: If you are operating on Ubuntu and prefer not to use a Vagrant machine, you can skip this.
1. Install [vagrant](https://www.vagrantup.com/).
2. From within the root directory, run `vagrant up`.
3. Once the machine is running, run `vagrant ssh` to access the virtual machine. Use `exit` at any point to leave the VM.
4. You may use `vagrant halt` to stop the VM, and `vagrant destroy` to tear it down.

### Dependencies
Hooli Drive requires the following dependencies to be installed on the VM.
* zlib (for computing checksums) `sudo apt-get install zlib1g-dev`
* Redis server `sudo apt-get install redis-server`
* Redis C library `sudo apt-get install libhiredis-dev`

### Running Client
Note that before executing the client, HMDS and HFTPD must be running and awaiting communication from the client.
Within the *client* folder, run `make`.

The client accepts 6 optional parameters followed by 2 required ones:
* `-s HOSTNAME` / `--server HOSTNAME`
  Specifies the database server's hostname (hmds). If not given, defaults to `localhost`.

* `-p PORT` / `--port PORT`
  Specifies the database server's port (hmds). If not given, defaults to `9000`.

* `-d DIR` / `--dir DIR`
  Specifies the Hooli root directory to be scanned for files. If not given, defaults to `~/hooli`.

* `-f / --fserver HOSTNAME`
  Specifies the HFTP server's hostname. If not given, default to `localhost`.

* `-o / --fport PORT`
  Specifies the HFTP server's port. If not given, defaults to `10000`.
  
* `-v`/ `--verbose`
  Enables verbose output.
  
Therefore, the client might be executed as follows:

`./client jerridan password`

or:

`./client -s example1.com -p 7777 -d ./my_directory -f example2.com -o 8888 -v`

### Running HMDS
Within the *hmds* folder, run `make`.

HMDS accepts 3 optional parameters:

* `-p PORT` / `--port PORT`
  Specifies the port on which to listen. If not given, defaults to `9000`.

* `-r HOSTNAME` / `--redis HOSTNAME`
  Specifies the hostname of the Redis server. If not given, defaults to `localhost`.

* `-v`/ `--verbose`
  Enables verbose output.
  
Therefore, HMDS might be executed as follows:

`./hmds`

or:

`./hmds -p 7777 -r example_redis.com -v`

### Running HFTPD
Within the *hftpd* folder, run `make`.

HFTPD accepts 5 optional parameters:
* `-p / --port PORT`
  Specifies the port on which to listen for HFTP messages. If not given, defaults to `10000`.

* `-r HOSTNAME / --redis HOSTNAME`
  Specifies the hostname of the Redis server. If not given, defaults to `localhost`.
  
* `-d / --dir ROOT`
  Specifies the directory in which uploaded files are stored.  Files are stored as `ROOT/username/relative/path/to/file`.
  If not given, defaults to `/tmp/hftpd`. If the directory does not exist, it will be created.

* `-t / --timewait SECONDS`
  The number of seconds HFTPD should spend in the *TIME_WAIT* state before exiting. If not given, defaults to `10`.
  
  Note: The *TIME_WAIT* state occurs after HFTPD has acknowledged a termination request by the client. In case the acknowledging message is lost, HFTPD waits to ensure the client does not resend a termination request.

* `-v / --verbose`
  Enable verbose output (see description later).  If not given, defaults to non-verbose output.
  
Therefore, HFTPD might be executed as follows:

`./hftpd`

or:

`./hftpd -p 8888 -r example_redis.com -d ~/myuploads -t 15 -v`

## Further documentation
The original documentation outlining the requirements and functionality of this assignment in depth can be found within the wiki. Note that the assignment was broken up into 3 separate parts.

If you are interested in the evaluations and feedback I received for this assignment, you will find them in the *evaluations* folder. Note that for assignment 2, a late penalty of -16% was incorrectly assigned, and was later changed to -4%.
