# earlyfreeze
![CI](https://github.com/scale03/earlyfreeze/actions/workflows/build.yml/badge.svg)
![License](https://img.shields.io/github/license/scale03/earlyfreeze)
> **The Early OOM freezer Daemon**

## Overview
Linux has a mechanism to handle low memory (the OOM Killer). It kills processes. Sometimes it kills the wrong process. 
If I have a generic worker running in the background, I don't want it dead. I just want it to freeze while the memory pressure is too high.

**earlyfreeze** is a very simple daemon that brings the concept of "Pause" to your background services using **Cgroups V2 freeze**.

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
