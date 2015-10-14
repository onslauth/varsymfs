# Variant Symlink Filesystem

This module provides a Variant Symlink Filesystem for linux
similar to the one implemented in FreeBSD.

A Variant Symlink is a symlink that is based off of an environment
variable's value. If the environment variable is not present, the symlink
simply looks like a broken link, and cannot be followed. If however the
process contains the specified environment variable, it will be used to
determine the path that the symlink will resolve to.


### Building

Simply clone the repo, and inside the directory run the following:
```
$ make
```

This should build the Filesystem as a module.


### Usage

Once the kernel module has been built, simply load it, and create a mount point at
the desired location:

```
$ sudo insmod varsymfs.ko
$ sudo mount -o ENV_VAR -t varsymfs none /opt/pq/0
```

This will create a mount point under the directory '/opt/pq/0/' which will contain a
single symlink 'resolve'. The symlink will resolve to the value of the environment
variable ENV_VAR. If the environment variable is not set, the symlink will simply
appear to be a broken symlink.


### Demo
```
guestm@lvm:~/work/varsymfs/varsymfs-new [0]
$ ls -l /opt/pq/0
total 0
lrwxrwxrwx 1 root root 0 Oct  9 09:49 resolve
guestm@lvm:~/work/varsymfs/varsymfs-new [0]
$ export ENV_VAR=~/pq/opt/pq/0/chassis/
next/ test/
guestm@lvm:~/work/varsymfs/varsymfs-new [0]
$ export ENV_VAR=~/pq/opt/pq/0/chassis/test/
guestm@lvm:~/work/varsymfs/varsymfs-new [0]
$ exec bash
guestm@lvm:~/work/varsymfs/varsymfs-new [0]
$ ls -l /opt/pq/0
total 0
lrwxrwxrwx 1 root root 0 Oct  9 09:49 resolve -> /home/guestm/pq/opt/pq/0/chassis/test/
guestm@lvm:~/work/varsymfs/varsymfs-new [0]
```


### Thanks
- Justin Swartz ( justin.swartz@risingedge.co.za )
  For providing me with code to do the environment variable lookup from inside a kernel module.
