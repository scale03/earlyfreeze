/* earlyfreeze.c - The Non-Destructive OOM Guard */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <getopt.h>
#include <time.h>

/* Default Config */
#define PSI_PATH "/proc/pressure/memory"
#define DFL_THRESHOLD 20.0f
#define DFL_RECOVER 5.0f
#define DFL_INTERVAL 100

/* Global Variables Signal Handling */
static char *target_path = NULL;
static int dry_run = 0;
static volatile sig_atomic_t keep_running = 1;

/* Write cgroup function */
void set_freeze(int freeze) {
    char path[512];
    snprintf(path, sizeof(path), "%s/cgroup.freeze", target_path);

    if (dry_run) {
        printf("[DRY-RUN] %s -> %s\n", freeze ? "FREEZING" : "THAWING", path);
        return;
    }

    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        /* Non spammiamo errori se il cgroup non esiste (magari il processo è finito) */
        return;
    }

    char val = freeze ? '1' : '0';
    if (write(fd, &val, 1) < 0) {
        perror("Error writing to cgroup.freeze");
    }
    close(fd);
}

/* Read and Parse /proc/pressure/memory */
float read_psi() {
    char buf[256];
    int fd = open(PSI_PATH, O_RDONLY);
    if (fd < 0) return -1.0f;

    int len = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    
    if (len <= 0) return -1.0f;
    buf[len] = '\0';

    /* Search "avg10=" */
    char *p = strstr(buf, "avg10=");
    if (!p) return -1.0f;

    return strtof(p + 6, NULL);
}

/* Signal handling (SIGINT, SIGTERM) */
void handle_signal(int sig) {
    (void)sig;
    keep_running = 0;
}

void cleanup() {
    if (target_path) {
        printf("\n[SHUTDOWN] Thawing target before exit...\n");
        set_freeze(0); // Freeze before kill!
    }
}

void print_usage(const char *prog) {
    printf("Usage: %s --target <path> [options]\n", prog);
    printf("Options:\n");
    printf("  -t, --target <path>     Path to cgroup (REQUIRED)\n");
    printf("  -h, --threshold <val>   PSI freeze threshold (default: %.1f)\n", DFL_THRESHOLD);
    printf("  -r, --recover <val>     PSI thaw threshold (default: %.1f)\n", DFL_RECOVER);
    printf("  -i, --interval <ms>     Check interval ms (default: %d)\n", DFL_INTERVAL);
    printf("  -d, --dry-run           Don't actually freeze\n");
}

int main(int argc, char *argv[]) {
    float threshold = DFL_THRESHOLD;
    float recover = DFL_RECOVER;
    int interval = DFL_INTERVAL;

    static struct option long_options[] = {
        {"target",    required_argument, 0, 't'},
        {"threshold", required_argument, 0, 'h'},
        {"recover",   required_argument, 0, 'r'},
        {"interval",  required_argument, 0, 'i'},
        {"dry-run",   no_argument,       0, 'd'},
        {"help",      no_argument,       0, '?'},
        {0, 0, 0, 0}
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "t:h:r:i:d?", long_options, NULL)) != -1) {
        switch (opt) {
            case 't': target_path = optarg; break;
            case 'h': threshold = strtof(optarg, NULL); break;
            case 'r': recover = strtof(optarg, NULL); break;
            case 'i': interval = atoi(optarg); break;
            case 'd': dry_run = 1; break;
            case '?': print_usage(argv[0]); return 0;
            default:  return 1;
        }
    }

    if (!target_path) {
        fprintf(stderr, "Error: --target is required.\n");
        print_usage(argv[0]);
        return 1;
    }

    /* Setup Signal Handling */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    printf("EARLYFREEZE (C Version)\n");
    printf("Target: %s\n", target_path);
    printf("Config: Freeze >%.2f%% | Thaw <%.2f%%\n", threshold, recover);

    int is_frozen = 0;
    struct timespec ts;
    ts.tv_sec = interval / 1000;
    ts.tv_nsec = (interval % 1000) * 1000000;

    /* Main Loop */
    while (keep_running) {
        float psi = read_psi();
        
        if (psi >= 0) {
            if (!is_frozen && psi > threshold) {
                printf("[ALERT] Pressure %.2f%%. FREEZING.\n", psi);
                set_freeze(1);
                is_frozen = 1;
            } else if (is_frozen && psi < recover) {
                printf("[INFO] Pressure %.2f%%. THAWING.\n", psi);
                set_freeze(0);
                is_frozen = 0;
            }
        } else {
            fprintf(stderr, "[WARN] Failed to read PSI\n");
        }

        nanosleep(&ts, NULL);
    }

    cleanup();
    return 0;
}
