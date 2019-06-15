#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <libusb-1.0/libusb.h>
#include <SDL2/SDL_audio.h>
#include <SDL2/SDL.h>

static libusb_device  *getPad(libusb_device **list) {
  size_t        i;
  struct libusb_device_descriptor desc;

  i = 0;
  while (list[i]) {
    libusb_get_device_descriptor(list[i], &desc);
    if ((desc.idVendor == 0x1235 && desc.idProduct == 0x000e) ||
        (desc.idVendor == 0x09e8 && desc.idProduct == 0x0076) ||
        (desc.idVendor == 0x09e8 && desc.idProduct == 0x0075)) {
      return (list[i]);
    }
    i += 1;
  }
  return (NULL);
}

static int  getKeyNbr(int nbr) {
  return ((8 * (nbr / 16)) + (nbr % 16));
}

static void *play_audio(void *data) {
  SDL_AudioDeviceID dev;
  SDL_AudioSpec     spec;
  Uint32            length;
  Uint8             *buffer;
  char              soundName[16];

  sprintf(soundName, "./sounds/%i.wav", getKeyNbr(*((unsigned char *)data)));
  if (SDL_LoadWAV(soundName, &spec, &buffer, &length) == NULL)
    return (NULL);
  dev = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
  SDL_QueueAudio(dev, buffer, length);
  SDL_PauseAudioDevice(dev, 0);
  while (SDL_GetQueuedAudioSize(dev) != 0)
    ;
  SDL_CloseAudioDevice(dev);
  SDL_FreeWAV(buffer);
  return (data);
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
    if ((error = libusb_interrupt_transfer(handle, 0x81, data, 150, NULL, 0)) != 0)
      dprintf(2, "Couldn't receive info %s\n", libusb_strerror(error));
    if (data[1] == 0)
      continue ;
    pthread_t thread;
    pthread_create(&thread, NULL, play_audio, &(data[0]));
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
    dprintf(2, "Couldn't find any device\n");
    libusb_free_device_list(list, 1);
    libusb_exit(NULL);
    return (1);
  }
  if (SDL_Init(SDL_INIT_AUDIO) == -1) {
    libusb_free_device_list(list, 1);
    libusb_exit(NULL);
    return (1);
  }
  handle_pad(pad);
  libusb_free_device_list(list, 1);
  libusb_exit(NULL);
  return (0);
}

