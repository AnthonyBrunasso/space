static inline void native_cpuid(unsigned int *eax, unsigned int *ebx,
                                unsigned int *ecx, unsigned int *edx) {
  /* ecx is often an input as well as an output. */
  asm volatile("cpuid"
               : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
               : "0"(*eax), "2"(*ecx)
               : "memory");
}

static inline void cpuid(unsigned int op, unsigned int *eax, unsigned int *ebx,
                         unsigned int *ecx, unsigned int *edx) {
  *eax = op;
  *ecx = 0;
  native_cpuid(eax, ebx, ecx, edx);
}

#define __cpuid(op, a, b, c, d) cpuid(op, &a, &b, &c, &d);

typedef struct {
  unsigned max_level;
  unsigned max_extended_level;
  unsigned genuine_intel;
  unsigned authentic_amd;
  unsigned family, model, stepping;
  unsigned amd_boost;
  unsigned long long tsc_hz;
} cpu_t;

#include "intel_cpuid.c"
#include "amd_cpuid.c"

#include <stdio.h>

static cpu_t cpu;

int main() {
  unsigned int eax, ebx, ecx, edx;
  __cpuid(0, cpu.max_level, ebx, ecx, edx);
  if (ebx == 0x756e6547 && ecx == 0x6c65746e && edx == 0x49656e69)
    cpu.genuine_intel = 1;
  else if (ebx == 0x68747541 && ecx == 0x444d4163 && edx == 0x69746e65)
    cpu.authentic_amd = 1;
  printf("%.4s%.4s%.4s\n", (char *)&ebx, (char *)&edx, (char *)&ecx);

  unsigned fms;
  unsigned ecx_flags, edx_flags;
  __cpuid(1, fms, ebx, ecx_flags, edx_flags);
  unsigned family = (fms >> 8) & 0xf;
  unsigned model = (fms >> 4) & 0xf;
  unsigned stepping = fms & 0xf;
  if (family == 0xf)
    family += (fms >> 20) & 0xff;
  if (family >= 6)
    model += ((fms >> 16) & 0xf) << 4;
  printf("family 0x%x model 0x%x stepping 0x%x\n", family, model, stepping);
  cpu.family = family;
  cpu.model = model;
  cpu.stepping = stepping;

  __cpuid(0x80000000, cpu.max_extended_level, ebx, ecx, edx);

  unsigned ext_edx_flags = 0;
  if (cpu.max_extended_level >= 0x80000007) {
    /*
     * Non-Stop TSC is advertised by CPUID.EAX=0x80000007: EDX.bit8
     * this check is valid for both Intel and AMD
     */
    __cpuid(0x80000007, eax, ebx, ecx, ext_edx_flags);
  }
  printf("%s %s\n", ecx_flags & (1 << 4) ? "TSC" : "",
         ext_edx_flags & (1 << 8) ? "Invariant" : "Variant");
  cpu.amd_boost = ext_edx_flags & (1<<9);

  if (cpu.genuine_intel) {
    intel_main(cpu, &cpu.tsc_hz);
  }
  else if (cpu.authentic_amd) {
    amd_main(cpu);
  }

  return 0;
}
