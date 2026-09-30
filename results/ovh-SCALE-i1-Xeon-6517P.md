try1 - default
root@ovh-bm-1:/home/ubuntu/gramine-cheker# ./check.sh 
== environment
INFO  host                   ovh-bm-1
INFO  os                     Ubuntu 26.04.1 LTS
INFO  kernel                 7.0.0-31-generic
INFO  cpu                    Intel(R) Xeon(R) 6517P
INFO  virt                   none
== cpu
PASS  cpuid.sgx              
PASS  cpuid.flc              Flexible Launch Control
PASS  cpuid.fsgsbase         
FAIL  cpuid.sgx1             
WARN  cpuid.sgx2             no SGX2/EDMM: keep sgx.edmm_enable=false
INFO  epc                    0 MiB (enclave working set beyond this pages → slow)
FAIL  epc                    no EPC reported: PRMRR/EPC size 0 in BIOS?
FAIL  bios.sgx-enabled       SGX disabled in BIOS
FAIL  bios.flc-enabled       BIOS locked launch-enclave hash (set 'SGX Launch Control Policy: Unlocked')
== kernel
PASS  kernel.version         >= 5.11
PASS  kernel.config          CONFIG_X86_SGX=y
FAIL  kernel.flag.sgx        kernel did not enable SGX (BIOS/driver) — see dmesg
FAIL  kernel.flag.sgx_lc     
PASS  kernel.flag.fsgsbase   
FAIL  dev.sgx_enclave        missing (pod: needs sgx.intel.com/enclave from Intel SGX device plugin)
WARN  dev.sgx_provision      missing: DCAP attestation will fail
INFO  dmesg                  [    3.964022] x86/cpu: SGX disabled or unsupported by BIOS.
== userspace
WARN  aesmd                  no /var/run/aesmd/aesm.socket: quotes need aesmd
WARN  dcap.qpl               libsgx-dcap-default-qpl missing
FAIL  gramine                not installed: run ./install.sh
== summary
RESULT: NOT READY (8 failures)