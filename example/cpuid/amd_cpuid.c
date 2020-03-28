
#define MAX_HW_PSTATES 10
#define MSR_AMD_PSTATE_LIMIT  0xc0010061

int read_msr(unsigned int idx, unsigned long long *val)
{
  int fd;
  char msr_file_name[64];

  const int cpu = 0;
  sprintf(msr_file_name, "/dev/cpu/%d/msr", cpu);
  fd = open(msr_file_name, O_RDONLY);
  if (fd < 0)
    return -1;
  if (lseek(fd, idx, SEEK_CUR) == -1)
    goto err;
  if (read(fd, val, sizeof *val) != sizeof *val)
    goto err;
  close(fd);
  return 0;
 err:
  close(fd);
  return -1;
}

void
amd_main(cpu_t cpu)
{
  if (cpu.family < 0x10 || cpu.family == 0x14) return;

  unsigned long long val;
  if (read_msr(cpu, MSR_AMD_PSTATE_LIMIT, &val))
    return -1;

  printf("%ull msr\n", val);
}
