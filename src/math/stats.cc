#pragma once

#include <cmath>

struct Stats {
  float moments[3];
  float min;
  float max;
};

float
StatsCount(const Stats *accum)
{
  return accum->moments[0];
}

float
StatsMean(const Stats *accum)
{
  return accum->moments[1];
}

float
StatsVariance(const Stats *accum)
{
  return accum->moments[2] / accum->moments[0];
}

float
StatsUnbiasedRsDev(const Stats *accum)
{
  return sqrtf(accum->moments[2] / (accum->moments[1] * accum->moments[1] *
                                   (accum->moments[0] - 1)));
}

float
StatsMin(const Stats *accum)
{
  return accum->min;
}

float
StatsMax(const Stats *accum)
{
  return accum->max;
}

void
StatsInit(Stats *accum)
{
  memset(accum->moments, 0, sizeof(accum->moments));
  accum->min = ~0u;
  accum->max = 0.f;
}

void
StatsInitArray(unsigned n, Stats *stats)
{
  for (int i = 0; i < n; ++i) {
    StatsInit(&stats[i]);
  }
}

void
StatsAdd(float sample, Stats *accum)
{
  // Calculate
  float n = accum->moments[0];
  float n1 = n + 1.0;
  float diff_from_mean = sample - accum->moments[1];
  float mean_accum = diff_from_mean / n1;
  float delta2 = mean_accum * diff_from_mean * n;

  // Apply
  accum->moments[0] += 1.0;
  accum->moments[1] += mean_accum;
  accum->moments[2] += delta2;

  // Min/max
  accum->max = fmaxf(sample, accum->max);
  accum->min = fminf(sample, accum->min);
}
