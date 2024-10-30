# LucyRTL8125Ethernet

A macOS driver for Realtek RTL8125 2.5GBit Ethernet Controllers

## Key Features of the Driver

* Supports all versions of Realtek's RTL8125 2.5GBit Ethernet Controllers found on recent boards.</br>
* Support for multisegment packets relieving the network stack of unnecessary copy operations when assembling packets for transmission. 
* No-copy receive and transmit. Only small packets are copied on reception because creating a copy is more efficient than allocating a new buffer. TCP, UDP and IPv4 checksum offload (receive and transmit).
* TCP segmentation offload over IPv4 and IPv6.
* Support for TCP/IPv4, UDP/IPv4, TCP/IPv6 and UDP/IPv6 checksum offload.
* Supports jumbo frames up to 4076 bytes.
* Fully optimized for Catalina. Note that older versions of macOS might not support 2.5GB Ethernet.
* Supports Wake on LAN (untested).
* Supports VLAN.
* Support for Energy Efficient Ethernet (EEE) which can be disabled by setting enableEEE to NO in the drivers Info.plist without rebuild. The default is YES.
* The driver is published under GPLv2.

## No support on GitHub!!!

In case you are looking for a prebuilt binary or need support, please see the driver's thread on insanelymac.com:

https://www.insanelymac.com/forum/topic/343542-lucyrtl8125ethernetkext-for-realtek-rtl8125/

Support requests here on GitHub will be ignored!
