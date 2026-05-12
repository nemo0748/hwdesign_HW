---
title: System Verilog Implementation
parent: Cubic Fixed Point
nav_order: 2
has_children: false
---

# System Verilog Implementation

To implement the cubic function in fixed point, I have provided an initial skeleton
of the code in `cubic.sv`.  There are several ways to implement the cubic polynomial function.
The skeleton code provided uses a three stage pipeline.
Other pipelining implementations are possible, and you can deviate from the suggested
code if you want.

The pipelining in the module operates in three stages:

- **Stage 0**:  Register all the inputs: 

```
x_s0 <= x, 
a0_s0 <= a0
a1_s0 <= a1
a2_s0 <= a2
```

    where we have added variables with an `_s0` prefix to note register values
    at the end of stage 0.

- **State 1**:  
    - Compute the square and linear terms:

```
x2_s1 <= fixed point ( x_s0 * x_s0 )
ax1_s1 <= fixed point (a1_s0 * x_s0 + a0_s0 )
```

     - Pass through `x_s0` and `a2_s0` since they will be needed in the final stage

```
x_s1 <= x_s0
a2_s1 <= a2_s0
```

- **Stage 2**:  combinational output:

```
x3 = fixed point( x_s1 * x2_s1 )
ax2 = fixed point( a2_s1 * x2_s1 )
y = fixed point( x3 + ax2 + ax1_s1 )
```


Here the challenge is to implement the fixed point implementations of the various products.
Recall that  a product in fixed point of two `Q(WID,FBITS)` numbers is implemented as:
```
prod = ((a*b) >> FBITS)
```

Once this product is assigned to a lower width, it may overflow. To saturate instead of wraparound,
I have a `sat` function, so you can write:

```
prod = sat((a*b)>>FBITS)
```

which will saturate back to `WID` bits.

If you want, just ignore the saturation at first.  For the test vector with
`WID=16, FBITS=8`, there is no saturation.  So, you can get this working first.


----

Go to [validate a single test case](./sim_sing.md).
