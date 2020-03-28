#include <stdio.h>

#define MAX_HW_PSTATE 10
#define MSR_AMD_PSTATE 0xc0010064
#define MSR_AMD_HWCR 0xc0010015

int
read_msr(unsigned int idx, unsigned long long* val)
{
  FILE* fd;
  char msr_file_name[64];

  const int cpu = 0;
  sprintf(msr_file_name, "/dev/cpu/%d/msr", cpu);
  fd = fopen(msr_file_name, "r");
  if (fd < 0) {
    perror("fopen");
    return -1;
  }
  if (fseek(fd, idx, SEEK_CUR) == -1) {
    perror("seek");
    goto err;
  }
  if (fread(val, sizeof(*val), 1, fd) != 1) {
    perror("read");
    goto err;
  }
  fclose(fd);
  return 0;
err:
  fclose(fd);
  return -1;
}

union msr_pstate {
  struct {
    unsigned fid : 6;
    unsigned did : 3;
    unsigned vid : 7;
    unsigned res1 : 6;
    unsigned nbdid : 1;
    unsigned res2 : 2;
    unsigned nbvid : 7;
    unsigned iddval : 8;
    unsigned idddiv : 2;
    unsigned res3 : 21;
    unsigned en : 1;
  } bits;
  struct {
    unsigned fid : 8;
    unsigned did : 6;
    unsigned vid : 8;
    unsigned iddval : 8;
    unsigned idddiv : 2;
    unsigned res1 : 31;
    unsigned en : 1;
  } fam17h_bits;
  unsigned long long val;
};

static int
get_did(int family, union msr_pstate pstate)
{
  int t;

  if (family == 0x12)
    t = pstate.val & 0xf;
  else if (family == 0x17 || family == 0x18)
    t = pstate.fam17h_bits.did;
  else
    t = pstate.bits.did;

  return t;
}

static int
get_cof(int family, union msr_pstate pstate)
{
  int t;
  int fid, did, cof;

  did = get_did(family, pstate);
  if (family == 0x17 || family == 0x18) {
    fid = pstate.fam17h_bits.fid;
    cof = 200 * fid / did;
  } else {
    t = 0x10;
    fid = pstate.bits.fid;
    if (family == 0x11) t = 0x8;
    cof = (100 * (fid + t)) >> did;
  }
  return cof;
}

void
amd_main(cpu_t cpu)
{
  int boost_count = 0;

  if (cpu.family < 0x10 || cpu.family == 0x14) {
    puts("AMD family should read ACPI tables. Not supported ATM.");
    return;
  }
  // TODO: filter boost states
  if (cpu.amd_boost) {
    puts("AMD family with boost states not yet supported");
    return;
    /* TODO: The first N entries are boost states and not relevant to tsc
     * if (cpu.family == 0x17 || cpu.family == 0x18) {
      if (read_msr(cpu, MSR_AMD_HWCR, &val) == 0) {
        boost_count = !(val & CPUPOWER_AMD_CPBDIS);
      }
    } else {
      boost_count = amd_pci_get_num_boost_states(active, states);
    }*/
  }

  int pstates[MAX_HW_PSTATE];
  int pstate_count = 0;
  for (int i = 0; i < MAX_HW_PSTATE; ++i) {
    union msr_pstate pstate;
    printf("Reading %d\n", i);
    if (read_msr(MSR_AMD_PSTATE + i, &pstate.val)) break;
    if ((cpu.family == 0x17) && (!pstate.fam17h_bits.en))
      continue;
    else if (!pstate.bits.en)
      continue;

    pstates[pstate_count++] = get_cof(cpu.family, pstate);
  }

  for (int i = 0; i < pstate_count; ++i) {
    printf("%d MHz\n", pstates[i]);
  }
}
