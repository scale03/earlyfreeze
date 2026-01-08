# earlyfreeze
![CI](https://github.com/scale03/earlyfreeze/actions/workflows/build.yml/badge.svg)
![License](https://img.shields.io/github/license/scale03/earlyfreeze)
> **The Early OOM freezer Daemon**


Heavily inspired by [`earlyoom`](https://github.com/rfjakob/earlyoom), **earlyfreeze** opts for a non-destructive strategy. Instead of sending `SIGKILL` to free up memory, it leverages the **Cgroup v2 freezer** to pause execution.

This allows the kernel to swap out pages calmly, preventing system lockups without sacrificing your running applications or data.

## Quick Start

```bash
git clone https://github.com/scale03/earlyfreeze.git
make
sudo ./earlyfreeze --target /sys/fs/cgroup/user.slice
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
