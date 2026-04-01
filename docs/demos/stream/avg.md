---
title: Streaming Average
parent: AXI4-Streaming
nav_order: 2
has_children: false
---

# Streaming Average

## Writing the Vitis HLS Kernel

To illustrate the AXI4-Streaming protocol we consider implementing a simple moving average:

```python 
y[n] = 1/(win_size)*( x[n]**2 + ... + x[n-win_size+1]**2 )
```

where `x[n]` is a stream of inputs and `y[n]` is the moving average window of the 
A Vitis HLS kernel implementing the moving average can be found in `avg_filt.cpp`,
the main body of the function is shown below.
Important points to note:

- The stream interfaces defined by `in_stream` and `out_stream`
- The Vitis IP is implemented as an infinite while loop
- In each iteration, it perfoms an `in_stream.read()` to read one element and an `out_stream.write()` to write one element.

```cpp
void avgfilt(
    hls::stream<float>& in_stream, 
    hls::stream<float>& out_stream)
{

#pragma HLS INTERFACE axis port=in_stream
#pragma HLS INTERFACE axis port=out_stream
#pragma HLS INTERFACE ap_ctrl_none port=return

    // Last squared values
    static float xsq0 = 0.;
    static float xsq1 = 0.;

    constexpr float inv_win_size = 1/static_cast<float>(win_size);

    while (1) {
        // Exit when in_stream is empty.  This is for C simulation
        // only to support Vitis C call convention.
#ifndef __SYNTHESIS__
        if (in_stream.empty()) {
            break;
        }
#endif

#pragma HLS PIPELINE II=1
        // Read value
        float xi = in_stream.read();
        float xsq = xi*xi;

        // Output value
        float yi = (xsq + xsq0 + xsq1)*inv_win_size;
        out_stream.write(yi);

        // Update delay line
        xsq1 = xsq0;
        xsq0 = xsq;

    }

}
```

## The Testbench

The testbench is in the file `avfilt_tb.cpp` and simply 
- Creates input and output streams
- Loads the input stream with a set of values
- Runs the kernel on the input stream
- Pulss the data from the output stream and compares the values to the expectd values