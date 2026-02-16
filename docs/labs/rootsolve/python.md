---
title: Building and Validating the Python Model
parent: Root Solver
nav_order: 1
has_children: false
---

# Building and Validating a Python Model

Similar to the [previous lab](../cubic/),
before implementing the function in Vitis HLS, it is better to
develop a model in a language like python where the debugging is simple.
This python simulation will serve
as a **golden model** meaning that it can provide a verified reference for the 
hardware design. 

Copy the jupyter notebook in:

```bash
hwdesign/labs/rootsolve/partial/rootsolve.ipynb
```

to

```bash
hwdesign/labs/rootsolve/cubic.ipynb
```

Then complete this notebook to:

- Create a floating point model of the cubic function
- Create a root solve function using the Euler method.
The function takes the polynomial coefficients, step size,
and maximum number of iterations among other parameters as inputs,
and outputs an estimate, `x_root` of the root of the polynomial,
and the function value `f(x_root)`.

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
is producing an estimate of the root `x_root` such that `f(x_root)`
is sufficiently small.


----

Go to [Building the VITIS HLS](./vitis.md).





