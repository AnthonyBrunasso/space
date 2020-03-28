
#include "intel_family.h"

#include <stdio.h>

unsigned int intel_model_duplicates(unsigned int model) {

  switch (model) {
  case INTEL_FAM6_NEHALEM_EP: /* Core i7, Xeon 5500 series - Bloomfield,
                                 Gainstown NHM-EP */
  case INTEL_FAM6_NEHALEM: /* Core i7 and i5 Processor - Clarksfield, Lynnfield,
                              Jasper Forest */
  case 0x1F:               /* Core i7 and i5 Processor - Nehalem */
  case INTEL_FAM6_WESTMERE:    /* Westmere Client - Clarkdale, Arrandale */
  case INTEL_FAM6_WESTMERE_EP: /* Westmere EP - Gulftown */
    return INTEL_FAM6_NEHALEM;

  case INTEL_FAM6_NEHALEM_EX:  /* Nehalem-EX Xeon - Beckton */
  case INTEL_FAM6_WESTMERE_EX: /* Westmere-EX Xeon - Eagleton */
    return INTEL_FAM6_NEHALEM_EX;

  case INTEL_FAM6_XEON_PHI_KNM:
    return INTEL_FAM6_XEON_PHI_KNL;

  case INTEL_FAM6_BROADWELL_X:
  case INTEL_FAM6_BROADWELL_D: /* BDX-DE */
    return INTEL_FAM6_BROADWELL_X;

  case INTEL_FAM6_SKYLAKE_L:
  case INTEL_FAM6_SKYLAKE:
  case INTEL_FAM6_KABYLAKE_L:
  case INTEL_FAM6_KABYLAKE:
    return INTEL_FAM6_SKYLAKE_L;

  case INTEL_FAM6_ICELAKE_L:
  case INTEL_FAM6_ICELAKE_NNPI:
    return INTEL_FAM6_CANNONLAKE_L;

  case INTEL_FAM6_ATOM_TREMONT_D:
    return INTEL_FAM6_ATOM_GOLDMONT_D;
  }
  return model;
}

void intel_main(cpu_t cpu, unsigned long long *tsc_hz) {
  unsigned same_as_model = intel_model_duplicates(cpu.model);
  printf("rewrite intel model 0x%x to 0x%x\n", cpu.model, same_as_model);

  if (cpu.max_level >= 0x15) {
    unsigned int eax_crystal;
    unsigned int ebx_tsc;
    unsigned int crystal_hz;
    unsigned int edx;

    /*
     * CPUID 15H TSC/Crystal ratio, possibly Crystal Hz
     */
    eax_crystal = ebx_tsc = crystal_hz = edx = 0;
    __cpuid(0x15, eax_crystal, ebx_tsc, crystal_hz, edx);

    if (ebx_tsc != 0) {

      printf("CPUID(0x15): eax_crystal: %d ebx_tsc: %d ecx_crystal_hz: %d\n",
             eax_crystal, ebx_tsc, crystal_hz);

      if (crystal_hz == 0)
        switch (same_as_model) {
        case INTEL_FAM6_SKYLAKE_L: /* SKL */
          crystal_hz = 24000000;   /* 24.0 MHz */
          break;
        case INTEL_FAM6_ATOM_GOLDMONT_D: /* DNV */
          crystal_hz = 25000000;         /* 25.0 MHz */
          break;
        case INTEL_FAM6_ATOM_GOLDMONT: /* BXT */
        case INTEL_FAM6_ATOM_GOLDMONT_PLUS:
          crystal_hz = 19200000; /* 19.2 MHz */
          break;
        default:
          crystal_hz = 0;
        }

      if (crystal_hz) {
        *tsc_hz = (unsigned long long)crystal_hz * ebx_tsc / eax_crystal;
        printf("TSC: %lld MHz (%d Hz * %d / %d / 1000000)\n", *tsc_hz / 1000000,
               crystal_hz, ebx_tsc, eax_crystal);
      }
    }
  }
}
