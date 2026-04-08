---
title: Building and Simulating Vitis HLS IP
parent: Line Intersection
nav_order: 3
has_children: false
---

# Building and Simulating the Vitis HLS IP and Testbench

Once the functional test is working, you can validate that the pipeling is working.  The C synthesis produces an XML report.  You can locate it in 

```bash
hls_component/solution1/syn/reports/csynth.xml
```

We will use the PySilicon utilities to parse this report.  You should see that all loops obtain a `PipelineII = 1`.
Run the jupyter notebook to parse the XML file and extract:

- the pipeline latency and II of each loop including the main compute loop
- the resource utilization

---

Go to [submission](./submit.md)

