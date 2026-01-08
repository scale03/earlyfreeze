# earlyfreeze
![CI](https://github.com/scale03/earlyfreeze/actions/workflows/build.yml/badge.svg)
![License](https://img.shields.io/github/license/scale03/earlyfreeze)
> **The Early OOM freezer Daemon**

**earlyfreeze** is a minimal, zero-dependency C99 daemon that prevents Linux OOM hangs by temporarily **suspending** processes via **cgroup v2 freezer** instead of killing them.

Unlike `earlyoom` or `systemd-oomd` (which use `SIGKILL`), earlyfreeze buys time for the kernel to swap out pages without destroying data or user sessions.

## Quick Start

```bash
git clone [https://github.com/scale03/earlyfreeze.git](https://github.com/scale03/earlyfreeze.git)
make
sudo ./earlyfreeze --target /sys/fs/cgroup/user.
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
