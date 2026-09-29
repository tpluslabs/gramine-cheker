// Runs inside the enclave: proves launch, then tries to produce a DCAP quote.
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    puts("hello from enclave");

    char type[32] = {0};
    int fd = open("/dev/attestation/attestation_type", O_RDONLY);
    if (fd < 0 || read(fd, type, sizeof type - 1) < 0) {
        perror("attestation_type");
        return 2;
    }
    close(fd);
    printf("attestation_type=%s\n", type);

    char data[64] = "gramine-cheker";
    fd = open("/dev/attestation/user_report_data", O_WRONLY);
    if (fd < 0 || write(fd, data, sizeof data) != sizeof data) {
        perror("user_report_data");
        return 2;
    }
    close(fd);

    static char quote[16384];
    fd = open("/dev/attestation/quote", O_RDONLY);
    ssize_t n = fd < 0 ? -1 : read(fd, quote, sizeof quote);
    if (n <= 0) {
        perror("quote");
        return 2;
    }
    printf("quote_bytes=%zd\n", n);
    return 0;
}
