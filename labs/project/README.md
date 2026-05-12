# Variable Arithmetic Core (VAC) — Design and Verification Document

**Document Type:** Engineering Design Review / Coursework Submission  
**Source Files:** `vac.sv`, `tb_vac_csv.sv`  
**Timescale:** 1 ns / 1 ps  
**Revision:** 1.0

---

## Table of Contents

1. [System Overview](#1-system-overview)
2. [Module Definitions](#2-module-definitions)
   - 2.1 [Top-Level: `vac`](#21-top-level-vac)
   - 2.2 [`axi_lite_regbank`](#22-axi_lite_regbank)
   - 2.3 [`power_engine`](#23-power_engine)
   - 2.4 [`weighted_accumulator`](#24-weighted_accumulator)
   - 2.5 [`dual_path_formatter`](#25-dual_path_formatter)
3. [Protocol and Message Formats](#3-protocol-and-message-formats)
4. [Pipeline Timing and Sequencing](#4-pipeline-timing-and-sequencing)
5. [Internal Data Flow and Architecture](#5-internal-data-flow-and-architecture)
6. [Testbench and Verification Strategy](#6-testbench-and-verification-strategy)
7. [Golden Model](#7-golden-model)
8. [Appendix: Register Map](#8-appendix-register-map)

---

## 1. System Overview

The **Variable Arithmetic Core (VAC)** is a pipelined, parameterizable hardware accelerator that evaluates a third-degree polynomial of the form:

```
y = A·x³ + B·x² + C·x + D
```

where `A`, `B`, `C`, and `D` are 32-bit signed integer coefficients, and `x` is a 32-bit signed integer input sample. The core is designed for high-throughput, single-cycle-per-sample data processing with a fixed end-to-end pipeline latency of **4 clock cycles**.

Coefficients are configured at runtime via an **AXI4-Lite slave interface**, making the VAC reconfigurable without recompilation. Input samples are ingested through an **AXI4-Stream slave interface**, and results are produced on an **AXI4-Stream master interface** as well as via dedicated direct output ports. Two output representations are provided simultaneously:

- **Saturated output (`result_sat`):** Clamps to `[INT32_MIN, INT32_MAX]` on overflow.
- **Overflow (wrap) output (`result_ovf`):** Returns the low 32 bits of the full-precision sum, enabling diagnostic inspection of overflow behavior.

The design is composed of four pipeline sub-modules wired in a strict linear chain, with a coefficient pipeline delay register bank inserted at the top level to synchronize coefficient values with delayed data arriving from the power engine.

---

## 2. Module Definitions

### 2.1 Top-Level: `vac`

#### Functional Overview

`vac` is the integration wrapper for the full polynomial evaluation pipeline. It instantiates all four sub-modules, manages the AXI4-Lite and AXI4-Stream interfaces, and implements a **2-stage coefficient delay chain** to compensate for the latency introduced by `power_engine`. This ensures that every input sample `x` meets its associated coefficient set `{A, B, C, D}` at exactly the right pipeline stage in `weighted_accumulator`.

#### Parameters

| Parameter | Type  | Default | Description                              |
|-----------|-------|---------|------------------------------------------|
| `DATA_W`  | `int` | `32`    | Data and coefficient bit width (bits)    |
| `ADDR_W`  | `int` | `5`     | AXI4-Lite address bus width (bits)       |

#### Interface Specification

**Clock and Reset**

| Signal | Direction | Width | Description                                      |
|--------|-----------|-------|--------------------------------------------------|
| `clk`  | `input`   | 1     | System clock. Rising-edge triggered throughout. |
| `rst`  | `input`   | 1     | Synchronous active-high reset.                  |

**AXI4-Lite Write Channel (passed through to `axi_lite_regbank`)**

| Signal            | Direction | Width      | Description                          |
|-------------------|-----------|------------|--------------------------------------|
| `s_axil_awaddr`   | `input`   | `ADDR_W`   | Write address                        |
| `s_axil_awvalid`  | `input`   | 1          | Address write valid                  |
| `s_axil_awready`  | `output`  | 1          | Address write ready (from slave)     |
| `s_axil_wdata`    | `input`   | `DATA_W`   | Write data                           |
| `s_axil_wvalid`   | `input`   | 1          | Write data valid                     |
| `s_axil_wready`   | `output`  | 1          | Write data ready (from slave)        |
| `s_axil_bresp`    | `output`  | 2          | Write response (always `2'b00` OKAY) |
| `s_axil_bvalid`   | `output`  | 1          | Write response valid                 |
| `s_axil_bready`   | `input`   | 1          | Write response ready (from master)   |

**AXI4-Lite Read Channel (passed through to `axi_lite_regbank`)**

| Signal           | Direction | Width    | Description              |
|------------------|-----------|----------|--------------------------|
| `s_axil_araddr`  | `input`   | `ADDR_W` | Read address             |
| `s_axil_arvalid` | `input`   | 1        | Read address valid       |
| `s_axil_arready` | `output`  | 1        | Read address ready       |
| `s_axil_rdata`   | `output`  | `DATA_W` | Read data                |
| `s_axil_rresp`   | `output`  | 2        | Read response            |
| `s_axil_rvalid`  | `output`  | 1        | Read data valid          |
| `s_axil_rready`  | `input`   | 1        | Read data ready          |

**AXI4-Stream Slave (data input)**

| Signal          | Direction | Width | Description                              |
|-----------------|-----------|-------|------------------------------------------|
| `s_axis_tdata`  | `input`   | 32    | Signed 32-bit input sample `x`           |
| `s_axis_tvalid` | `input`   | 1     | Input data valid                         |
| `s_axis_tready` | `output`  | 1     | Always driven high (backpressure-free)   |

**AXI4-Stream Master (result output)**

| Signal          | Direction | Width | Description                                               |
|-----------------|-----------|-------|-----------------------------------------------------------|
| `m_axis_tdata`  | `output`  | 64    | `{result_sat[31:0], result_ovf[31:0]}` packed output      |
| `m_axis_tvalid` | `output`  | 1     | Output data valid                                         |
| `m_axis_tready` | `input`   | 1     | Downstream backpressure signal                            |

**Direct Result Outputs**

| Signal       | Direction | Width     | Description                               |
|--------------|-----------|-----------|-------------------------------------------|
| `result_sat` | `output`  | 32 signed | Saturated polynomial result               |
| `result_ovf` | `output`  | 32 signed | Overflowed (wrapped) polynomial result    |

#### Internal Coefficient Pipeline

`vac` instantiates a 2-stage shift register for each coefficient `{A, B, C, D}` to delay them by exactly 2 clock cycles. This is required because `power_engine` introduces 2 pipeline stages before producing `x_out` and `x2_out`. Without this alignment, the accumulator would receive unrelated coefficient values for a given sample.

```
cX → [Reg q1] → [Reg q2] → weighted_accumulator
                             ↑
x_in → [power_engine stg0] → [power_engine stg1] → x_out / x2_out
```

---

### 2.2 `axi_lite_regbank`

#### Functional Overview

`axi_lite_regbank` implements an AXI4-Lite compliant slave register file that holds the four polynomial coefficients (`A`, `B`, `C`, `D`). Coefficients are written by a host master (e.g., an embedded processor or test controller) and are exposed as combinational outputs to the downstream pipeline.

**Design Note (Bug Fix):** The module comment documents a critical correction from an earlier design. The original implementation used a two-phase handshake where `AWVALID` and `WVALID` had to be observed in separate clock cycles. This caused off-by-one write errors when both channels were presented simultaneously (as is standard AXI4-Lite behavior). The corrected implementation accepts and commits the write atomically in the **same cycle** when `AWVALID`, `AWREADY`, `WVALID`, and `WREADY` are all asserted together — which is fully compliant with the AXI4-Lite specification.

#### Parameters

| Parameter | Type  | Default | Description             |
|-----------|-------|---------|-------------------------|
| `DATA_W`  | `int` | `32`    | Register data width     |
| `ADDR_W`  | `int` | `5`     | Address bus width       |

#### Interface Specification

The interface is a strict subset of the AXI4-Lite protocol — write and read channels only (no burst, strobe, or protection signals). See the top-level table for full signal descriptions; all are passed through verbatim.

**Coefficient Outputs**

| Signal    | Direction | Width     | Description                       |
|-----------|-----------|-----------|-----------------------------------|
| `coeff_A` | `output`  | 32 signed | Registered value of coefficient A |
| `coeff_B` | `output`  | 32 signed | Registered value of coefficient B |
| `coeff_C` | `output`  | 32 signed | Registered value of coefficient C |
| `coeff_D` | `output`  | 32 signed | Registered value of coefficient D |

#### Write Protocol

The write handshake uses a combined AW+W atomic acceptance:

1. Both `AWREADY` and `WREADY` are driven **combinationally** as `~s_axil_bvalid`. This means they are always high when no response is pending.
2. A `write_fire` pulse occurs when `AWVALID & AWREADY & WVALID & WREADY` are simultaneously true.
3. On `write_fire`, the addressed register is updated and `BVALID` is raised with `BRESP = 2'b00` (OKAY).
4. `BVALID` is cleared one cycle after the master asserts `BREADY`.

This prevents simultaneous queuing of multiple writes before a response is consumed (a deliberate simplification acceptable for low-frequency coefficient configuration).

#### Write Address Decode

Address decoding uses bits `[4:2]` of `s_axil_awaddr`, giving word-aligned access:

| `awaddr[4:2]` | Register | Byte Address |
|---------------|----------|--------------|
| `3'd0`        | `reg_A`  | `0x00`       |
| `3'd1`        | `reg_B`  | `0x04`       |
| `3'd2`        | `reg_C`  | `0x08`       |
| `3'd3`        | `reg_D`  | `0x0C`       |
| other         | (no-op)  | —            |

#### Read Protocol

1. `ARREADY` begins high (after reset).
2. On `ARVALID & ARREADY`, the module latches the address, deasserts `ARREADY`, and presents `RDATA` with `RVALID` on the next cycle.
3. On `RVALID & RREADY`, `RVALID` is cleared and `ARREADY` is re-asserted.

One additional readable address exists at `araddr[4:2] == 3'd4` (byte offset `0x10`), which returns the constant value `32'h1`. This serves as a **status/identity register** (e.g., confirming the core is present and responding).

#### Reset Behavior

All four coefficient registers initialize to `32'sd0`. `BVALID` and `RVALID` are deasserted, and `ARREADY` is initialized to `1'b1`.

---

### 2.3 `power_engine`

#### Functional Overview

`power_engine` is the first arithmetic stage of the pipeline. It accepts a signed 32-bit sample `x` via AXI4-Stream and, over two pipeline stages, produces the **passthrough value `x`** and the **squared value `x²`** aligned in time at its outputs. The squared value is computed and held at full 64-bit precision to prevent truncation-induced error in the downstream cubic term.

#### Interface Specification

| Signal          | Direction | Width     | Description                                  |
|-----------------|-----------|-----------|----------------------------------------------|
| `clk`           | `input`   | 1         | System clock                                 |
| `rst`           | `input`   | 1         | Synchronous active-high reset                |
| `s_axis_tdata`  | `input`   | 32 signed | Input sample `x`                             |
| `s_axis_tvalid` | `input`   | 1         | Input valid                                  |
| `s_axis_tready` | `output`  | 1         | Always asserted (`1'b1`); never backpressures|
| `x_out`         | `output`  | 32 signed | Pipelined passthrough of `x` (2-cycle delay) |
| `x2_out`        | `output`  | 64 signed | `x²` at full precision (2-cycle delay)       |
| `valid_out`     | `output`  | 1         | Indicates `x_out` / `x2_out` are valid       |

#### Internal Pipeline Stages

**Stage 0 (registration):** `s_axis_tdata` and `s_axis_tvalid` are registered into `x_s0` and `valid_s0`. Simultaneously, `x²` is computed combinationally as a full 64-bit product using sign-extended operands:

```
x_s0_se = sign_extend_64(x_s0)
x2_next = x_s0_se × x_s0_se        // 64-bit signed product
```

**Stage 1 (output registration):** `x_s0` passes to `x_s1` (the module output `x_out`), and `x2_next` is registered into `x2_s1` (the module output `x2_out`). The valid signal propagates accordingly.

**Latency:** 2 clock cycles from `s_axis_tdata` to `x_out` / `x2_out`.

**Important:** `s_axis_tready` is hardwired to `1'b1`. The core is always ready to accept a new sample every cycle. There is no stall or flow control within the pipeline.

---

### 2.4 `weighted_accumulator`

#### Functional Overview

`weighted_accumulator` forms all four polynomial terms and accumulates them into a 64-bit signed sum. It receives the aligned `x` and `x²` from `power_engine`, computes `x³`, and evaluates:

```
sum = A·x³ + B·x² + C·x + D
```

All intermediate multiplications are performed at full 64-bit precision before final accumulation.

**Design Note (Bug Fix):** The module comment documents a second critical correction. The original implementation computed `x³` as `x × x²[31:0]` — truncating `x²` to 32 bits before multiplying — and similarly truncated `x³` to 32 bits before multiplying by `A`. This double truncation silently discarded high-order bits for inputs where `x³` significantly exceeds `INT32_MAX` (e.g., `x = 1500` gives `x³ = 3.375 × 10⁹ > 2³¹ − 1`). The corrected implementation uses `x²` at full 64-bit width and carries full 64-bit precision through the cubic and quartic products, with saturation deferred to the output formatter.

#### Interface Specification

| Signal      | Direction | Width     | Description                             |
|-------------|-----------|-----------|---------------------------------------- |
| `clk`       | `input`   | 1         | System clock                            |
| `rst`       | `input`   | 1         | Synchronous active-high reset           |
| `x_in`      | `input`   | 32 signed | Passthrough `x` from `power_engine`     |
| `x2_in`     | `input`   | 64 signed | `x²` at full precision                  |
| `valid_in`  | `input`   | 1         | Validity flag from `power_engine`       |
| `coeff_A`   | `input`   | 32 signed | Delayed coefficient A                   |
| `coeff_B`   | `input`   | 32 signed | Delayed coefficient B                   |
| `coeff_C`   | `input`   | 32 signed | Delayed coefficient C                   |
| `coeff_D`   | `input`   | 32 signed | Delayed coefficient D                   |
| `sum_out`   | `output`  | 64 signed | Full-precision polynomial sum           |
| `valid_out` | `output`  | 1         | Output valid (1-cycle latency)          |

#### Internal Computation

All intermediate computations are **combinational** (in an `always_comb` block) before being registered in the sequential `always_ff`:

| Term     | Computation                                          | Notes                                  |
|----------|------------------------------------------------------|----------------------------------------|
| `x3_val` | `sign_extend_64(x_in) × x2_in`                      | Full 64-bit cubic result               |
| `term_A` | `sign_extend_64(coeff_A) × x3_val`                  | 64-bit; may exceed 64 bits in extreme cases (truncated at 64) |
| `term_B` | `sign_extend_64(coeff_B) × x2_in`                   | 64-bit                                 |
| `term_C` | `sign_extend_64(coeff_C) × sign_extend_64(x_in)`   | 64-bit                                 |
| `term_D` | `sign_extend_64(coeff_D)`                            | No multiplication needed               |

`sum_out` is the registered sum `term_A + term_B + term_C + term_D`.

**Latency:** 1 clock cycle (single register stage). Total pipeline latency to this point: 3 cycles.

---

### 2.5 `dual_path_formatter`

#### Functional Overview

`dual_path_formatter` is the final pipeline stage. It takes the 64-bit signed sum from `weighted_accumulator` and produces two 32-bit output representations simultaneously:

- **Saturated (`result_sat`):** If the 64-bit sum exceeds `INT32_MAX` (`0x7FFFFFFF`) or is less than `INT32_MIN` (`0x80000000`), it is clamped to the respective boundary. Otherwise the low 32 bits are used directly.
- **Overflow / wrap (`result_ovf`):** Always the raw low 32 bits `sum_in[31:0]`, regardless of overflow. This exposes the natural 32-bit wrap-around behavior for diagnostic use.

The 64-bit AXI-Stream output `m_axis_tdata` packs both results as `{result_sat, result_ovf}`.

#### Interface Specification

| Signal          | Direction | Width     | Description                                         |
|-----------------|-----------|-----------|-----------------------------------------------------|
| `clk`           | `input`   | 1         | System clock                                        |
| `rst`           | `input`   | 1         | Synchronous active-high reset                       |
| `sum_in`        | `input`   | 64 signed | Accumulated polynomial sum                          |
| `valid_in`      | `input`   | 1         | Input valid from accumulator                        |
| `m_axis_tdata`  | `output`  | 64        | `{result_sat[31:0], result_ovf[31:0]}`              |
| `m_axis_tvalid` | `output`  | 1         | AXI-Stream output valid                             |
| `m_axis_tready` | `input`   | 1         | Downstream ready (used to clear `m_axis_tvalid`)    |
| `result_sat`    | `output`  | 32 signed | Saturated polynomial result (direct port)           |
| `result_ovf`    | `output`  | 32 signed | Overflow/wrapped polynomial result (direct port)    |

#### Saturation Logic

Saturation is evaluated combinationally before registration:

```
if      (sum_in > 64'sh0000_0000_7FFF_FFFF)  sat_comb = 32'sh7FFF_FFFF  // clamp to MAX
else if (sum_in < 64'shFFFF_FFFF_8000_0000)  sat_comb = 32'sh8000_0000  // clamp to MIN
else                                          sat_comb = sum_in[31:0]    // in range
```

Note that `sum_in` is compared as a **signed 64-bit** value, which correctly handles negative overflow.

#### Output Packing

`m_axis_tdata` is organized as:

| Bits       | Content        |
|------------|----------------|
| `[63:32]`  | `result_sat`   |
| `[31:0]`   | `result_ovf`   |

**Latency:** 1 clock cycle. Total end-to-end pipeline latency: **4 clock cycles**.

---

## 3. Protocol and Message Formats

### 3.1 AXI4-Lite Write Transaction

A single write transaction follows this sequence:

```
Cycle N:   AWVALID=1, AWADDR=<addr>, WVALID=1, WDATA=<data>
           (AWREADY=1, WREADY=1 if not blocked by pending BVALID)
           → write_fire asserts; register updated; BVALID set
Cycle N+1: BVALID=1; master asserts BREADY=1
           → BVALID cleared; handshake complete
```

Both address and data must be presented simultaneously for single-cycle acceptance. The core does not support split-phase writes where address and data arrive on different cycles in separate transactions.

### 3.2 AXI4-Lite Read Transaction

```
Cycle N:   ARVALID=1, ARADDR=<addr>; ARREADY=1
           → ARREADY de-asserts; RVALID will assert next cycle
Cycle N+1: RVALID=1, RDATA=<register value>
           master asserts RREADY
           → RVALID cleared; ARREADY re-asserts
```

Read latency is 1 cycle (registered read).

### 3.3 AXI4-Stream Data Transaction

The stream interface is simple and non-blocking:

```
Cycle N:   TVALID=1, TDATA=x_value
           TREADY=1 always (hardwired in power_engine)
           → x is accepted and begins pipeline traversal
Cycle N+1: TVALID may deassert
```

Output `m_axis_tdata` follows the same handshake on the master side, with `m_axis_tvalid` raised for one cycle per valid result and deasserted when `m_axis_tready` is high.

### 3.4 Output Data Format

| Field         | Bits      | Type      | Description                             |
|---------------|-----------|-----------|-----------------------------------------|
| `result_sat`  | `[63:32]` | `int32`   | Polynomial result, saturated to ±2³¹−1 |
| `result_ovf`  | `[31:0]`  | `int32`   | Polynomial result, lower 32 bits (wrap) |

---

## 4. Pipeline Timing and Sequencing

### 4.1 End-to-End Latency Diagram

```
Cycle:       0       1       2       3       4
             │       │       │       │       │
s_axis_tdata─┤  x    │       │       │       │
             │       │       │       │       │
power_engine─│──────────────►│ x,x²  │       │
(2 cycles)   │       │       │       │       │
             │       │       │       │       │
coeff delay ─│───────────────┤cX_q2  │       │
(2 cycles)   │               │       │       │
             │               │       │       │
w_acc ───────│───────────────┤──────►│ sum   │
(1 cycle)    │               │       │       │
             │               │       │       │
formatter ───│───────────────│───────┤──────►│ result
(1 cycle)    │               │       │       │
```

Total pipeline depth: **4 clock cycles** from input sample to registered output.

### 4.2 Coefficient Alignment

Because `power_engine` takes 2 cycles to produce `x` and `x²`, and the coefficients are available immediately from the register bank (0 cycles), the top-level inserts a **2-stage coefficient delay pipeline** (`cX_q1 → cX_q2`) to align coefficient values with data values at the accumulator inputs.

**Critical Requirement:** Coefficients must not be modified while a sample is in flight through the pipeline unless the new coefficient values are also intended for that sample. Changing coefficients mid-flight will produce a result computed with a mix of old and new coefficient values.

### 4.3 Reset Behavior

Reset is **synchronous and active-high**. On assertion of `rst`:

- All pipeline registers clear to zero.
- All `valid` signals clear to zero (pipeline is drained).
- AXI handshake signals return to idle state (`ARREADY = 1`, `BVALID = 0`, `RVALID = 0`).
- Coefficient registers reset to `32'sd0`.
- After deassertion, the first input sample begins propagating on the following rising edge.

### 4.4 Throughput

The pipeline is **fully pipelined** — it accepts one new sample per clock cycle with no throughput penalty. At a 100 MHz clock (10 ns period), theoretical throughput is 100 million polynomial evaluations per second. Backpressure on `m_axis_tready` does not stall the input stream; however, results may be overwritten if the downstream is not ready.

---

## 5. Internal Data Flow and Architecture

### 5.1 Block Diagram

```
                 ┌──────────────────────────────────────────────┐
                 │                 vac (top)                     │
  AXI4-Lite ────►│  axi_lite_regbank                            │
  Write/Read     │  ┌─────────┐ cA,cB,cC,cD                    │
                 │  │reg_A    ├────────────►[cX_q1]─►[cX_q2]─┐ │
                 │  │reg_B    │                               │ │
                 │  │reg_C    │                               ▼ │
                 │  │reg_D    │                 weighted_accumulator
                 │  └─────────┘                    ┌──────────┤ │
                 │                                 │          │ │
  AXI4-Stream ──►│  power_engine                   │sum_out   │ │
  (x samples)   │  ┌──────────┐ x_out, x2_out     │          │ │
                 │  │stage 0  ├──────────────────►─┘          │ │
                 │  │stage 1  │                               │ │
                 │  └──────────┘                              │ │
                 │                              dual_path_fmt │ │
                 │                              ┌─────────────┘ │
                 │                              │  result_sat  ──►
                 │                              │  result_ovf  ──►
                 │                              │  m_axis_tdata──►
                 └──────────────────────────────────────────────┘
```

### 5.2 Precision Strategy

The design uses deliberately wide intermediate types to preserve numerical accuracy:

| Stage                 | Precision        | Rationale                                        |
|-----------------------|------------------|--------------------------------------------------|
| `x` (input)           | 32-bit signed    | Interface constraint                             |
| `x²` (power_engine)  | 64-bit signed    | `x_max² = (2³¹-1)² ≈ 4.6×10¹⁸` — exceeds 32 bits |
| `x³` (accumulator)   | 64-bit signed    | Prevents double-truncation bug                   |
| `A·x³` (term_A)       | 64-bit signed    | May exceed 64 bits for extreme inputs; truncated |
| `sum` (accumulator)   | 64-bit signed    | Allows saturation detection in formatter          |
| `result_sat` (output) | 32-bit signed    | Final output after range check                   |
| `result_ovf` (output) | 32-bit signed    | Raw truncation for diagnostics                   |

---

## 6. Testbench and Verification Strategy

### 6.1 Testbench Module: `tb_vac_csv`

`tb_vac_csv` is a self-checking, file-driven testbench that:
1. Reads test vectors from an external CSV file.
2. Programs the DUT coefficients via AXI4-Lite writes.
3. Drives input samples via AXI4-Stream.
4. Waits exactly `PIPE_LAT = 4` cycles for the result.
5. Compares `result_sat` and `result_ovf` against expected values from the CSV.
6. Writes a detailed results CSV for post-simulation analysis.

#### Key Parameters

| Parameter    | Value  | Description                                |
|--------------|--------|--------------------------------------------|
| `DATA_W`     | 32     | Data width (matches DUT)                   |
| `ADDR_W`     | 5      | Address width (matches DUT)                |
| `CLK_PERIOD` | 10 ns  | Clock period (100 MHz)                     |
| `PIPE_LAT`   | 4      | Pipeline latency (cycles) for result capture|
| `LBUF_BYTES` | 512    | CSV line buffer size (bytes)               |

### 6.2 Verification Goals

The testbench is designed to verify:

- **Functional correctness** of the polynomial computation across all coefficient and input combinations.
- **Saturation behavior**: correct clamping at `INT32_MIN` and `INT32_MAX`.
- **Overflow/wrap behavior**: correct low-32-bit result when overflow occurs.
- **AXI4-Lite write correctness**: coefficients are properly committed on simultaneous `AWVALID`+`WVALID` assertion.
- **Pipeline timing**: results appear exactly 4 cycles after the input is driven.
- **Coefficient isolation**: each test vector programs fresh coefficients before applying `x`, with a 10-cycle flush gap to prevent inter-test contamination.

### 6.3 Test Scenarios

| Scenario                  | Description                                                                                  |
|---------------------------|----------------------------------------------------------------------------------------------|
| **Normal operation**      | Mid-range `x` values with well-behaved coefficients — result fits in 32 bits                |
| **All-zero coefficients** | A=B=C=D=0 → expected result is 0 for any `x`                                               |
| **Unit coefficient**      | A=1, B=C=D=0 → expected result is `x³`                                                      |
| **Positive saturation**   | Large positive `x` or large positive A → `result_sat` should clamp to `0x7FFFFFFF`          |
| **Negative saturation**   | Large negative `x` or large negative A → `result_sat` should clamp to `0x80000000`          |
| **Wrap-around**           | Same overflow cases — `result_ovf` should wrap at 32-bit boundary                          |
| **Negative `x` values**   | Negative inputs test sign extension and cubic sign behavior (`x³` preserves sign)            |
| **`x = 0`**               | Any coefficient set should yield `D` (constant term only)                                   |
| **`x = ±1`**              | Result equals `±A ± B ± C + D` — tests correct sign handling for all terms                 |
| **Large `x` (e.g., 1500)**| `x³ = 3.375e9`, which overflows 32 bits — tests whether `x²` full precision fixes the bug  |
| **AXI write timeout guard**| `write_timeout` counter ensures the testbench does not hang if READY is stuck low          |

### 6.4 Test Sequence Per Vector

For each row in the input CSV, the testbench executes the following cycle-accurate sequence:

```
1. safe_write(0x00, A)   — programs reg_A
2. safe_write(0x04, B)   — programs reg_B
3. safe_write(0x08, C)   — programs reg_C
4. safe_write(0x0C, D)   — programs reg_D
5. wait 10 cycles         — flushes residual pipeline state; lets coefficients settle
6. Drive s_axis_tdata = x, s_axis_tvalid = 1 for 1 cycle
7. Deassert s_axis_tvalid
8. Wait PIPE_LAT = 4 cycles
9. Sample result_sat and result_ovf
10. Compare to expected values; write PASS/FAIL to results.csv
```

### 6.5 AXI Write Task: `safe_write`

```systemverilog
task automatic safe_write(input [ADDR_W-1:0] addr, input [DATA_W-1:0] data);
```

The task presents `AWVALID` and `WVALID` simultaneously (as required by the corrected `axi_lite_regbank`). It polls until `AWREADY && WREADY` are observed, with a 100-cycle timeout watchdog that emits an error message if the slave is unresponsive. This prevents simulation hang on handshake failures.

### 6.6 CSV Parser

The testbench includes a **custom signed-integer CSV parser** implemented in pure Verilog-2005 constructs. This was necessitated by a known simulator bug in **Vivado XSim 2023.x** where `$fscanf("%d", ...)` mishandles negative numbers and silently skips affected rows.

The parser operates on a 512-byte line buffer (`LBUF_BYTES = 512`) and:
- Uses `$fgets` to read whole lines into a packed register.
- Traverses the buffer **right-to-left** (MSB to LSB, since `$fgets` writes left-aligned in the packed reg).
- Skips whitespace, CR, and comma delimiters.
- Handles optional leading sign (`+` or `-`).
- Accumulates decimal digits into a 64-bit accumulator.
- Produces up to 7 signed 32-bit fields per line.
- Emits a `WARNING` for partially parsed lines and silently skips blank lines.

This parser is portable across Vivado XSim, ModelSim, VCS, and Icarus Verilog.

### 6.7 CSV I/O Format

**Input CSV (`test_vectors.csv`)**

```
A,B,C,D,x,exp_sat,exp_ovf
1,0,0,0,10,1000,1000
1,0,0,0,1500,2147483647,<wrapped value>
...
```

| Column    | Type    | Description                            |
|-----------|---------|----------------------------------------|
| `A`       | int32   | Cubic coefficient                      |
| `B`       | int32   | Quadratic coefficient                  |
| `C`       | int32   | Linear coefficient                     |
| `D`       | int32   | Constant term                          |
| `x`       | int32   | Input sample                           |
| `exp_sat` | int32   | Expected saturated output              |
| `exp_ovf` | int32   | Expected overflow/wrap output          |

**Output CSV (`results.csv`)**

```
A,B,C,D,x,got_sat,got_ovf,exp_sat,exp_ovf,status
```

| Column    | Description                              |
|-----------|------------------------------------------|
| `got_sat` | Actual `result_sat` from DUT             |
| `got_ovf` | Actual `result_ovf` from DUT             |
| `status`  | `PASS` or `FAIL`                         |

The output CSV allows automated post-processing (e.g., with Python or a spreadsheet) to count pass/fail rates, identify failing test cases, and plot output distributions.

### 6.8 Path Resolution

The testbench searches for `test_vectors.csv` in three locations in order:
1. Default (simulator CWD): `test_vectors.csv`
2. Parent directory: `../test_vectors.csv`
3. Explicit: `./test_vectors.csv`

An override mechanism via `+TV=<path>` is supported for all major simulators that accept `+` plusargs.

---

## 7. Golden Model

The expected values in `test_vectors.csv` are pre-computed by a Python Jupyter notebook (`golden_model.ipynb`). The notebook generates **exactly 30 test vectors per run**, structured as 6 samples drawn from each of 5 distinct test categories. Sampling is deterministic and reproducible: a `BATCH` integer seeds a per-category `random.Random` instance, so changing `BATCH` produces a different but fully reproducible set of 30 vectors without modifying any pool definitions.

### 7.1 Core `poly_golden` Function

```python
import numpy as np

INT32_MAX =  2**31 - 1
INT32_MIN = -2**31

def poly_golden(A, B, C, D, x):
    # Input range guard — all five values must be valid int32
    for name, val in [('A',A),('B',B),('C',C),('D',D),('x',x)]:
        if not (INT32_MIN <= val <= INT32_MAX):
            raise ValueError(f"{name}={val} out of int32 range")

    # Arbitrary-precision Python integer arithmetic — no intermediate truncation
    result = int(A)*int(x)**3 + int(B)*int(x)**2 + int(C)*int(x) + int(D)

    # Saturated path: clamp to [INT32_MIN, INT32_MAX]
    expected_sat = max(INT32_MIN, min(INT32_MAX, result))

    # Overflow path: two's-complement 32-bit wrap (lower 32 bits as signed int32)
    expected_ovf = int(np.int32(np.uint32(result & 0xFFFFFFFF)))

    return int(expected_sat), int(expected_ovf)
```

Intermediate arithmetic is performed using Python's arbitrary-precision integers — there is no 64-bit truncation at any intermediate step, faithfully matching the full-precision accumulation in the DUT's `weighted_accumulator`. The overflow path replicates the DUT's `sum_in[31:0]` extraction by masking to 32 bits and reinterpreting as a signed `int32` via NumPy.

After all 30 vectors are generated, the notebook runs a **cross-validation pass** — re-calling `poly_golden` on every row and asserting that `expected_sat` and `expected_ovf` match — before writing `test_vectors.csv`. This guards against regressions if pool definitions or the golden function are edited independently.

### 7.2 Test Vector Categories

The 30 vectors are drawn from five pools, 6 vectors per category per run:

#### Category 1 — Basic Cubic (pool size: 70)

Well-behaved small-magnitude inputs with all four coefficient terms active. Ten coefficient families are crossed with seven `x` values in `{−3, −2, −1, 0, 1, 2, 3}`:

| Representative coefficient sets | Purpose |
|----------------------------------|---------|
| `(2,3,4,5)`, `(1,2,3,4)`, `(3,1,2,1)` | Mixed non-zero coefficients |
| `(2,2,2,2)`, `(5,4,3,2)` | Uniform and descending coefficients |
| `(1,0,0,0)`, `(0,1,0,0)`, `(0,0,1,0)` | Isolated cubic, quadratic, and linear terms |
| `(3,0,0,1)`, `(1,1,1,1)` | Sparse and unit coefficients |

All results fit comfortably within INT32, so `expected_sat == expected_ovf` for every vector in this category. The sweep over `x ∈ {−3..3}` exercises sign handling for odd-power (cubic) and even-power (quadratic) terms simultaneously.

#### Category 2 — Linear / Isolated Terms (pool size: 70)

`A = B = 0` throughout, isolating the `C·x + D` path. Ten `(C, D)` pairs are crossed with `x ∈ {−10, −5, −1, 0, 1, 5, 10}`:

| Representative `(C, D)` pairs | Purpose |
|-------------------------------|---------|
| `(1,0)`, `(-1,0)` | Pure linear, positive and negative slope |
| `(2,5)`, `(-3,7)`, `(5,-5)` | Linear with non-zero constant offset |
| `(10,-4)`, `(100,0)`, `(-7,3)` | Larger linear coefficients |
| `(1,1)`, `(0,100)` | Unit slope with offset; constant-only passthrough |

This category confirms that zeroing out `A` and `B` via AXI writes correctly disables the cubic and quadratic pipeline paths, and that the linear and constant terms accumulate correctly in isolation.

#### Category 3 — Saturation (pool size: 20)

All vectors use `B = C = D = 0`, focusing exclusively on whether `A·x³` overflows INT32. The pool is evenly split between positive overflow (10 vectors) and negative overflow (10 vectors):

**Positive overflow** (`expected_sat = INT32_MAX = 2147483647`):

| `(A, x)` | `A·x³` |
|----------|--------|
| `(1, 1500)` | 3.375 × 10⁹ |
| `(1, 2000)` | 8.0 × 10⁹ |
| `(1, 1291)` | 2.151 × 10⁹ *(near boundary)* |
| `(1, 1300)` | 2.197 × 10⁹ |
| `(2, 1200)` | 3.456 × 10⁹ |
| `(1, 1400)`, `(3, 1100)`, `(1, 1350)`, `(2, 1100)`, `(1, 1600)` | Various > INT32_MAX |

**Negative overflow** (`expected_sat = INT32_MIN = −2147483648`):

| `(A, x)` | Notes |
|----------|-------|
| `(1, −1500)`, `(1, −2000)`, `(1, −1291)` | Negative `x`, positive `A` |
| `(−1, 1500)`, `(−1, 2000)`, `(−2, 1200)` | Positive `x`, negative `A` |
| `(1, −1300)`, `(1, −1400)`, `(−1, 1300)`, `(−1, 1400)` | Additional combinations |

For all saturation vectors, `expected_sat ≠ expected_ovf` by construction — the notebook asserts this at runtime and reports a count. In BATCH=0, 6 of the 6 sampled saturation vectors trigger this divergence, with 4 clamping to `INT32_MAX` and 2 clamping to `INT32_MIN`.

#### Category 4 — Mixed Signs (pool size: 70)

Negative leading coefficient `A` and alternating-sign coefficient sets, crossed with `x ∈ {−4, −2, −1, 0, 1, 2, 3}`. Ten coefficient families are used:

| Representative coefficient sets | Characteristic |
|---------------------------------|----------------|
| `(-1,2,-3,10)`, `(-2,3,-1,5)` | Dominant negative cubic, positive quadratic |
| `(-1,-1,1,1)`, `(-1,1,-1,1)` | Alternating signs throughout |
| `(1,-2,3,-4)`, `(-3,2,-1,0)` | Mixed polarity with cancellation at some `x` |
| `(-5,10,-2,7)`, `(-4,3,-2,1)` | Large magnitude quadratic partially compensates cubic |
| `(2,-3,2,-1)`, `(1,-1,1,-1)` | Alternating positive/negative patterns |

All results in this category are within INT32 range for the small `x` values used, so `expected_sat == expected_ovf`. The category specifically exercises correct sign propagation through the cubic term when `A < 0` and `x > 0` (result negative) versus `A < 0` and `x < 0` (result positive, since `(−A)(x³)` with odd `x` is positive).

#### Category 5 — Zero / Constant / Boundary `x` (pool size: 32)

Three sub-groups verify degenerate and boundary conditions:

**All-zero polynomial** (`A=B=C=D=0`, 8 vectors): `x ∈ {−5, −3, −1, 0, 1, 3, 5, 7}`. Expected output is `0` regardless of `x`. Confirms the pipeline produces no spurious output when all coefficients are zero.

**Constant-only** (`A=B=C=0`, 18 vectors): Six `D` values `{100, −100, 1, −1, 999, −50}` each tested with `x ∈ {−99, 0, 5}`. Expected output equals `D` regardless of `x`. Verifies that the constant term passes through correctly and is unaffected by any residual activity in the cubic/quadratic/linear paths.

**Boundary `x` with linear passthrough** (6 vectors): `x = INT32_MAX` or `x = INT32_MIN` paired with simple coefficient sets:

| `(A,B,C,D)` | `x` | Expected behavior |
|-------------|-----|-------------------|
| `(0,0,1,0)` | `INT32_MAX` | `result = INT32_MAX` (no overflow — fits in int64, then sat equals it) |
| `(0,0,1,0)` | `INT32_MIN` | `result = INT32_MIN` (similarly exact) |
| `(0,0,0,100)` | `INT32_MAX` | `result = 100` (constant only; `x` irrelevant) |
| `(0,0,0,100)` | `INT32_MIN` | `result = 100` |
| `(0,0,0,0)` | `INT32_MAX` | `result = 0` |
| `(0,0,0,0)` | `INT32_MIN` | `result = 0` |

These cases confirm that `INT32_MAX` and `INT32_MIN` inputs do not corrupt the pipeline when only lower-degree terms are active.

### 7.3 Sampling and Reproducibility

```python
BATCH = 0   # Change to get a different reproducible set of 30

PER_CATEGORY = 6  # 5 × 6 = 30 total

def sample_category(pool, n, seed):
    rng = random.Random(BATCH * 1000 + seed)
    shuffled = list(pool)
    rng.shuffle(shuffled)
    # Deduplicate while preserving shuffled order
    seen, out = set(), []
    for item in shuffled:
        if item not in seen:
            seen.add(item)
            out.append(item)
        if len(out) == n:
            break
    return out
```

Each category uses a distinct `seed` value (`1`–`5`), so the five draws are statistically independent within a given `BATCH`. Deduplication is applied within each category pool to prevent repeated identical vectors. The `BATCH` parameter acts as a global seed multiplier, making any batch value produce a fully different but deterministic 30-vector set.

### 7.4 Comparison Methodology

The testbench compares DUT outputs using the strict SystemVerilog `===` operator (4-state equality, distinguishing `X` and `Z` from `0` and `1`). A test vector **passes** if and only if:

```
result_sat === tv_exp_sat  AND  result_ovf === tv_exp_ovf
```

Any `X` or `Z` on the DUT outputs causes the comparison to fail, catching uninitialized pipeline state. The column names in `test_vectors.csv` use `expected_sat` / `expected_ovf` (from the notebook), while the testbench internally aliases these as `tv_exp_sat` / `tv_exp_ovf` after parsing.

### 7.5 Coverage Summary by Category

| Category | Vectors/run | `sat == ovf`? | Overflow direction tested | Key DUT paths exercised |
|----------|-------------|---------------|--------------------------|-------------------------|
| Basic cubic | 6 | Always | None | All four terms, small `x` |
| Linear/isolated | 6 | Always | None | C·x + D path only |
| Saturation | 6 | Never | Positive and negative | Full-precision cubic, sat/ovf divergence |
| Mixed signs | 6 | Always | None | Sign propagation, coefficient cancellation |
| Zero/constant/boundary | 6 | Always | None | Zero-path, constant passthrough, INT32 boundary `x` |

---

## 8. Appendix: Register Map

| Byte Offset | `awaddr[4:2]` | Register | Reset Value | Description              |
|-------------|---------------|----------|-------------|--------------------------|
| `0x00`      | `3'd0`        | `reg_A`  | `32'sd0`    | Cubic coefficient A      |
| `0x04`      | `3'd1`        | `reg_B`  | `32'sd0`    | Quadratic coefficient B  |
| `0x08`      | `3'd2`        | `reg_C`  | `32'sd0`    | Linear coefficient C     |
| `0x0C`      | `3'd3`        | `reg_D`  | `32'sd0`    | Constant term D          |
| `0x10`      | `3'd4`        | (status) | `32'h1`     | Read-only identity/status|
| `0x14`+     | other         | —        | —           | Reads return `32'h0`     |

> **Note:** All registers are 32-bit wide and interpret their contents as signed two's complement integers. Writes to undefined addresses are silently ignored (AXI `BRESP` still returns `OKAY`).

---

*End of Document*
