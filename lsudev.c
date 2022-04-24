
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <libudev.h>

struct lsudev {
	struct udev *udev;
	
	char show_all_properties;
	char grep_output;
	char show_udev_rule;
	char show_all_devices;
	char *query_subsystem;
	char *wildcard;
};

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

const char *selected_props[] = {
	"ID_MODEL",
	"ID_BUS",
	"ID_VENDOR_ID",
	"ID_MODEL_ID",
	"ID_NET_NAME_PATH",
	"ID_MODEL_FROM_DATABASE",
	0,
};

const char *selected_attrs[] = {
	"address",
	0,
};

void print_udev_rule(struct udev_device *dev, const char *prefix) {
	const char *value, *subsystem;
	
	subsystem = udev_device_get_subsystem(dev);
	
	value = udev_device_get_devnode(dev);
	if (value) {
		value = udev_device_get_sysname(dev);
		printf("%s  RULE_DEV_OWNER=ACTION==\"add\", SUBSYSTEM==\"%s\", KERNEL==\"%s\", OWNER=\"<new_owner>\", GROUP=\"<new_group>\", MODE=\"0660\"\n", prefix, subsystem, value);
		printf("%s  RULE_DEV_SYMLINK=ACTION==\"add\", SUBSYSTEM==\"%s\", KERNEL==\"%s\", SYMLINK+=\"<new_name>\"\n", prefix, subsystem, value);
	}
	
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
		
		printf("%s  RULE_BUS_TEMPLATE=ACTION==\"add\", SUBSYSTEM==\"%s\", ATTRS{idVendor}==\"%s\", ATTRS{idProduct}==\"%s\"", prefix, subsystem, vendor, model);
		
		value = udev_device_get_property_value(dev, "ID_USB_INTERFACE_NUM");
		if (value)
			printf(", ENV{ID_USB_INTERFACE_NUM}==\"%s\"", value);
		
		printf(",\n");
	}
	
	value = udev_device_get_sysattr_value(dev, "address");
	if (value) {
		printf("%s  RULE_NET_NAME=ACTION==\"add\", SUBSYSTEM==\"%s\", ATTRS{address}==\"%s\", NAME=\"<new_name>\"\n", prefix, subsystem, value);
		printf("%s  RULE_NET_MAC=ACTION==\"add\", SUBSYSTEM==\"%s\", ATTRS{address}==\"%s\", PROGRAM=\"/sbin/ip link set %%k address <new_mac>\"\n", prefix, subsystem, value);
	}
}

#define RULE_NONE 0
#define RULE_ENV 1
#define RULE_ATTR 2

void print_key(const char *prefix, const char *key, const char *value, int env_or_attr, int show_udev_rule) {
	if (show_udev_rule) {
		printf("%s  %s%s%s==\"%s\"\n",
			prefix,
			env_or_attr == 0 ? "" : (env_or_attr == 1 ? "ATTR{":"ENV{"),
			key,
			env_or_attr == 0 ? "" : "}",
			value
			);
	} else {
		printf("%s  %s = %s\n", prefix, key, value);
	}
}

