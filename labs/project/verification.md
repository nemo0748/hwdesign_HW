# VAC IP Verification Evidence

---

## 1. Simulation File Reference

All files listed below are located in the `sim` folder of the project.

| File | Path | Purpose |
|------|------|---------|
| `xsim.log` | `sim/logs/xsim.log` | Full simulation transcript: tool version, run command, testbench `$display` output, finish timestamp, and exit status. Primary evidence of simulation completion. |
| `xsim.jou` | `sim/xsim.jou` | Journal of Tcl commands issued to XSim (`source`, `run -all`). Provides an auditable record of exactly how the simulation session was driven. |
| `xsimkernel.log` | `sim/xsimkernel.log` | Low-level kernel log from the XSim simulation engine. Records design load status, waveform database target, and CPU/memory usage for the simulation run. |
| `xsimSettings.ini` | `sim/xsimSettings.ini` | XSim GUI configuration: display radix (`hex`), time unit (`ns`), trace limit, and signal scope/object visibility filters. Does not affect simulation correctness; governs waveform viewer behavior only. |
| `results.csv` | `sim/results.csv` | Per-vector scoreboard output written by the testbench. Contains DUT-produced values (`got_sat`, `got_ovf`), golden-model expected values (`exp_sat`, `exp_ovf`), and a `PASS`/`FAIL` verdict for each of the 30 test vectors. |

The simulation was invoked on **Tue May 12 18:57:11 2026** on `ecs02.poly.edu` (Vivado XSim 2023.2, 64-bit, Linux):

```
xsim -log logs/xsim.log -mode tcl -source xsim.dir/tb_vac_csv_sim/xsim_script.tcl
```

A waveform database (`tb_vac_csv_sim.wdb`) was also generated, confirming the simulation ran in GUI-compatible mode with full signal tracing enabled up to the `TRACE_LIMIT` of 2,147,483,647 transitions.

---

## 2. Evidence of Successful Simulation

### 2.1 Simulation Completion (`xsim.log`, `xsimkernel.log`)

```
Reading vectors from: ../test_vectors.csv
Done! Processed 30 vectors. Results saved to results.csv
$finish called at time : 7895 ns
INFO: [Common 17-206] Exiting xsim at Tue May 12 18:57:14 2026...
```

```
Design successfully loaded
Simulation completed
```

| Check | Result |
|-------|--------|
| Design loaded | ✅ `Design successfully loaded` (`xsimkernel.log`) |
| Simulation completed | ✅ `$finish` reached at 7895 ns — no hang or fatal error |
| All vectors consumed | ✅ 30 of 30 vectors processed |
| XSim exit status | ✅ Clean `INFO` exit; no `ERROR` or `FATAL` messages |
| Non-fatal warning | ⚠️ `test_vectors.csv` not found at CWD; resolved via fallback to `../test_vectors.csv` — expected behavior per testbench path-resolution logic |

### 2.2 Functional Correctness (`results.csv`)

All 30 test vectors produced a `PASS` verdict. The DUT `got_sat` and `got_ovf` outputs matched the golden model `exp_sat` / `exp_ovf` exactly in every case.

**Pass/Fail Summary**

| Result | Count |
|--------|-------|
| PASS | **30** |
| FAIL | 0 |
| Total | 30 |

**Results by Test Category**

| Category | Vectors | All Pass? | Notes |
|----------|---------|-----------|-------|
| Basic cubic | 6 | ✅ | Small `x`, all four terms active; results in-range |
| Linear / isolated terms | 6 | ✅ | `A=B=0`; verifies `C·x + D` path in isolation |
| Saturation | 6 | ✅ | `sat ≠ ovf` for all 6; correct clamping and wrap confirmed |
| Mixed signs | 6 | ✅ | Negative leading coefficients; sign propagation verified |
| Zero / constant / boundary `x` | 6 | ✅ | All-zero poly, constant-only, and `x = INT32_MAX` cases |

**Selected Saturation Vectors (illustrating `sat ≠ ovf` divergence)**

| A | B | C | D | x | `got_sat` | `got_ovf` |
|---|---|---|---|---|-----------|-----------|
| 2 | 0 | 0 | 0 | 1100 | 2147483647 | −1632967296 |
| 1 | 0 | 0 | 0 | 1300 | 2147483647 | −2097967296 |
| 3 | 0 | 0 | 0 | 1100 | 2147483647 | −301967296 |
| 1 | 0 | 0 | 0 | 1400 | 2147483647 | −1550967296 |
| 1 | 0 | 0 | 0 | −1300 | −2147483648 | 2097967296 |
| −2 | 0 | 0 | 0 | 1200 | −2147483648 | 838967296 |

The saturation results confirm that the full-precision accumulation fix in `weighted_accumulator` (carrying `x²` and `x³` at 64-bit width) is functioning correctly — the correct wrap-around values for `got_ovf` match the golden model's arbitrary-precision Python computation exactly.

---

## 3. Simulation Performance Metrics

| Metric | Value | Source |
|--------|-------|--------|
| Total simulation time | 7895 ns | `$finish` in `xsim.log` |
| Clock period | 10 ns (100 MHz) | Testbench `CLK_PERIOD` parameter |
| Total clock cycles simulated | ~790 cycles | 7895 ns ÷ 10 ns |
| Vectors processed | 30 | `xsim.log` |
| Pipeline latency (by design) | 4 cycles / 40 ns | `PIPE_LAT` testbench parameter |
| Approx. cycles per vector | ~26 cycles | 4 AXI writes + 10-cycle flush + 1 input + 4 pipeline + handshake overhead |
| Simulation CPU time | 20 ms | `xsimkernel.log` |
| Design load CPU time | 10 ms | `xsimkernel.log` |
| Peak simulation memory | 182,968 KB (~179 MB) | `xsimkernel.log` |
| Steady-state simulation memory | 138,508 KB (~135 MB) | `xsimkernel.log` |




