
lsudev
======

lsudev provides easy access to the udev device database. Without arguments, it
simply shows information about devices that have a device file under the /dev
hierarchy. With `-a`, all devices are shown and with `-U` additional information
is shown that helps to create a udev rule for a device, e.g., changing owner and
permissions of a device file or changing the name of a network interface.

lsudev also accepts a search string as parameter which can be used to only show
devices which matching properties.

```
Usage: ./lsudev [args] [search_string]

lsudev provides easy access to the udev device database.

Args:
  -h             show help
  -a             include devices without entry in /dev
  -c             show children of device
  -s <subsystem> limit search to this subsystem
  -p             show all properties of a device
  -G             make output grep friendly
  -U             show relevant parts for a udev rule
```

Examples
--------

Show information about a network interface:
```
$ lsudev -a -p eno2
/sys/devices/pci0000:00/0000:00:1f.6/net/eno2
  KERNEL = eno2
  DEVTYPE = (null)
  DRIVER = e1000e
  DEVPATH = /devices/pci0000:00/0000:00:1f.6/net/eno2
  ID_BUS = pci
  ID_MM_CANDIDATE = 1
  ID_MODEL_FROM_DATABASE = Ethernet Connection (11) I219-LM
  ID_MODEL_ID = 0x0d4c
  ID_NET_DRIVER = e1000e
  ID_NET_LABEL_ONBOARD = Onboard - Ethernet
  ID_NET_LINK_FILE = /lib/systemd/network/99-default.link
  ID_NET_NAME = eno2
  ID_NET_NAME_MAC = enx123456789abc
  ID_NET_NAME_ONBOARD = eno2
  ID_NET_NAME_PATH = enp0s31f6
  ID_NET_NAMING_SCHEME = v245
  ID_PATH = pci-0000:00:1f.6
  ID_PATH_TAG = pci-0000_00_1f_6
  ID_PCI_CLASS_FROM_DATABASE = Network controller
  ID_PCI_SUBCLASS_FROM_DATABASE = Ethernet controller
  ID_VENDOR_FROM_DATABASE = Intel Corporation
  ID_VENDOR_ID = 0x8086
  IFINDEX = 2
  INTERFACE = eno2
  SUBSYSTEM = net
  SYSTEMD_ALIAS = /sys/subsystem/net/devices/eno2
  TAGS = :systemd:
  USEC_INITIALIZED = 1295865
  addr_assign_type = 0
  addr_len = 6
  address = 12:34:56:78:9a:bc
  broadcast = ff:ff:ff:ff:ff:ff
  carrier = 1
  carrier_changes = 704
  carrier_down_count = 352
  carrier_up_count = 352
  dev_id = 0x0
  dev_port = 0
  dormant = 0
  duplex = full
  flags = 0x1003
  gro_flush_timeout = 0
  ifalias = 
  ifindex = 2
  iflink = 2
  link_mode = 0
  mtu = 1500
  name_assign_type = 4
  napi_defer_hard_irqs = 0
  netdev_group = 0
  operstate = up
  proto_down = 0
  speed = 1000
  subsystem = net
  testing = 0
  threaded = 0
  tx_queue_len = 1000
  type = 1
  lsudev_child_devices = 0
```

Or the same without `-p` (i.e. without all properties) but with `-U` ("udev
rule" mode) that shows just the most relevant information for creating a udev
rule and common examples for such rules like renaming a network interface or
changing its MAC address:
```
$ lsudev -a -U eno2
/sys/devices/pci0000:00/0000:00:1f.6/net/eno2
  ATTR{ID_VENDOR_ID}=="0x8086"
  ATTR{ID_MODEL_ID}=="0x0d4c"
  ATTR{ID_NET_NAME_PATH}=="enp0s31f6"
  ATTR{ID_MODEL_FROM_DATABASE}=="Ethernet Connection (11) I219-LM"
  ENV{address}=="12:34:56:78:9a:bc"
  RULE_BUS_TEMPLATE=ACTION=="add", SUBSYSTEM=="net", ATTRS{idVendor}=="0x8086", ATTRS{idProduct}=="0x0d4c",
  RULE_NET_NAME=ACTION=="add", SUBSYSTEM=="net", ATTRS{address}=="12:34:56:78:9a:bc", NAME="<new_name>"
  RULE_NET_MAC=ACTION=="add", SUBSYSTEM=="net", ATTRS{address}=="12:34:56:78:9a:bc", PROGRAM="/sbin/ip link set %k address <new_mac>"
  lsudev_child_devices = 0
```

Some information about a USB-serial converter stick and its sub-devices:
```
$ lsudev -a -c -U /dev/bus/usb/001/025
/dev/bus/usb/001/025
  /sys/devices/pci0000:00/0000:00:14.0/usb1/1-2
  ATTR{ID_VENDOR}=="FTDI"
  ATTR{ID_MODEL}=="TTL232R-3V3"
  ATTR{ID_VENDOR_ID}=="0403"
  ATTR{ID_MODEL_ID}=="6001"
  ATTR{ID_MODEL_FROM_DATABASE}=="FT232 Serial (UART) IC"
  RULE_DEV_OWNER=ACTION=="add", SUBSYSTEM=="usb", KERNEL=="1-2", OWNER="<new_owner>", GROUP="<new_group>", MODE="0660"
  RULE_DEV_SYMLINK=ACTION=="add", SUBSYSTEM=="usb", KERNEL=="1-2", SYMLINK+="<new_name>"
  RULE_BUS_TEMPLATE=ACTION=="add", SUBSYSTEM=="usb", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001",
/sys/devices/pci0000:00/0000:00:14.0/usb1/1-2/1-2:1.0
  ATTR{ID_MODEL_FROM_DATABASE}=="FT232 Serial (UART) IC"
  ENV{interface}=="TTL232R-3V3"
/sys/devices/pci0000:00/0000:00:14.0/usb1/1-2/1-2:1.0/gpio/gpiochip144
/dev/gpiochip1
  /sys/devices/pci0000:00/0000:00:14.0/usb1/1-2/1-2:1.0/gpiochip1
  RULE_DEV_OWNER=ACTION=="add", SUBSYSTEM=="gpio", KERNEL=="gpiochip1", OWNER="<new_owner>", GROUP="<new_group>", MODE="0660"
  RULE_DEV_SYMLINK=ACTION=="add", SUBSYSTEM=="gpio", KERNEL=="gpiochip1", SYMLINK+="<new_name>"
/sys/devices/pci0000:00/0000:00:14.0/usb1/1-2/1-2:1.0/ttyUSB4
/dev/ttyUSB4
  /sys/devices/pci0000:00/0000:00:14.0/usb1/1-2/1-2:1.0/ttyUSB4/tty/ttyUSB4
  ATTR{ID_VENDOR}=="FTDI"
  ATTR{ID_MODEL}=="TTL232R-3V3"
  ATTR{ID_VENDOR_ID}=="0403"
  ATTR{ID_MODEL_ID}=="6001"
  ATTR{ID_MODEL_FROM_DATABASE}=="FT232 Serial (UART) IC"
  RULE_DEV_OWNER=ACTION=="add", SUBSYSTEM=="tty", KERNEL=="ttyUSB4", OWNER="<new_owner>", GROUP="<new_group>", MODE="0660"
  RULE_DEV_SYMLINK=ACTION=="add", SUBSYSTEM=="tty", KERNEL=="ttyUSB4", SYMLINK+="<new_name>"
  RULE_BUS_TEMPLATE=ACTION=="add", SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6001", ENV{ID_USB_INTERFACE_NUM}=="00",
```

