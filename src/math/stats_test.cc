
#include <cstdio>

#include "stats.cc"

int
main()
{
  float vals[] = {727.7, 1086.5, 1091.0, 1361.3, 1490.5, 1956.1};
  constexpr int kLen = sizeof(vals) / sizeof(vals[0]);
  float sum = 0.f;
  for (int i = 0; i < kLen; ++i) {
    sum += vals[i];
  }
  float mean = sum / kLen;
  printf("%f mean\n", mean);

  float sq_diff = 0.f;
  for (int i = 0; i < kLen; ++i) {
    float diff = mean - vals[i];
    printf("%d %f diff\n", i, diff);
    sq_diff += diff * diff;
  }
  printf("Sqdiff %f\n", sq_diff);
  float stddev = sqrt(sq_diff / (kLen - 1));
  printf("Stddev %f\n", stddev);

  float rsdev = 100 * stddev / fabs(mean);
  printf("Rsdev %f\n", rsdev);

  // Test Stats
  Stats foo;

  StatsInit(&foo);
  for (int i = 0; i < kLen; ++i) {
    StatsSampleAdd(vals[i], &foo);
  }

  printf("%f samples\n", StatsSampleCount(&foo));
  printf("%f rolling mean\n", StatsMean(&foo));
  printf("%f rolling variance\n", StatsVariance(&foo));
  printf("%f rolling sample variance\n",
         StatsVariance(&foo) / (StatsSampleCount(&foo) - 1));

  float rolling_stddev = sqrt(StatsVariance(&foo) * StatsSampleCount(&foo) /
                              (StatsSampleCount(&foo) - 1));
  printf("%f rolling stddev\n", rolling_stddev);
  float rolling_rsdev = 100 * rolling_stddev / fabs(StatsMean(&foo));
  printf("%f rolling rsdev\n", rolling_rsdev);
  return 0;
}
