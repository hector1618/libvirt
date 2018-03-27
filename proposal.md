Objective:
---------
The objective of this project is to implement domxml-from-native and domxml-to-native for OCI formatted containers.

Container formats is being standardized by Open Container Initiative. OCI has two specifications. 1. Runtime Specification 2. Image Specification. Containers are built an run based on these specifications.

domxml-from-native generates domxml from containers’ configuration. Docker image’s configuration is created in JSON format. Based on this configuration, a domxml file need to be generated that libvirt understands and can generate lxc from this xml.

Implementation plan:
-------------------

1. Understand lxc driver
   a. lxcConnectDomainXMLFromNative
2. Understand OCI & Docker
   a. Runtime spec
   b. Image spec
3. Convert Docker config to domxml
   a. ...

Timeline:
--------

I’ve been working on understanding background of the project and to get overview of libvirt, docker. As of now, I don’t understand lxc driver or docker well enough to provide a timeline.  Any timeline I propose at this point in time will be a guess work and very likely to be inaccurate. But I intend to keep working to understand them and come up with reasonably accurate timeline. I’ll update the timeline details in the above mentioned document as soon as possible.


Work done so far:
----------------

1. Setup dev env
I've set up the development environment and compiled the source. Played around with virsh. Created qemu and lxc images with various configurations.


2.Contribution
Worked on one of bite sized tasks and submitted the following patches
https://libvirt.org/git/?p=libvirt.git;h=3f9e02b40aa291ea58aefb085d882174d2ec6330


I'm currently working on replacing virConfGetValue function with type specific virConfGetValue* functions in lxc/lxc_native.c. Also, I have been exploring the code base to understand source structure and important data structures.




Personal information:
--------------------
Name: Prafullkumar Tale
Email: talep158@gmail.com
Occupation: JRF at Institute of Mathematical Sciences
