#!/bin/bash
# test_logic.sh - Testing logic by mocking the kernel
set -e

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

echo "--- Setting up Test Environment ---"

# 1. We will use a temporary Playground 
TEST_DIR=$(mktemp -d)
FAKE_PSI="$TEST_DIR/fake_memory_psi"
FAKE_CGROUP="$TEST_DIR/fake_cgroup"
FAKE_FREEZE="$FAKE_CGROUP/cgroup.freeze"
TARGET_BIN="$TEST_DIR/earlyfreeze_test"

mkdir -p "$FAKE_CGROUP"

# 2. Init Test Files
# PSI format: some avg10=0.00 avg60=0.00 ...
echo "some avg10=0.00 avg60=0.00 avg300=0.00 total=0" > "$FAKE_PSI"
# Freeze file: Default 0 (unfreezed)
echo "0" > "$FAKE_FREEZE"

echo "Mocks created at $TEST_DIR"

# 3. HACK: We modify the copy of the source code to use the temporary file
# change /proc/pressure/memory with our test file.
echo "--- Compiling Test Build ---"
sed "s|/proc/pressure/memory|$FAKE_PSI|g" earlyfreeze.c > "$TEST_DIR/earlyfreeze_test.c"

# Compiling test version
gcc -Wall -O2 -std=c99 -o "$TARGET_BIN" "$TEST_DIR/earlyfreeze_test.c"

# 4. Daemon start
# Deafault freeze threshold: 20.0, Recover Threshold: 5.0, Check every 100ms
"$TARGET_BIN" --target "$FAKE_CGROUP" --threshold 20.0 --recover 5.0 --interval 50 > "$TEST_DIR/daemon.log" 2>&1 &
DAEMON_PID=$!

echo "Daemon started (PID $DAEMON_PID). Watching logic..."

# Funzione di pulizia (per non lasciare processi appesi)
cleanup() {
    echo ""
    echo "--- Cleaning up ---"
    kill $DAEMON_PID 2>/dev/null || true
    rm -rf "$TEST_DIR"
}
trap cleanup EXIT

# --- start TEST---

# Step A: Current state
sleep 0.2
CURRENT_STATE=$(cat "$FAKE_FREEZE")
if [ "$CURRENT_STATE" == "0" ]; then
    echo -e "${GREEN}[PASS] Initial state is Thawed (0)${NC}"
else
    echo -e "${RED}[FAIL] Initial state should be 0, got $CURRENT_STATE${NC}"
    exit 1
fi

# Step B: Spike test
echo ">>> Simulating Pressure Spike (30.00%)"
echo "some avg10=30.00 avg60=0.00 avg300=0.00 total=0" > "$FAKE_PSI"
sleep 0.2 # We give time for the daemon to read

CURRENT_STATE=$(cat "$FAKE_FREEZE")
if [ "$CURRENT_STATE" == "1" ]; then
    echo -e "${GREEN}[PASS] Daemon detected pressure and Froze the cgroup (1)${NC}"
else
    echo -e "${RED}[FAIL] Expected freeze (1), got $CURRENT_STATE${NC}"
    cat "$TEST_DIR/daemon.log"
    exit 1
fi

# Step C: Hysteresis
echo ">>> Simulating Pressure Drop to 10.00% (Should stay Frozen)"
echo "some avg10=10.00 avg60=0.00 avg300=0.00 total=0" > "$FAKE_PSI"
sleep 0.2

CURRENT_STATE=$(cat "$FAKE_FREEZE")
if [ "$CURRENT_STATE" == "1" ]; then
    echo -e "${GREEN}[PASS] Hysteresis working. Still Frozen at 10%${NC}"
else
    echo -e "${RED}[FAIL] Hysteresis failed. Should be 1, got $CURRENT_STATE${NC}"
    exit 1
fi

# Step D: Complete recovery
echo ">>> Simulating Pressure Drop to 1.00% (Should Thaw)"
echo "some avg10=1.00 avg60=0.00 avg300=0.00 total=0" > "$FAKE_PSI"
sleep 0.2

CURRENT_STATE=$(cat "$FAKE_FREEZE")
if [ "$CURRENT_STATE" == "0" ]; then
    echo -e "${GREEN}[PASS] Daemon detected recovery and Thawed (0)${NC}"
else
    echo -e "${RED}[FAIL] Expected thaw (0), got $CURRENT_STATE${NC}"
    exit 1
fi

echo ""
echo -e "${GREEN}ALL TESTS PASSED. Logic is solid.${NC}"
