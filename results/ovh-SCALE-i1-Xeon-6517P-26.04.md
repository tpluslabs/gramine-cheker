# try3 - bios, SGX factory reset and
CPU
- SMX: disabled
- Total Memory Encryption (TME): Enabled.
- TME algorithm: AES-XTS-256
- TDX: Enabled
- SW Guard Extensions (SGX): Enabled
- SGX PRMRR Size 32Gb
- TME-MT: enabled
- SNC: enabled
# result
== environment
INFO  host                   ovh-bm-1
INFO  os                     Ubuntu 26.04.1 LTS
INFO  kernel                 7.0.0-34-generic
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
INFO  dmesg                  [    6.620397] sgx: There are zero EPC sections.
== userspace
PASS  aesmd                  socket present
PASS  dcap.qpl               libdcap_quoteprov
INFO  dcap.pccs              "pccs_url": "https://localhost:8081/sgx/certification/v4/"
PASS  gramine                installed
INFO  is-sgx-available       SGX supported by CPU: true
INFO  is-sgx-available       SGX1 (ECREATE, EENTER, ...): false
INFO  is-sgx-available       SGX2 (EAUG, EACCEPT, EMODPR, ...): false
INFO  is-sgx-available       Flexible Launch Control (IA32_SGXPUBKEYHASH{0..3} MSRs): true
INFO  is-sgx-available       SGX extensions for virtualizers (EINCVIRTCHILD, EDECVIRTCHILD, ESETCONTEXT): false
INFO  is-sgx-available       Extensions for concurrent memory management (ETRACKC, ELDBC, ELDUC, ERDINFO): false
INFO  is-sgx-available       EDECCSSA instruction: false
INFO  is-sgx-available       CET enclave attributes support (See Table 37-5 in the SDM): false
INFO  is-sgx-available       Key separation and sharing (KSS) support (CONFIGID, CONFIGSVN, ISVEXTPRODID, ISVFAMILYID report fields): false
INFO  is-sgx-available       AEX-Notify: false
INFO  is-sgx-available       Max enclave size (32-bit): 0x1
INFO  is-sgx-available       Max enclave size (64-bit): 0x1
INFO  is-sgx-available       EPC size: 0x0
INFO  is-sgx-available       SGX driver loaded: false
INFO  is-sgx-available       AESMD installed: true
INFO  is-sgx-available       SGX PSW/libsgx installed: false
INFO  is-sgx-available       #PF/#GP information in EXINFO in MISC region of SSA supported: false
INFO  is-sgx-available       #CP information in EXINFO in MISC region of SSA supported: false
== functional
PASS  build                  signed (edmm=0)
PASS  run.gramine-direct     
      | Gramine is starting. Parsing TOML manifest file, this may take some time...
      | error: Cannot open /dev/sgx_enclave (No such file or directory (ENOENT)). This may happen because the current user has insufficientpermissions to this device, your kernel is too old or your machine doesn't support SGX. Use is-sgx-available tool to get more information.
      | error: load_enclave() failed with error: No such file or directory (ENOENT)
FAIL  run.gramine-sgx        rc=254
FAIL  attest.dcap-quote      check aesmd, dcap qpl, PCCS reachability, platform registration
== summary
RESULT: NOT READY (5 failures)
report: results/ovh-bm-1-202609
# try2 - bios
## changes
CPU config
- SMX: disabled
- Total Memory Encryption (TME): Enabled.
- TME algorithm: AES-XTS-256
- SW Guard Extensions (SGX): Enabled.
- SGX PRMRR size: 64GB, half of real mem
- SGX Launch Control / Auto MP Registration: leave unlocked / enabled if shown.
- TME-MT, memory integrity, TDX, TDX secure arbitration, key split │ Disabled / 1 │ Only matter for TDX; leave.
## result
install E: Unable to locate package libsgx-aesm-launch-plugin

root@ovh-bm-1:/home/ubuntu/gramine-cheker# ./check.sh 
== environment
INFO  host                   ovh-bm-1
INFO  os                     Ubuntu 26.04.1 LTS
INFO  kernel                 7.0.0-34-generic
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
FAIL  dev.sgx_enclave        missing (pod: needs sgx.intel.com/enclave from Intel SGX device plugin)
WARN  dev.sgx_provision      missing: DCAP attestation will fail
INFO  dmesg                  [    6.487469] sgx: There are zero EPC sections.
== userspace
WARN  aesmd                  no /var/run/aesmd/aesm.socket: quotes need aesmd
WARN  dcap.qpl               libsgx-dcap-default-qpl missing
FAIL  gramine                not installed: run ./install.sh
== summary
RESULT: NOT READY (4 failures)



# try1 - default
## result
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