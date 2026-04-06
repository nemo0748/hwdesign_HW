---
title: Polynomial Example
parent: Command-Response FIFO Interface
nav_order: 2
has_children: false
---

# Polynomial Example

We will illustrate the command-response FIFO design mehodology with an example of a simple polynomial accelerator.  The accelerator computes a polynomial function of `x[k]`:

```python
y[k] = a[0] + a[1]*x[k] + ... + a[d-1]*x[k]**d
```

All the examples can be found in `hwdesign/demos/stream` with the main notebook in `poly_demo.ipynb`.

## Protocol

The IP we will build will follow the folliwng command-response FIFO protocol:

- The PS sends a `PolyCmdHdr` with the coefficients, `coeffs`, number of samples, `nsamp`, and a transaction ID, `tx_id`.
- The IP repsonds with a `PolyRespHdr` message with the transcation ID, `tx_id`.  This ensures that the IP is in sync and has received the data.
- The PS then starts streaming the input data array `x`. 
- As the data streams in, the IP computes the output data array `y`.
- When complete the IP sends a `PolyRespFtr` message with an indication of the number of samples and any error code.

## Implementation


An implementation of the polynomial kernel and testbench can be found in the  [notebook](https://github.com/sdrangan/hwdesign/blob/main/demos/stream/poly_demo.ipynb).  The implementation uses tthe [Pysilicon package](https://sdrangan.github.io/pysilicon/docs/guide/schema/).  
The notebook shows:

- How to construct python PySilicon **data schemas** representing the messages
- **Auto-generate** HLS C++ header files corresponding to the messages
- Write Vitis HLS C++ code using the **serialization** and **deserialization** functions in the headers to transfer data between the kernel and IP using the messages
- Test for synchronization errors
- Run the C simulation and RTL simulation
- Extract a **VCD** file from the RTL simulation
- View the AXI4 timing bursts.
