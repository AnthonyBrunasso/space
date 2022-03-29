#pragma once

#include <cmath>

struct Stats {
  r32 moments[3];
  r32 min;
  r32 max;
};

r32 StatsCount(const Stats *accum) {
  return accum->moments[0];
}

r32 StatsMean(const Stats *accum) {
  return accum->moments[1];
}

r32 StatsVariance(const Stats *accum) {
  return accum->moments[2] / accum->moments[0];
}

r32 StatsRsDev(const Stats* accum) {
  return sqrtf(accum->moments[2] / (accum->moments[0] - 1));
}

r32 StatsUnbiasedRsDev(const Stats *accum) {
  return sqrtf(accum->moments[2] / (accum->moments[1] * accum->moments[1] *
                                   (accum->moments[0] - 1)));
}

r32 StatsMin(const Stats *accum) {
  return accum->min;
}

r32 StatsMax(const Stats *accum) {
  return accum->max;
}

void StatsInit(Stats *accum) {
  memset(accum->moments, 0, sizeof(accum->moments));
  accum->min = (r32)~0u;
  accum->max = 0.f;
}

void StatsInitArray(u32 n, Stats *stats) {
  for (u32 i = 0; i < n; ++i) {
    StatsInit(&stats[i]);
  }
}

void StatsAdd(r32 sample, Stats *accum) {
  // Calculate
  r32 n = accum->moments[0];
  r32 n1 = n + 1.0f;
  r32 diff_from_mean = sample - accum->moments[1];
  r32 mean_accum = diff_from_mean / n1;
  r32 delta2 = mean_accum * diff_from_mean * n;

  // Apply
  accum->moments[0] += 1.0f;
  accum->moments[1] += mean_accum;
  accum->moments[2] += delta2;

  // Min/max
  accum->max = fmaxf(sample, accum->max);
  accum->min = fminf(sample, accum->min);
}
