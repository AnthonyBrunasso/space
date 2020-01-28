#pragma once

#include <cmath>

struct Stats {
  double moments[3];
  double min;
  double max;
};

double
StatsCount(const Stats *accum)
{
  return accum->moments[0];
}

double
StatsMean(const Stats *accum)
{
  return accum->moments[1];
}

double
StatsVariance(const Stats *accum)
{
  return accum->moments[2] / accum->moments[0];
}

double
StatsUnbiasedRsDev(const Stats *accum)
{
  return sqrt(accum->moments[2] / (accum->moments[1] * accum->moments[1] *
                                   (accum->moments[0] - 1)));
}

double
StatsMin(const Stats *accum)
{
  return accum->min;
}

double
StatsMax(const Stats *accum)
{
  return accum->max;
}

void
StatsInit(Stats *accum)
{
  *accum = (Stats){.min = ~0u};
}

void
StatsInitArray(unsigned n, Stats stats[static n])
{
  for (int i = 0; i < n; ++i) {
    StatsInit(&stats[i]);
  }
}

void
StatsAdd(double sample, Stats *accum)
{
  // Calculate
  double n = accum->moments[0];
  double n1 = n + 1.0;
  double diff_from_mean = sample - accum->moments[1];
  double mean_accum = diff_from_mean / n1;
  double delta2 = mean_accum * diff_from_mean * n;

  // Apply
  accum->moments[0] += 1.0;
  accum->moments[1] += mean_accum;
  accum->moments[2] += delta2;

  // Min/max
  accum->max = fmax(sample, accum->max);
  accum->min = fmin(sample, accum->min);
}
