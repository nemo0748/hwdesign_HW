---
title: AXI4-Streaming
parent: Demos
nav_order: 5
has_children: true
---

# AXI4-Stream Interface

In the [simple scalar function example](../scalar_fun/), the IP interfaced with the processor via a set of simple
AXI4-Lite command registers. The AXI4-Lite interface is often inefficient for large data transfers since each read and write typically takes multiple clock cycles.  In this demo, we introduce the **AXI4-Streaming** interface,
an efficient protocol for transmitting long, continuous bursts of data.   We will specifically focus on the
so-called **pure AXI4-Stream** where data is continuously transmitted with no explicit start and end of the stream.  Later demos will introduce **block** transfers.

In going through this demo, you will learn to:

- Implement simple streaming Vitis HLS modules with pure streaming interfaces
- Simulate the Vitis HLS IP
- Visualize the AXI-Stream protocols in timing diagrams


---

Go to [AXI4-Streaming Protocol](./axistream.md)

 
