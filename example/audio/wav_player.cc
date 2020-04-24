#include <cstdio>

#include <cstdio>
#include <string.h>

#include "audio/audio.cc"

#include "platform/platform.cc"

int
main()
{
  audio::Initialize();
  audio::SetListener(v3f(0.f, 0.f, 1.f), v3f(0.f, 0.f, 0.f),
                     v3f(0.f, 0.f, 1.f), v3f(0.f, 1.f, 0.f));
  uint32_t aid = audio::LoadSound("example/audio/test.wav");
  if (!aid) {
    printf("Unable to load sound...\n");
    return 1;
  }
  audio::PlaySound(aid);
  platform::sleep_ms((uint64_t)audio::FindSound(aid)->length_ms);
  return 0;
}
