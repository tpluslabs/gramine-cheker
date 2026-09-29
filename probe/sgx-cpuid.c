// Prints SGX-related CPUID bits and IA32_FEATURE_CONTROL (MSR 0x3a, root + msr module) as key=value.
#include <cpuid.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    unsigned a, b, c, d;
    __cpuid(1, a, b, c, d);
    printf("hypervisor=%u\n", c >> 31 & 1);

    __cpuid_count(7, 0, a, b, c, d);
    unsigned sgx = b >> 2 & 1;
    printf("sgx=%u\nsgx_lc=%u\nfsgsbase=%u\n", sgx, c >> 30 & 1, b & 1);

    uint64_t epc = 0;
    if (sgx && __get_cpuid_max(0, NULL) >= 0x12) {
        __cpuid_count(0x12, 0, a, b, c, d);
        printf("sgx1=%u\nsgx2=%u\nmax_enclave_log2=%u\n", a & 1, a >> 1 & 1, d >> 8 & 0xff);
        for (unsigned i = 2;; i++) {  // EPC section enumeration
            __cpuid_count(0x12, i, a, b, c, d);
            if ((a & 0xf) != 1)
                break;
            epc += (uint64_t)(c & 0xfffff000) | (uint64_t)(d & 0xfffff) << 32;
        }
    }
    printf("epc_bytes=%llu\n", (unsigned long long)epc);

    uint64_t fc;
    int fd = open("/dev/cpu/0/msr", O_RDONLY);
    if (fd >= 0 && pread(fd, &fc, sizeof fc, 0x3a) == sizeof fc)
        printf("msr_locked=%llu\nmsr_flc=%llu\nmsr_sgx=%llu\n",
               (unsigned long long)(fc & 1), (unsigned long long)(fc >> 17 & 1),
               (unsigned long long)(fc >> 18 & 1));
    return 0;
}
