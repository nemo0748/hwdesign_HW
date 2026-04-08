---
title: Course Units
parent: NYU class
nav_order: 1
has_children: true
---


A tentative set of units is as follows.  I've only added a few units so far, but I am hoping to get done by the end of the semester.  Problem sets can also be found on the [AI Grader portal](../aiautograder/), but this website is experimental.

* Course Introduction
    * Lecture:  [[PDF]](https://github.com/sdrangan/hwdesign/tree/main/units/unit00_course_intro/course_intro.pdf) [[pptx]](https://github.com/sdrangan/hwdesign/tree/main/units/unit00_course_intro/course_intro.pptx) 
    * [Software and hardware set-up](./setup/)
* Unit 1:  Data Types and Combinational Logic
    * [Review of number systems](../../units/unit02_numbers/review_numbers.pptx) from previous class
    * Lecture:  [[PDF]](https://github.com/sdrangan/hwdesign/tree/main/units/unit01_basic_logic/basic_logic.pdf) [[pptx]](https://github.com/sdrangan/hwdesign/tree/main/units/unit01_basic_logic/basic_logic.pptx) 
    * [Problems](../../units/unit01_basic_logic/prob/basic_logic.html) 
* Unit 2:  Sequential Logic and Finite State Machines
    * Lecture:  [[PDF]](https://github.com/sdrangan/hwdesign/tree/main/units/unit02_fsm/fsm.pdf) [[pptx]](https://github.com/sdrangan/hwdesign/tree/main/units/unit02_fsm/fsm.pptx) 
    * [SystemVerilog Demo:  Simple scalar functions](../demos/simp_fun/)
    * [Problems](../../units/unit02_fsm/prob/fsm.html) 
    * Lab:  [Division by conditional subtraction](../labs/subc/)
* Unit 3:  Floating and Fixed Point Arithmetic
    * Lecture:  [[PDF]](https://github.com/sdrangan/hwdesign/tree/main/units/unit03_fixp/fixp.pdf) [[pptx]](https://github.com/sdrangan/hwdesign/tree/main/units/unit03_fixp/fixp.pptx) 
    * [In-class exercise](../../units/unit03_fixp/fixp_inclass.ipynb)
        <a href="https://colab.research.google.com/github/sdrangan/hwdesign/blob/main/units/unit03_fixp/fixp_inclass.ipynb" target="_blank">
        <img src="https://colab.research.google.com/assets/colab-badge.svg" alt="Open In Colab"/> 
        </a> 
    * [SystemVerilog Demo:  Fixed point implementation](../demos/fixp/)
    * [Problems](../../units/unit03_fixp/prob/fixp.html)
    * Lab:  [Fixed point implementation of a polynomial](../labs/cubic/)
* Unit 4:  Bus Basics and Memory-Mapped Interfaces
    * Lecture:  [[PDF]](https://github.com/sdrangan/hwdesign/tree/main/units/unit04_procif/procif.pdf) [[pptx]](https://github.com/sdrangan/hwdesign/tree/main/units/unit04_procif/procif.pptx) 
    * [Vitis HLS Demo:  Simple AXI4-Lite interface](../demos/procif/) 
    * Lab:  [Build a root solving accelerator in Vitis HLS](../labs/rootsolve/)
    * [Problems](../../units/unit04_procif/prob/procif.html)
* Unit 5:  FIFO and Streaming Interfaces
    * Lecture:  [[PDF]](https://github.com/sdrangan/hwdesign/tree/main/units/unit05_fifo/fifo.pdf) [[pptx]](https://github.com/sdrangan/hwdesign/tree/main/units/unit04_fifo/fifo.pptx) 
    * [Vitis HLS Demo: AXI4-Streaming FIFO interface](../demos/fifoif/)
    * [Problems](../../units/unit05_fifo/prob/fifo.html)
* Unit 6:  Timing and Critical Path Analysis
    * Lecture:  [[PDF]](https://github.com/sdrangan/hwdesign/tree/main/units/unit05_fifo/fifo.pdf) [[pptx]](https://github.com/sdrangan/hwdesign/tree/main/units/unit04_fifo/fifo.pptx) 
    * [Problems](../../units/unit06_timing/prob/timing.html)
* Unit 7:  Pipelining and Loop Optimization
    * [Vitis HLS Demo: A pipelined polynomial accelerator](../demos/loopopt)
    * [Problems](../../units/unit07_loopopt/prob/loopopt.html)
    * Lab:  [Building a pipelined line intersection accelartor](../labs/intersect/)
* Unit 8:  Dataflow and Shared Memory architectures
* Unit 9:  Systolic Arrays
* Unit 10:  Multi-Threaded Control