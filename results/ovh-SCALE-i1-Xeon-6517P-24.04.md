# try1 - reinstall to 24.04 via OVH API, BIOS untouched (same as 26.04 try3)
## changes
- OS reinstalled with `POST /dedicated/server/{serviceName}/reinstall`, template `ubuntu2404-server_64` (CA API)
- `sudo ./run.sh`: Gramine 1.9 + Intel PSW 2.30 from the `noble` repos, `libsgx-aesm-launch-plugin` 2.27 installed
- BIOS as left on 26.04: TME enabled, SGX enabled, PRMRR 32G (OS sees 93 GB), TME policy 0x2
- `GET /dedicated/server/{serviceName}/biosSettings/sgx` returns 404: OVH offers no SGX toggle for this server

## result
Same as 26.04: zero EPC, so the OS is not the cause.
```
== environment
INFO  host                   ovh-bm-1
INFO  os                     Ubuntu 24.04.5 LTS
INFO  kernel                 6.8.0-142-generic
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
PASS  bios.sgx-enabled       IA32_FEATURE_CONTROL[18]
PASS  bios.flc-enabled       IA32_FEATURE_CONTROL[17]
== kernel
PASS  kernel.version         >= 5.11
PASS  kernel.config          CONFIG_X86_SGX=y
PASS  kernel.flag.sgx        
PASS  kernel.flag.sgx_lc     
PASS  kernel.flag.fsgsbase   
FAIL  dev.sgx_enclave        missing: kernel has no EPC (see epc/dmesg); in a pod: needs sgx.intel.com/enclave
WARN  dev.sgx_provision      missing: DCAP attestation will fail
INFO  dmesg                  [    9.944654] sgx: There are zero EPC sections.
== userspace
PASS  aesmd                  socket present
PASS  dcap.qpl               libdcap_quoteprov
INFO  dcap.pccs              "pccs_url": "https://localhost:8081/sgx/certification/v4/"
PASS  gramine                installed
== functional
PASS  build                  signed (edmm=0)
PASS  run.gramine-direct     
      | Gramine is starting. Parsing TOML manifest file, this may take some time...
      | error: load_enclave() failed with error: No such file or directory (ENOENT)
FAIL  run.gramine-sgx        rc=254
FAIL  attest.dcap-quote      check aesmd, dcap qpl, PCCS reachability, platform registration
== summary
RESULT: NOT READY (5 failures)
report: results/ovh-bm-1-20260930T131649Z.txt
```
