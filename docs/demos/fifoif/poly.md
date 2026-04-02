---
title: Polynomial Example
parent: Command-Response FIFO Interface
nav_order: 2
has_children: false
---

# Polynomial Example

## Files

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

----

Go to [defining the data structures](./datastructs.md)

## Message Definitions

