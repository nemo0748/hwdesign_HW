---
title: Building the Vitis HLS IP
parent: Root Solver
nav_order: 2
has_children: false
---

# Building the Vitis HLS IP and Testbench

We next build the Vitis HLS IP to run the Euler root solving
iteration.  Copy the files from:

```
rootsolve/partial/fsolve.cpp -> rootsolve/fsolve.cpp
rootsolve/partial/tb_fsolve.cpp -> rootsolve/tb_fsolve.cpp
```

Then, complete all the sections marked `TODO`.  In `fsolve.cpp`,
try to make the algorihm match the python.  You will have to:

- Set the `#pragma` statements for the AXI4-Lite interface
- Implement the loop including the early termination condition

In the testbench, `tb_fsolve.cpp`, I have already implemented all
the file reading and writing.  You just need to write the
condition to ensure that the Vitis IP matches the outputs from the python.

You now have two options to test the Vitis HLS IP and testbench:

- [Use the Vitis HLS GUI](./vitis_gui.md)
- [Call Vitis HLS from the command line](./vitis_tcl.md)

Both methds will result in the same final outputs and will receive full credits.
So, it is just your preference.





