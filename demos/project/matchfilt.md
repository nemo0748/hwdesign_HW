# Matched Filter Detector

## IP definition

Detecting a preamble in a streaming input of data is a basic problem in digital communications
and RF sensing.  In this project, the processing system (PS) will send the IP a known waveform
to detect.  The IP is in two parts:  A transmitter (TX) that generates a test data stream with the preamble.
A receiver (RX) that performs a match filter detector to search for the preamble.  When it has detected
the preamble, it captures a set of samples (number set by the PS) and dumps them to shared memory.
It then informs the PS that the samples are collected (via a FIFO) and the PS can retrieve them and then
initiate another search.

## Architecture

The TX will be implemented with a circular buffer with a shared memory interface to the PS to load
the data.  Data loads will come via a command from the PS with the source location and data length.

The RX will perform the MF via frequency-domain correlation.  It will take the FFT of the signal,
multiply by the target, take the IFFT, and perform a peak search.  These operations can be done in a single
Vitis block.