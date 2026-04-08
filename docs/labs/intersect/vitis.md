---
title: Building and Simulating Vitis HLS IP
parent: Line Intersection
nav_order: 2
has_children: false
---

# Building and Simulating the Vitis HLS IP and Testbench

## Completing the Vitis IP
We next build the Vitis HLS IP to run the line intersection.  Copy the file:

```
intersect/partial/line_inter.cpp -> rootsolve/line_inter.cpp
```

Then, complete all the sections marked `TODO` in the Vitis kernel file.  You will need to implement the
compute loops to obtain a pipeline with `II=1`.  


## C simulation

Once you have completed the Vitis kernel, you can simulate it with:

```bash
vitis-run --mode hls --tcl run_hls.tcl
```

If you are using the [NYU server](../../support/nyuremote/), you must use the older command line syntax

```bash
vitis_hls -f run_hls.tcl
```

If you prefer, you can also run the C simulation from the GUI.  This command will run the C simulation and C synthesis.
The testbench will:

- read the data file inputs from the python simulation
- create an instance of the kernel as a DUT (device under test)
- inject the test vectors into the DUT
- read the output 
- measure the MAE 
- save the output for files that can be read in the python program

You can now test that the simulation was correct with:

```bash
python run_tests.py --tests functional
```

---

Go to 


