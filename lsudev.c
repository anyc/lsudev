
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <libudev.h>

void help(char **argv) {
	fprintf(stdout, "Usage: %s [args] [search_string]\n", argv[0]);
	fprintf(stdout, "\n");
	fprintf(stdout, "lsudev provides easy access to the udev device database.\n");
	fprintf(stdout, "\n");
	fprintf(stdout, "Args:\n");
	fprintf(stdout, "  -h             show help\n");
	fprintf(stdout, "  -a             include devices without entry in /dev\n");
	fprintf(stdout, "  -s <subsystem> limit search to this subsystem\n");
	fprintf(stdout, "  -p             show all properties of a device\n");
	fprintf(stdout, "  -G             make output grep friendly\n");
	fprintf(stdout, "  -U             show relevant parts for a udev rule\n");
}

void print_udev_rule(struct udev_device *dev, const char *prefix) {
	const char *value, *subsystem;
	
	subsystem = udev_device_get_subsystem(dev);
	
	value = udev_device_get_property_value(dev, "ID_BUS");
	if (
		value && (
			!strcmp(value, "usb")
			|| !strcmp(value, "pci")
		)
	)
	{
		const char *vendor, *model;
		vendor = udev_device_get_property_value(dev, "ID_VENDOR_ID");
		model = udev_device_get_property_value(dev, "ID_MODEL_ID");
		
		printf("%s  RULE=SUBSYSTEM==\"%s\", ATTRS{idVendor}==\"%s\", ATTRS{idProduct}==\"%s\"", prefix, subsystem, vendor, model);
		
		value = udev_device_get_property_value(dev, "ID_USB_INTERFACE_NUM");
		if (value)
			printf(", ENV{ID_USB_INTERFACE_NUM}==\"%s\"", value);
		
		printf(",\n");
	}
}

int main(int argc, char **argv) {
	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;
	int opt, show_all_properties, grep_output, show_udev_rule, show_all_devices;
	char *query_subsystem, *wildcard;
	
	
	show_all_devices = 0;
	query_subsystem = 0;
	wildcard = 0;
	show_all_properties = 0;
	grep_output = 0;
	show_udev_rule = 0;
	while ((opt = getopt (argc, argv, "has:pGU")) != -1) {
		switch (opt) {
			case 'h':
				help(argv);
				return 0;
			case 'a':
				show_all_devices = 1;
				break;
			case 's':
				query_subsystem = strdup(optarg);
				break;
			case 'p':
				show_all_properties = 1;
				break;
			case 'G':
				grep_output = 1;
				break;
			case 'U':
				show_udev_rule = 1;
				break;
		}
	}
	
	if (optind < argc)
		wildcard = argv[optind];
	
	udev = udev_new();
	if (!udev) {
		fprintf(stderr, "error: udev_new() failed\n");
		return 1;
	}
	
	enumerate = udev_enumerate_new(udev);
	if (!enumerate) {
		fprintf(stderr, "error: udev_enumerate_new() failed\n");
		return 1;
	}
	
	udev_enumerate_scan_devices(enumerate);
	
	devices = udev_enumerate_get_list_entry(enumerate);
	if (!devices) {
		fprintf(stderr, "error: udev_enumerate_get_list_entry() failed\n");
		return 1;
	}
	
	udev_list_entry_foreach(dev_list_entry, devices) {
		const char *path, *subsystem;
		struct udev_device *dev, *dev_parent;
		int show;
		struct udev_list_entry *entry;
		const char *key, *value, *prefix;
		
		
		show = 0;
		
		path = udev_list_entry_get_name(dev_list_entry);
		dev = udev_device_new_from_syspath(udev, path);
		
		subsystem = udev_device_get_subsystem(dev);
		if (query_subsystem && strcmp(query_subsystem, subsystem)) {
			udev_device_unref(dev);
			continue;
		}
		
		if (wildcard) {
			if (strstr(subsystem, wildcard))
				show = 1;
		} else {
			show = 1;
		}
		
		if (udev_device_get_devnode(dev) || show_all_devices) {
			char buf[512];
			const char *name;
			
			name = udev_device_get_devnode(dev);
			if (!name) {
				snprintf(buf, sizeof(buf), "/sys%s", udev_device_get_devpath(dev));
				name = buf;
			}
			
			if (!show && (strstr(name, wildcard)))
				show = 1;
			if (!show && (strstr(name, wildcard)))
				show = 1;
			if (!show) {
				udev_list_entry_foreach(entry, udev_device_get_properties_list_entry(dev)) {
					value = udev_list_entry_get_value(entry);
					
					if (!show && (strstr(value, wildcard))) {
						show = 1;
						break;
					}
				}
			}
			
			if (show) {
				if (grep_output) {
					prefix = name;
				} else {
					printf("%s\n", name);
					prefix = "";
				}
				
				if (show_all_properties) {
					printf("%s  KERNEL=%s\n", prefix, udev_device_get_sysname(dev));
					printf("%s  DEVTYPE=%s\n", prefix, udev_device_get_devtype(dev));
					
					dev_parent = udev_device_get_parent(dev);
					if (dev_parent)
						printf("%s  DRIVER=%s\n", prefix, udev_device_get_driver(dev_parent));
				} else {
					if (udev_device_get_devnode(dev))
						printf("%s  /sys%s\n", prefix, udev_device_get_devpath(dev));
					
					value = udev_device_get_property_value(dev, "ID_MODEL");
					if (value)
						printf("%s  MODEL=%s\n", prefix, value);
					
					value = udev_device_get_property_value(dev, "ID_BUS");
					if (
						value && (
							!strcmp(value, "usb")
							|| !strcmp(value, "pci")
							)
						)
					{
						const char *vendor, *model;
						vendor = udev_device_get_property_value(dev, "ID_VENDOR_ID");
						if (vendor)
							printf("%s  VENDOR_ID=%s\n", prefix, vendor);
						
						model = udev_device_get_property_value(dev, "ID_MODEL_ID");
						if (model)
							printf("%s  MODEL_ID=%s\n", prefix, model);
					}
				}
				
				if (show_all_properties) {
					udev_list_entry_foreach(entry, udev_device_get_properties_list_entry(dev)) {
						key = udev_list_entry_get_name(entry);
						value = udev_list_entry_get_value(entry);
						printf("%s  %s = %s\n", prefix, key, value);
					}
					
					udev_list_entry_foreach(entry, udev_device_get_sysattr_list_entry(dev)) {
						key = udev_list_entry_get_name(entry);
						
						if (!strcmp(key, "uevent"))
							continue;
						
						value = udev_device_get_sysattr_value(dev, key);
						
						if (!value)
							continue;
						
						printf("%s  %s = %s\n", prefix, key, value);
					}
				}
				
				if (show_udev_rule) {
					print_udev_rule(dev, prefix);
				}
			}
		}
		
		udev_device_unref(dev);
	}
	
	udev_enumerate_unref(enumerate);
	udev_unref(udev);
	
	return 0;
}
