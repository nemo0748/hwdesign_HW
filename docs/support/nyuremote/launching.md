---
title: Launching Vitis and Vivado
parent: NYU Remote Server
nav_order: 3
has_children: False
---

## Launching Vitis and Vivado within the Remote server

Once you are in the server, you can now activate most of the command line AMD tools with:

```bash
source tcshrc_xilinx_local
```

Run this command from your home directory.  For example, you can run:

```bash
$ which vitis_hls
$ vitis_hls -version
$ which vivado
$ vivado -version
```

One caution:  The NYU installation is 2023.1, which is a bit old.  But, so far,
everything has worked.  You can now follow the instructions for 
[launching Vitis and Vivado](../amd/lauching.md) for either the command line
or GUI tools.  Note that you since you have run `source tcshrc_xilinx_local`,
you can skip the steps for running `settings64.bat`.

---

Go to [running python remotely](./python.md)
