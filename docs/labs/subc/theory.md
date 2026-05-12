---
title: Theory
parent: Conditional Subtraction Division
nav_order: 1
has_children: false
---

# Theory and Building a Python Golden Model

## Theory

We are given two unsigned integers `a` and `b` with `b > a >=0` and we want to find
the quotitient `a/b`.  In general, the quotient could require an arbitrary number of bits.  The division by conditional subtraction algorithm computes an integer number `z` that can be used to compute a  `nbits`
approximation to the quotient `a/b` of the form:

```python
     qhat = z / (2**nbits) ~= a/b.
```

So, the output of the algorithm `z` is the numerator of the quotient with values `z = 0,...,2**(nbits-1)-1`.  Note that, since we assumed `a < b`, the true quotient `a/b < 1`.

For example suppose `a=3` and `b=10`, we can get the approximations:

* `nbits=1`, `z=0`, `qhat=0/1 = 0`
* `nbits=2`, `z=1`, `qhat=1/4 = 0.25`
* `nbits=4`, `z=4`, `qhat=1/4 = 0.25`
* `nbits=8`, `z=76`, `qhat=76/256 = 0.297`

So, as we increase the number of bits, we get closer to the true fraction `a/b=0.3`.

The python equivalent of the algorithm is simple:

```python
z = 0
for i in range(nbits):
    z <<= 1  # Shift left to make space for the next bit
    a <<= 1  # Shift left to bring down the next bit of the dividend
    if a >= b:
        a -= b
        z |= 1  # Set the least significant bit of z to 1
return z
```

We won't perform the algorithm analysis, but it can shown that if `qhat = z/(2**nbits)`, the approximation
error is bounded as

```python
    qhat <= a/b <=  qhat + 2**(-nbits)
```

so the approximation error decreases geometrically with the number of bits.

---

Go to [Building a python model](./python.md).





