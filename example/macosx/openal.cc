//#include <AL/al.h>
//#include <AL/alc.h>

#include <OpenAL/al.h>
#include <OpenAL/alc.h>

#include <cstdio>
#include <string.h>

void
ListAudioDevices(const ALCchar* devices)
{
  const ALCchar* device = devices, *next = devices + 1;
  size_t len = 0;

  printf("Device List\n");
  while (device && *device != '\0' && next && *next != '\0') {
    printf("%s\n", device);
    len = strlen(device);
    device += (len + 1);
    next += (len + 2);
  }
}

int
main()
{
  ALCdevice* device;
  device = alcOpenDevice(nullptr);
  if (!device) printf("Could not open device.\n");
  ALboolean enumeration;
  enumeration = alcIsExtensionPresent(nullptr, "ALC_ENUMERATION_EXT");
  if (enumeration == AL_FALSE) printf("ALC_ENUMERATION_EXT not present\n");
  ListAudioDevices(alcGetString(nullptr, ALC_DEVICE_SPECIFIER));
  return 0;
}
