---
title: Python Golden Model
parent: Fixed Point Design
nav_order: 1
has_children: false
---

# Creating and Simulating a Python Golden Model


Before writing the SV code, we build a python model that will serve
as the **golden model** reference for the SV implmentation.
Go through the [jupyter notebook](https://github.com/sdrangan/hwdesign/blob/main/demos/fixp/piecewise.ipynb).

<a href="https://colab.research.google.com/github/sdrangan/hwdesign/blob/main/demos/fixp/piecewise.ipynb" target="_blank">
  <img src="https://colab.research.google.com/assets/colab-badge.svg" alt="Open In Colab"/>
</a>


The code will:

- Creates a floating point model for the piecewise linear function
- Creates a python simulation of a fixed point version
with configurable number of bits 
- Measures the approximation error as a function of the number
of bits
- Creates test vectors that can be used to verify the SV model
