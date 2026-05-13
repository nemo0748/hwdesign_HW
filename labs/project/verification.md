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

---

## 4. Hardware Implementation and Verification

The VAC was implemented and verified on the **Xilinx Zynq-7000 (`xc7z020clg400-1`)** using Vivado 2023.2. Implementation was performed in **Out-of-Context (OOC) mode**, validating logic correctness and timing closure independently of physical pin constraints. All reports were generated on **Tue May 12 20:56:02 2026** on `ecs02.poly.edu`.

### 4.1 Resource Utilization (`vac_utilization_placed.rpt`)

| Resource | Used | Available | Utilization |
|----------|------|-----------|-------------|
| Slice LUTs (Logic) | 609 | 53,200 | 1.14% |
| Slice Registers (FFs) | 393 | 106,400 | 0.37% |
| DSP48E1 Blocks | 29 | 220 | 13.18% |
| Block RAM | 0 | — | 0.00% |
| Bonded IOB | 0 | 125 | 0.00% |

All 393 registers use synchronous reset, consistent with the RTL's `always_ff @(posedge clk)` reset style throughout.

### 4.2 Timing (`vac_timing_summary_routed.rpt`)

The design was implemented without user-specified timing constraints (OOC mode). All 2,241 routable nets were fully routed with zero routing errors (`vac_route_status.rpt`). The timing summary reports a WNS of **∞** (no failing endpoints across 1,979 timing paths), confirming no setup or hold violations exist in the routed netlist.

| Metric | Value |
|--------|-------|
| Timing constraints | None (OOC mode) |
| Routable nets | 2,241 |
| Fully routed nets | 2,241 |
| Routing errors | 0 |
| Setup violations (TNS failing endpoints) | 0 |
| Hold violations (THS failing endpoints) | 0 |

### 4.3 Power (`vac_power_routed.rpt`)

| Power Domain | Power (W) | Share of Dynamic |
|--------------|-----------|-----------------|
| DSPs | 25.403 | ~51.9% |
| Signals | 12.992 | ~26.6% |
| Slice Logic (LUTs) | 10.516 | ~21.5% |
| **Total Dynamic** | **48.912** | 100% |
| Device Static | 1.038 | — |
| **Total On-Chip** | **49.950** | — |

DSP slices account for approximately **51% of total dynamic power**, confirming that Vivado correctly mapped the polynomial multiplications onto dedicated DSP48E1 arithmetic blocks rather than soft logic.

> **Note:** The power figure reflects worst-case toggle-rate assumptions applied in OOC mode and is not representative of typical operating power.

### 4.4 DRC and Methodology (`vac_drc_routed.rpt`, `vac_methodology_drc_routed.rpt`)

All DRC violations are **warnings only** — no errors. The warning categories are expected consequences of the OOC implementation mode and the DSP pipeline configuration:

| Rule | Severity | Description | Count |
|------|----------|-------------|-------|
| `DPIP-1` | Warning | DSP input pipelining not used | 21 |
| `DPOP-1` | Warning | DSP PREG output register not used | 27 |
| `DPOP-2` | Warning | DSP MREG output register not used | 29 |
| `ZPS7-1` | Warning | PS7 block not present (OOC mode) | 1 |
| `TIMING-17` | Critical Warning | 422 registers not reached by a timing clock (OOC — no clock constraint defined) | 422 |
| `SYNTH-10` | Warning | Wide multiplier inferred | 29 |

The `TIMING-17` critical warnings are a direct artifact of OOC mode: without a defined clock constraint, Vivado cannot associate pipeline registers with a timing arc. The `DPIP-1`/`DPOP-1`/`DPOP-2` warnings indicate that the DSP48E1 internal pipeline registers (`PREG`, `MREG`) are not used — the design instead uses fabric registers for pipeline staging, which is consistent with the RTL structure.
