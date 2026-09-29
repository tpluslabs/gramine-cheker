# gramine-cheker

Finds hosts that can really run [Gramine](https://gramine.readthedocs.io) with Intel SGX (`gramine-sgx`),
from GKE pods and GCP VMs to bare metal in colocation. It checks the whole chain:
CPU → BIOS → hypervisor → kernel → userspace → enclave launch → DCAP quote.

```bash
git clone https://github.com/tpluslabs/gramine-cheker && cd gramine-cheker
./check.sh --no-run      # fast, read-only: hardware/BIOS/kernel/userspace
sudo ./install.sh        # Gramine + aesmd + DCAP QPL (Ubuntu 22.04/24.04, Debian 12)
sudo ./check.sh          # full: also builds, signs and runs a real enclave and asks for a quote
```

Run as root to get the BIOS checks (they read MSR `0x3a`). Each run saves a report to
`results/<host>-<utc>.txt`. The exit code is the number of FAILs; `0` means ready.
Kubernetes: `kubectl apply -f k8s/pod.yaml` (see the comments in the file).

| Path | What |
|---|---|
| `check.sh` | the test runner |
| `probe/sgx-cpuid.c` | reads raw CPUID leaf 7/0x12 and `IA32_FEATURE_CONTROL`, so it can tell "CPU lacks SGX" apart from "BIOS disabled it". Built on the fly if `cc` exists, else downloaded prebuilt from the `latest` release |
| `.github/workflows/build.yml` | CI: lints the scripts and builds the probe as a static binary on every push/PR. Pushes to `main` update the rolling [`latest`](https://github.com/tpluslabs/gramine-cheker/releases/tag/latest) prerelease; `v*` tags get a versioned release. The binary ships with a `.sha256` |
| `app/` | minimal enclave: prints `hello from enclave`, then gets a DCAP quote via `/dev/attestation/quote` |
| `install.sh` | installs the runtime dependencies |
| `k8s/pod.yaml` | runs the check inside a pod |

---

## 1. What Gramine-SGX needs (all of it)

Gramine ≥ 1.9 supports **only** the upstream in-kernel driver, so **FLC is mandatory**.

| # | Layer | Requirement | Checked by |
|---|---|---|---|
| 1 | CPU | Intel CPU with SGX (`CPUID.7.0:EBX[2]`) | `cpuid.sgx` |
| 2 | CPU | Flexible Launch Control (`CPUID.7.0:ECX[30]`, `sgx_lc`) | `cpuid.flc` |
| 3 | CPU | FSGSBASE | `cpuid.fsgsbase` |
| 4 | CPU | SGX2/EDMM: optional, but needed for `sgx.edmm_enable=true` (dynamic memory, faster startup) | `cpuid.sgx2` |
| 5 | BIOS | SGX = **Enabled** (not "Software Controlled"), `IA32_FEATURE_CONTROL[18]` | `bios.sgx-enabled` |
| 6 | BIOS | SGX Launch Control Policy = **Unlocked** (`IA32_FEATURE_CONTROL[17]`) | `bios.flc-enabled` |
| 7 | BIOS | PRMRR/EPC size > 0; on Xeon Scalable, TME/TME-MT enabled (needed for SGX), and on multi-socket, UMA/NUMA settings as the board vendor requires | `epc` |
| 8 | Hypervisor | For VMs: the VMM has to expose SGX and an EPC section to the guest. **No public cloud except Azure DC-series and Alibaba g7t/c7t/r7t does this.** TDX/SEV guests can't use SGX | `cpuid.*`, `confidential-vm` |
| 9 | Kernel | Linux ≥ 5.11, `CONFIG_X86_SGX=y`, FSGSBASE enabled (≥ 5.9, no `nofsgsbase`) | `kernel.*` |
| 10 | Kernel | `/dev/sgx_enclave` (rw for the user), `/dev/sgx_provision` (for attestation), `/dev` **not** `noexec` | `dev.*` |
| 11 | Containers | Device nodes passed in (k8s: [Intel SGX device plugin](https://github.com/intel/intel-device-plugins-for-kubernetes), resources `sgx.intel.com/{epc,enclave,provision}`), aesmd socket reachable | `dev.*`, `aesmd` |
| 12 | Userspace | `gramine` package (`gramine-sgx`, `gramine-manifest`, `gramine-sgx-sign`, `is-sgx-available`), signing key from `gramine-sgx-gen-private-key` (RSA-3072) | `gramine`, `build` |
| 13 | Attestation | `sgx-aesm-service` + `libsgx-aesm-{launch,quote-ex,ecdsa}-plugin` (aesmd running, `/var/run/aesmd/aesm.socket`) | `aesmd` |
| 14 | Attestation | `libsgx-dcap-default-qpl` + `/etc/sgx_default_qcnl.conf` pointing at a reachable PCCS (or at your provider's cache). EPID/IAS is dead (EOL April 2025), so it's DCAP only | `dcap.*` |
| 15 | Attestation | Platform registered with Intel PCS. Single-socket boxes are fine by default; **multi-socket boxes need multi-package registration** (`sgx-ra-service` / MPA) before a PCK cert exists | `attest.dcap-quote` |
| 16 | Keep current | TCB: microcode/BIOS recovery. An out-of-date TCB still produces quotes, but verifiers will report `OUT_OF_DATE`/`SW_HARDENING_NEEDED` | manual |

The only full proof is the end-to-end test: `run.gramine-sgx` (enclave launched in production mode, `sgx.debug=false`)
plus `attest.dcap-quote`.

---

## 2. CPUs that can run Gramine-SGX

"Capable" here means SGX **with FLC**. Always check the exact SKU on [ark.intel.com](https://ark.intel.com)
("Intel SGX" and "Max Enclave Size") and confirm it with `check.sh`.

### ✅ Usable

| Family | Codename | SGX2/EDMM | EPC (per socket) | Notes |
|---|---|---|---|---|
| Xeon E-2100 (E-21xx) | Coffee Lake | no | 128 MB (~93 MB usable) | single socket |
| Xeon E-2200 (E-22xx) | Coffee Lake-R | no | 128 MB | single socket |
| Xeon E-2300 (E-23xx) | Rocket Lake | script reports it | up to 512 MB | single socket |
| Xeon D-1700 / D-2700 | Ice Lake-D | yes | per SKU | edge/embedded |
| Xeon Scalable 3rd gen (x3xx, **not** Cooper Lake x3xxH) | Ice Lake-SP | yes | 8–512 GB, per SKU | first server line with big EPC |
| Xeon Scalable 4th gen (x4xx) | Sapphire Rapids | yes | up to 512 GB, per SKU | also has TDX |
| Xeon Scalable 5th gen (x5xx) | Emerald Rapids | yes | up to 512 GB, per SKU | also has TDX |
| Xeon 6 P-core (6500P/6700P/6900P) | Granite Rapids | yes | per SKU | also has TDX |
| Pentium/Celeron J/N 4xxx/5xxx | Gemini Lake | yes | 128 MB | lab/dev only |

Verify on ARK before buying: Xeon E-2400 (Raptor Lake-E), Xeon W-3300, Xeon 6 E-core (Sierra Forest).

### ❌ Not usable

| CPU | Why |
|---|---|
| Xeon E3 v5/v6, Core 6th/7th gen (Skylake/Kaby Lake) | SGX1 **without FLC**, which Gramine ≥ 1.9 doesn't support |
| Core 11th gen+ desktop/mobile | SGX removed/deprecated on client parts |
| Xeon Scalable 1st/2nd gen (Skylake-SP, Cascade Lake: Gold 61xx/62xx, Silver 42xx), Cooper Lake | no SGX |
| Xeon E5/E7 all versions, Xeon D-1500/D-2100 | no SGX |
| All AMD (EPYC, Ryzen) | no SGX (SEV instead) |

---

## 3. Google Cloud (GCE / GKE)

**Verdict: nothing on GCP runs Gramine-SGX.** Google exposes SGX on no machine type, bare metal included.
Intel confidential computing on GCP means **TDX** (Confidential VM / Confidential GKE Nodes on C3), and SGX
can't be used from inside a TD. AMD series offer SEV/SEV-SNP.

| Series | CPU | Silicon has SGX+FLC? | SGX exposed? | Confidential option |
|---|---|---|---|---|
| N1, N2 (Cascade Lake), C2 | Skylake/Cascade Lake | no | no | – |
| N2 (Ice Lake), M3 | Ice Lake-SP | yes | **no** | – |
| C3, H3, A3 | Sapphire Rapids | yes | **no** | TDX (C3) |
| `c3-{highcpu,standard,highmem}-192-metal` | Sapphire Rapids | yes | **no** (not documented) | none on metal |
| `x4-*-metal`, `z3-*-metal` | Sapphire Rapids | yes | **no** | none on metal |
| N4, C4 | Emerald/Granite Rapids | yes | **no** | – |
| `c4-{standard,highmem}-288-metal` | Granite Rapids | yes | **no** (not documented) | none on metal |
| N2D, C2D, C3D, C4D (+ `c4d-*-metal`), T2D | AMD EPYC | no | – | SEV/SEV-SNP |
| C4A, T2A, A4X | Arm | no | – | – |

Bare metal is the only place where SGX *could* leak through, if Google left it on in the BIOS.
Run `sudo ./check.sh --no-run` on a `c3-*-metal` and a `c4-*-metal` to settle it. We expect
`bios.sgx-enabled`/`kernel.flag.sgx` to FAIL. GKE pods on standard or Confidential nodes: expect `cpuid.sgx` FAIL.

Gramine with TDX exists only as the experimental [gramine-tdx](https://github.com/gramineproject/gramine-tdx) fork, which is not production-ready and not covered here.

Outside GCP/OVH, the public clouds with real SGX are Azure DCsv2/DCsv3/DCdsv3 and Alibaba g7t/c7t/r7t.

---

## 4. OVHcloud bare metal

Source: OVH public order catalog API (`/1.0/order/catalog/public/{baremetalServers,eco}?ovhSubsidiary=FR`), pulled 2026-09-29.
Availability differs by region/subsidiary.

How to enable SGX: Control Panel → server → *Intel SGX*, or the API:
`GET /dedicated/server/{serviceName}/biosSettings/sgx`, then
`POST /dedicated/server/{serviceName}/biosSettings/sgx/configure` (reboots the server; follow the task with
`GET /dedicated/server/{serviceName}/task/{taskId}`). A 404 on the GET means OVH offers no SGX toggle for that server.
Before ordering many, check that the toggle exists on one, especially on the Eco (Kimsufi/So you Start/Rise) lines
where you have no BIOS access.

### ✅ CPU capable (SGX + FLC)

| Range | Plan | CPU |
|---|---|---|
| Rise | RISE-1 | Xeon E-2386G |
| Rise | RISE-2 | Xeon E-2388G |
| Rise | RISE-6 | Xeon Gold 6312U (Ice Lake) |
| So you Start | SYS-1 | Xeon E-2136 |
| So you Start | SYS-3 | Xeon E-2288G |
| Kimsufi | KS-5-A | Xeon E-2274G |
| Scale | Scale-i1 | Xeon Gold 6426Y (SPR) / Xeon 6517P |
| Scale | Scale-i2 | Xeon Gold 6442Y (SPR) / Xeon 6527P |
| Scale | Scale-i3 | Xeon Gold 6438M (SPR) / Xeon 6737P |
| High Grade | HGR-HCI-i1 | 2× Xeon Gold 5515+ (EMR) / 2× Xeon 6517P |
| High Grade | HGR-HCI-i2 | 2× Xeon Gold 6526Y (EMR) / 2× Xeon 6527P |
| High Grade | HGR-HCI-i3 | 2× Xeon Gold 6542Y (EMR) |
| High Grade | HGR-HCI-i4 | 2× Xeon Gold 6554S (EMR) |
| High Grade | HGR-SDS-1 | 2× Xeon Gold 5515+ (EMR) |
| High Grade | HGR-SDS-2 | 2× Xeon Gold 6542Y (EMR) |
| High Grade | HGR-STOR-1 | Xeon Gold 6554S (EMR) |

Dual-socket HGR boxes need multi-package platform registration before DCAP quotes work (requirement 15).
The E-2xxx boxes have a small EPC (128/512 MB), which is fine for small enclaves.

### ❌ Not usable

| Plan | CPU | Why |
|---|---|---|
| KS-3, KS-4, KS-5, KS-A, KS-GAME | E3-1245 v5, E3-1230 v6, E3-1270 v6, i7-6700K, i7-7700K | no FLC |
| KS-1, KS-2, KS-STOR, KS-1-B, SYS-2 | Xeon D-15xx / D-21xx | no SGX |
| KS-5-B, KS-6-B, KS-B, KS-C | Xeon E5 | no SGX |
| SYS-3-P, SYS-5, SYS-6 | Silver 4214R, Gold 6132 | no SGX |
| HGR-SAP-1/2/3 | Gold 6226R/6242R/6248R (Cascade Lake) | no SGX |
| Advance (current), Scale-a*, HGR-*-a* | AMD EPYC | no SGX |

---

## 5. Reading results

| First FAIL | Meaning / fix |
|---|---|
| `cpuid.sgx` with `hypervisor` INFO | VM/pod: the provider doesn't expose SGX, so move to bare metal |
| `cpuid.sgx` on bare metal | CPU without SGX, or SGX set to Disabled in BIOS |
| `cpuid.flc` / `bios.flc-enabled` | old CPU, or BIOS "Launch Control Policy" locked, so set it to Unlocked |
| `bios.sgx-enabled`, `epc` | BIOS: SGX Enabled, PRMRR size > 0, TME on (Xeon SP) |
| `kernel.*` | kernel < 5.11, or `dmesg \| grep sgx` shows why the driver didn't load |
| `dev.*` | udev/group permissions, or the container didn't get the devices |
| `run.gramine-sgx` | see the enclave log printed above it; often EPC too small or `noexec` |
| `attest.dcap-quote` | aesmd not running, QPL/PCCS misconfigured, platform not registered |
