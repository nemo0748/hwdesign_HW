# Project Setup
To be able to run the code, the environment must be set up. The following steps explain how to setup this entire project in a NYU server. 

# Using Python on NYU EDA Servers

The NYU EDA servers provide a system Python installation, but `pip` is not available and you do not have permission to install system-wide packages.

To work around this, students should use `uv`, a modern, fast, user-level Python package manager that installs entirely in your home directory. This suggestion was provided by the student Ashesh Kaji.

`uv` is a drop-in replacement for:

- `pip`
- `virtualenv`
- `pipx`
- `pip-tools`
- `poetry` (for basic workflows)

It requires no `sudo` access and works perfectly on the NYU servers.

---

# 1. Install uv

Log into the EDA server:

```bash
ssh <netid>@ecs02.poly.edu
```

Then install `uv` into your home directory:

```bash
curl -LsSf https://astral.sh/uv/install.sh | sh
```

This command installs `uv` into a local binary in the directory:

```bash
~/.local/bin
```

We need to add this path to the system. Linux has different shells, and the commands differ slightly depending on the shell type.

Below are the commands for the `tcsh` shell type.

You can verify your shell type with:

```bash
echo $SHELL
```

If you are not using `tcsh`, the commands below will not work.

Now, assuming you are using `tcsh`, open the file:

```bash
~/.tcshrc
```

Add the following lines at the end of the file:

```bash
setenv PATH "$HOME/.local/bin:$PATH"
unsetenv PYTHONPATH
```

You can edit the file using any Linux editor such as `vi`.

Now rerun the shell initialization:

```bash
source ./.tcshrc
```

Note that you do not need to perform this command for subsequent shells. This command will be run automatically.

You can verify installation with:

```bash
uv --version
```

---

# 2. Create a Virtual Environment

Navigate to the directory where you cloned the `hwdesign` repository. Generally, this is:

```bash
~/hwdesign
```

Inside that project directory, run:

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

From the `hwdesign` directory, while the virtual environment is activated, install the packages with:

```bash
uv pip install -r requirements.txt
```

Then install the `xilinxutils` package as editable:

```bash
uv pip install -e .
```

---

# 4. Running Python

Once the environment is activated:

```bash
uv run python your_script.py
```

You can also run scripts like `sv_sim` with:

```bash
uv run sv_sim --source [source files] --tb [tb_files]
```

Everything runs inside your private environment, not the system Python.
