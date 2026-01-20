# earlyfreeze
![CI](https://github.com/scale03/earlyfreeze/actions/workflows/build.yml/badge.svg)
![License](https://img.shields.io/github/license/scale03/earlyfreeze)
> **Early OOM Cgroups Freezer Daemon**

## Overview
The Linux OOM Killer is a mechanism of last resort. When memory runs out, the kernel sacrifices a process to save the system. It is effective, but it is destructive. It often kills the wrong process, and for background workers processing long-running jobs, "death" means lost state and wasted compute.

**earlyfreeze** proposes a different contract: Suspension.

By monitoring Linux **PSI (Pressure Stall Information)**, earlyfreeze detects memory contention before the kernel panics. When pressure spikes, it uses the **Cgroups V2 freezer controller** to atomically pause non-critical workloads. Once the pressure subsides, they resume exactly where they left off.

It is a very simple daemon designed to keep your workers alive, your system responsive, and the OOM Killer asleep.
## Quick Start

```bash
git clone https://github.com/scale03/earlyfreeze.git
make
```


## Usage:
```bash
./earlyfreeze --target <path> [options]


Options:
  -t, --target <path>     Target cgroup (e.g., /sys/fs/cgroup/user.slice)
  -h, --threshold <val>   Freeze PSI % (default: 20.0)
  -r, --recover <val>     Thaw PSI % (default: 5.0)
  -i, --interval <ms>     Poll interval (default: 100)
  -d, --dry-run           Log only, do not freeze
```
### Requirements
* Linux Kernel 5.2+
* Cgroup v2 mounted
* PSI enabled

**Note for non-systemd users (Alpine, Void, Artix):**
`earlyfreeze` is init-agnostic. It works on any cgroup v2 directory.
Just point it to your custom group:
`./earlyfreeze --target /sys/fs/cgroup/my_custom_group`