int process_device(struct lsudev *lsudev, struct udev_list_entry *dev_list_entry) {
	const char *path, *subsystem;
	struct udev_device *dev, *dev_parent;
	int show;
	struct udev_list_entry *entry;
	const char *key, *value, *prefix;
	
	
	show = 0;
	
	path = udev_list_entry_get_name(dev_list_entry);
	dev = udev_device_new_from_syspath(lsudev->udev, path);
	
	subsystem = udev_device_get_subsystem(dev);
	if (lsudev->query_subsystem && strcmp(lsudev->query_subsystem, subsystem)) {
		udev_device_unref(dev);
		return 1;
	}
	
	if (lsudev->wildcard) {
		if (strstr(subsystem, lsudev->wildcard))
			show = 1;
	} else {
		show = 1;
	}
	
	if (udev_device_get_devnode(dev) || lsudev->show_all_devices) {
		char buf[512];
		const char *name;
		
		name = udev_device_get_devnode(dev);
		if (!name) {
			snprintf(buf, sizeof(buf), "/sys%s", udev_device_get_devpath(dev));
			name = buf;
		}
		
		if (!show && (strstr(name, lsudev->wildcard)))
			show = 1;
		if (!show && (strstr(name, lsudev->wildcard)))
			show = 1;
		if (!show) {
			udev_list_entry_foreach(entry, udev_device_get_properties_list_entry(dev)) {
				value = udev_list_entry_get_value(entry);
				
				if (!show && (strstr(value, lsudev->wildcard))) {
					show = 1;
					break;
				}
			}
		}
		
		if (show) {
			if (lsudev->grep_output) {
				prefix = name;
			} else {
				printf("%s\n", name);
				prefix = "";
			}
			
			if (lsudev->show_all_properties) {
				print_key(prefix, "KERNEL", udev_device_get_sysname(dev), RULE_NONE, lsudev->show_udev_rule);
				print_key(prefix, "DEVTYPE", udev_device_get_devtype(dev), RULE_NONE, lsudev->show_udev_rule);
				
				dev_parent = udev_device_get_parent(dev);
				if (dev_parent)
					print_key(prefix, "DRIVER", udev_device_get_driver(dev_parent), RULE_ENV, lsudev->show_udev_rule);
			} else {
				const char **iter;
				
				if (udev_device_get_devnode(dev))
					printf("%s  /sys%s\n", prefix, udev_device_get_devpath(dev));
				
				iter = selected_props;
				while (*iter) {
					value = udev_device_get_property_value(dev, *iter);
					if (value)
						print_key(prefix, *iter, value, RULE_ENV, lsudev->show_udev_rule);
					
					iter++;
				}
				
				iter = selected_attrs;
				while (*iter) {
					value = udev_device_get_sysattr_value(dev, *iter);
					if (value)
						print_key(prefix, *iter, value, RULE_ATTR, lsudev->show_udev_rule);
					
					iter++;
				}
			}
			
			if (lsudev->show_all_properties) {
				udev_list_entry_foreach(entry, udev_device_get_properties_list_entry(dev)) {
					key = udev_list_entry_get_name(entry);
					value = udev_list_entry_get_value(entry);
					
					print_key(prefix, key, value, RULE_ENV, lsudev->show_udev_rule);
				}
				
				udev_list_entry_foreach(entry, udev_device_get_sysattr_list_entry(dev)) {
					key = udev_list_entry_get_name(entry);
					
					if (!strcmp(key, "uevent"))
						continue;
					
					value = udev_device_get_sysattr_value(dev, key);
					
					if (!value)
						continue;
					
					print_key(prefix, key, value, RULE_ATTR, lsudev->show_udev_rule);
				}
			}
			
			if (lsudev->show_udev_rule) {
				print_udev_rule(dev, prefix);
			}
		}
	}
	
	udev_device_unref(dev);
	
	return 0;
}

int main(int argc, char **argv) {
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;
	int opt;
	struct lsudev lsudev = {0};
	
	
	while ((opt = getopt (argc, argv, "has:pGU")) != -1) {
		switch (opt) {
			case 'h':
				help(argv);
				return 0;
			case 'a':
				lsudev.show_all_devices = 1;
				break;
			case 's':
				lsudev.query_subsystem = strdup(optarg);
				break;
			case 'p':
				lsudev.show_all_properties = 1;
				break;
			case 'G':
				lsudev.grep_output = 1;
				break;
			case 'U':
				lsudev.show_udev_rule = 1;
				break;
		}
	}
	
	if (optind < argc)
		lsudev.wildcard = argv[optind];
	
	lsudev.udev = udev_new();
	if (!lsudev.udev) {
		fprintf(stderr, "error: udev_new() failed\n");
		return 1;
	}
	
	enumerate = udev_enumerate_new(lsudev.udev);
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
		process_device(&lsudev, dev_list_entry);
	}
	
	udev_enumerate_unref(enumerate);
	udev_unref(lsudev.udev);
	
	return 0;
}
