# LucyRTL8125Ethernet

A macOS driver for Realtek RTL8125 2.5GBit Ethernet Controllers

LucyRTL8125Ethernet is EOL. It is recommeded to switch to RTL812xLucy, which is a superset of LucyRTL8125Ethernet.

**Key Features of LucyRTL8125Ethernet**

* Supports all versions of Realtek's RTL8125 2.5GBit Ethernet Controllers found on recent boards.</br>
* Support for multisegment packets relieving the network stack of unnecessary copy operations when assembling packets for transmission. 
* No-copy receive and transmit. Only small packets are copied on reception because creating a copy is more efficient than allocating a new buffer. TCP, UDP and IPv4 checksum offload (receive and transmit).
* TCP segmentation offload over IPv4 and IPv6.
* Supports AppleVTD (Tahoe included).
* Support for TCP/IPv4, UDP/IPv4, TCP/IPv6 and UDP/IPv6 checksum offload.
* Supports jumbo frames up to 9000 bytes.
* Fully optimized for Catalina and newer. Note that older versions of macOS might not support 2.5GB Ethernet.
* Supports Wake on LAN.
* Supports VLAN.
* Support for Energy Efficient Ethernet (EEE).
* The driver is published under GPLv2.


**Support**

In case you are looking for a prebuilt binary or need support, please see the driver's thread on insanelymac.com:

https://www.insanelymac.com/forum/topic/343542-lucyrtl8125ethernetkext-for-realtek-rtl8125/


**Contributions**

If you find my projects useful, please consider to buy me a cup of coffee: https://buymeacoffee.com/mieze

Thank you for your support! Your contribution helps me to continue development.

