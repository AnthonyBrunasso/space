
static const char* kEmptyString = "";

EXTERN(s32 platform_optind = 1);
EXTERN(const char* platform_optarg);

s32
platform_getopt(s32 argc, char* const argv[], const char* optstring)
{
  for (; platform_optind < argc; ++platform_optind) {
    const char* arg = argv[platform_optind];
    platform_optarg = kEmptyString;

    const char* optiter = optstring;
    for (; *optiter; ++optiter) {
      s32 val_param = *(optiter + 1) == ':';
      if (arg[0] == '-' && *optiter == arg[1]) {
        unsigned used_param = 1;
        if (val_param && platform_optind + 1 < argc) {
          platform_optarg = argv[platform_optind + 1];
          used_param += 1;
        }
        platform_optind += used_param;
        return *optiter;
      }
    }
    break;
  }

  return -1;
}
