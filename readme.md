# VAC Project Location
Select the labs/project/ folder which will have all the project files. The instructions below also provide a quick way to enter it quickly by using the provided cd command.
* The top-level `README.md` in `hwdesign_HW/` explains project setup, environment configuration, and how to run the project using VS Code.
* The detailed hardware project documentation is located in `labs/project/README.md`. This file contains the VAC IP design description, architecture, interfaces, module organization, and implementation details.
* Verification evidence and results analysis are located in `labs/project/verification.md`, including simulation outputs, verification methodology, and performance evaluation.
* The directory `labs/project/sim/` contains the simulation-generated files and artifacts produced during verification.
* RTL source files and testbench files are also contained within the `labs/project/` directory structure.

All implementation, documentation, verification artifacts, and simulation outputs are contained within the same repository.

Teammates: I did not have a team this time. I worked by myself and completed the project to the best of my ability.

# VAC Project Setup
To be able to run the code, the environment must be set up. The following steps explain how to setup this entire project in a NYU server. 

# Using Python on NYU EDA Servers

The NYU EDA servers provide a system Python installation, but `pip` is not available and you do not have permission to install system-wide packages.

To work around this, students should use `uv`, a modern, fast, user-level Python package manager that installs entirely in your home directory.

---

# 1. Install uv

Log into the EDA server:

```bash
ssh <netid>@ecs02.poly.edu
```

Clone this repository so it appears in VSCode.
Enter the hwdesign_HW folder on VSCode. Execute the following commands in the terminal.

Install `uv` into your home directory:

```bash
curl -LsSf https://astral.sh/uv/install.sh | sh
```

Set the paths for accessing the files. 
```bash
setenv PATH "$HOME/.local/bin:$PATH"
unsetenv PYTHONPATH
```

Verify installation with:

```bash
uv --version
```
---

# 2. Create a Virtual Environment

Run:

```bash
uv venv
```

This creates a `.venv/` folder containing a private Python environment.

Activate it with:

```bash
source .venv/bin/activate.csh  # for tcsh
source .venv/bin/activate      # for bash
```

Your prompt should now show something like:

```bash
(.venv) <netid>@ecs02:~/project$
```

You can deactivate the environment with:

```bash
deactivate
```

---

# 3. Install Your Project or Dependencies

From the `hwdesign_HW` directory, while the virtual environment is activated, install the packages with:

```bash
uv pip install -r requirements.txt
```

Then install the `xilinxutils` package as editable:

```bash
uv pip install -e .
```

---

# 4. Running VAC

Once the environment is activated, enter the Project folder from the hwdesign_HW folder. 
Use the following command to do so. 
```bash
cd labs/project
```

Once in the project folder, run the following command to execute the project testbenches and code. 

```bash
uv run sv_sim --source vac.sv --tb tb_vac_csv.sv
```

The logs, output, and results.csv files will be placed in the sim folder. 
