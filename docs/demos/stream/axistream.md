---
title: AXI4-Stream protocol
parent: AXI4-Streaming
nav_order: 1
has_children: false
---

# AXI4-Stream Protocol Overview

## Protocol Structure

The  **AXI4-Stream protocol** is a
lightweight data transfer mechanism that uses a simple **handshake** between the source (master) and sink (slave). Data is sent in units called **bursts**, which are sequences of one or more data **beats**. Each beat is transferred when both sides agree: the source asserts **TVALID** to indicate valid data, and the sink asserts **TREADY** to signal readiness to accept it.

Within a burst:

- **TVALID** stays high while the source has data to send.
- **TREADY** may toggle depending on whether the sink can accept data.
- A **transfer** occurs only when both TVALID and TREADY are high in the same cycle.

This handshake ensures **flow control**: the source never overruns the sink, and the sink can apply backpressure by deasserting TREADY.

---

