---
title: Building and Validating the Python Model
parent: Line Intersection
nav_order: 1
has_children: false
---

# Building and Validating a Python Model

Similar to the other labs,before implementing the function in Vitis HLS, it is better to
develop a model in a language like python where the debugging is simple.
This python simulation will serve as a **golden model** meaning that it can provide a verified reference for the hardware design. 
Copy the jupyter notebook in:

```bash
hwdesign/labs/intersect/partial/line_inter.ipynb
```

to

```bash
hwdesign/labs/rootsintersectolve/line_inter.ipynb
```

Then complete this notebook to:

- Define the data schemas for the messages to and from the IP
- Create a python distance function calculator
- Create test messages to send to the python golden model
- Store the test vectors
- Create a root solve function using the Euler method.


Once the python model is complete, you can run the test

```bash
python run_tests.py --tests python
```

If you are on the [NYU server](../../support/nyuremote/), you will need
to follow the [instructions for running python with uv](../../support/nyuremote/python.md),
and run

```bash
uv run python run_tests.py --tests python
```

This test will inspect the test outputs to make sure that the algorithm
is computing the distance correctly.


----

Go to [Building and simulating the VITIS HLS kernel](./vitis.md).





