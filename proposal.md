Objective:
---------
The objective of this project is to implement domxml-from-native and domxml-to-native for OCI formatted containers.

Container formats is being standardized by Open Container Initiative. OCI has two specifications. 1. Runtime Specification 2. Image Specification. Containers are built an run based on these specifications.

domxml-from-native generates domxml from containers’ configuration. Docker image’s configuration is created in JSON format. Based on this configuration, a domxml file need to be generated that libvirt understands and can generate lxc from this xml.

Implementation plan:
-------------------

1. Understand lxc driver
   1. lxcConnectDomainXMLFromNative
1. Understand OCI & Docker
   1. Runtime spec
   2. Image spec
1. Convert Docker config to domxml
   1. ...

Work done so far:
----------------
1. Setup dev env
2. Compiled and tried.
3. Sent a patch to the list.
4. virsh implementation.

Personal information:
---------------------
Name: Prafullkumar Tale
Email: talep158@gmail.com
Occupation: JRF at Institute of Mathematical Sciences
