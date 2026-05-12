---
title: Vitis and Vivado
parent: Support Material
nav_order: 1
has_children: true
---

# Vitis and Vivado

The two key pieces of software we will use in this class.  Both are from AMD and can be downloaded together:

*  [**Vitis HLS** (High-Level Synthesis)](https://www.amd.com/en/products/software/adaptive-socs-and-fpgas/vitis/vitis-hls.html) is a tool we will use to design the hardware accelerators of **Vitis IP** (IP = intellectual property).  We can write the specification for the IP in high-level language like  C, C++, or OpenCL, and Vitis HLS converts it automatically into optimized, lower-level **register transfer level** (RTL) specification for the hardware 
* [**Vivado**](https://www.amd.com/en/products/software/adaptive-socs-and-fpgas/vivado.html) is Xilinx / AMD’s FPGA design suite that lets you create, simulate, and synthesize digital circuits. It provides a graphical interface to build projects, configure hardware like the Zynq processor, and generate bitstreams for deployment on supported boards.We then integrate that IP into a larger FPGA designs in Vivado.
In this class, we will integrate the Vitis IP into the larger Vivado FPGA project and deploy that onto the FPGA.

Both Vitis and Vivado have free versions that are fine for this class.  But, to access them you will need to [create and AMD account](https://login.amd.com/).


This directory provides some notes in setting up and using this software.
