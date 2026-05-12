---
title: Building and Validating the Python Model
parent: Cubic Fixed Point
nav_order: 1
has_children: false
---

# Building and Validating a Python Model

Before implementing the function in SystemVerilog, it is better to
develop a model in a language like python where the debugging is simple.
This python simulation will serve
as a **golden model** meaning that it can provide a verified reference for the 
hardware design. 

Copy the jupyter notebook in:

```bash
hwdesign/labs/cubic/partial/cubic.ipynb
```

to

```bash
hwdesign/labs/cubic/cubic.ipynb
```

Then complete this notebook to:

- Create a floating point model of the cubic function
- Implement a fixed point model with variable bit widths
- Evaluate the error with two settings: `(W,F)=(16,8)` and  `(W,F)=(16,12)`.
The first has minimal overflow while the second has significant overflow.
- Generate test vectors

The test vectors outputs will be stored in:

- `test_vectors/tv_w16_f8.csv`
- `test_vectors/tv_w16_f12.csv`

The test vectors correspond to the two bit width settings

Once the python model is complete, you can run the tests

```bash
python run_tests --tests python_f8 python_f12
```

This test will inspect the test outputs to make sure that approximation error is what is expected.


----

Go to [Building a SystemVerilog module](./sv.md).





