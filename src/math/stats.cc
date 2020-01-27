#pragma once

#include <cmath>

struct Stats {
  double moments[3];
  double min;
  double max;
};

double
StatsSampleCount(const Stats *accum)
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
StatsRsDev(const Stats *accum)
{
  double mean = StatsMean(accum);
  return StatsVariance(accum) / (mean * mean);
}

double
stats_min(const Stats *accum)
{
  return accum->min;
}

double
stats_max(const Stats *accum)
{
  return accum->max;
}

static void
add_sample(double newValue, Stats *accum)
{
  // Calculate
  double n = StatsSampleCount(accum);
  double n1 = n + 1.0;
  double diff_from_mean = newValue - StatsMean(accum);
  double mean_accum = diff_from_mean / n1;
  double delta2 = mean_accum * diff_from_mean * n;

  // Apply
  accum->moments[0] += 1.0;
  accum->moments[1] += mean_accum;
  accum->moments[2] += delta2;
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
StatsSampleAdd(double sample, Stats *accum)
{
  add_sample(sample, accum);
  accum->max = fmaxl(sample, accum->max);
  accum->min = fminl(sample, accum->min);
}
