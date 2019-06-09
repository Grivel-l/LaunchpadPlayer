#include <stdio.h>
#include <string.h>
#include <libusb-1.0/libusb.h>

static libusb_device  *getPad(libusb_device **list) {
  size_t        i;
  struct libusb_device_descriptor desc;

  i = 0;
  while (list[i]) {
    libusb_get_device_descriptor(list[i], &desc);
    if (desc.idVendor == 0x1235 && desc.idProduct == 0x000e) {
      dprintf(2, "Found launchpad\n");
      return (list[i]);
    }
    i += 1;
  }
  return (NULL);
}

static int  handle_pad(libusb_device *pad) {
  int             error;
  unsigned char   data[150];
  libusb_device_handle  *handle;

  if (libusb_open(pad, &handle) != 0)
    return (-1);
  memset(data, 0, 150);
  if (libusb_kernel_driver_active(handle, 0)) {
    if (libusb_detach_kernel_driver(handle, 0) != 0) {
      dprintf(2, "Couldn't detach kernel driver\n");
      return (-1);
    }
  }
  if ((error = libusb_claim_interface(handle, 0)) != 0) {
    dprintf(2, "Couldn't claim interface |%s|\n", libusb_strerror(error));
    libusb_close(handle);
    return (-1);
  }
  while (1) {
    if ((error = libusb_bulk_transfer(handle, 0x81, data, 150, NULL, 0)) != 0)
      dprintf(2, "Couldn't receive info %s\n", libusb_strerror(error));
    dprintf(2, "Data: %i, %i, %i, %i, %i, %i\n", data[0],data[1], data[2],data[3],data[4],data[5]);
  }
  libusb_release_interface(handle, 0);
  libusb_close(handle);
  return (0);
}

int   main(void) {
  libusb_device *pad;
  libusb_device **list;

  if (libusb_init(NULL) != 0) {
    dprintf(2, "Couldn't init libusb\n");
    return (1);
  }
  if (libusb_get_device_list(NULL, &list) < 0) {
    dprintf(2, "Couldn't get devices list\n");
    return (1);
  }
  if ((pad = getPad(list)) == NULL) {
    libusb_free_device_list(list, 1);
    libusb_exit(NULL);
    return (1);
  }
  handle_pad(pad);
  libusb_free_device_list(list, 1);
  libusb_exit(NULL);
  return (0);
}

