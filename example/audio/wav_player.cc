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
  uint32_t aid1 = audio::LoadSound("example/audio/test.wav");
  if (!aid1) {
    printf("Unable to load sound...\n");
    return 1;
  }
  uint32_t aid2 = audio::LoadSound("example/audio/step.wav");
  if (!aid2) {
    printf("Unable to load sound...\n");
    return 1;
  }
  audio::PlaySound(aid1);
  audio::PlaySound(aid2);
  platform::sleep_ms((uint64_t)audio::FindSound(aid1)->length_ms);
  return 0;
}
