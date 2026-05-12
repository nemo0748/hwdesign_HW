---
title: Using Python
parent: NYU Remote Server
nav_order: 4
has_children: False
---
# Using Python 

The NYU EDA servers provide a system Python installation, but **`pip` is not available** and you do **not** have permission to install system‑wide packages.  
To work around this, students should use **`uv`**, a modern, fast, user‑level Python package manager that installs entirely in your home directory.
This suggestion was provided by the student Ashesh Kaji, so please thank him next time.

**'`uv`** is a drop‑in replacement for:
- `pip`
- `virtualenv`
- `pipx`
- `pip-tools`
- `poetry` (for basic workflows)

It requires **no sudo access** and works perfectly on the NYU servers.

---

## 1. Install `uv`

Log into the EDA server:

```bash
ssh <netid>@ecs02.poly.edu
```

Then install `uv` into your home directory:

```bash
curl -LsSf https://astral.sh/uv/install.sh | sh
```

This command installs `uv` in a local binary in the directory `~/.local/bin`. 
We need to add the path to the system.  Linux has different *shells*,
and the commands differ slightly depending on the shell type.
Below are the commands for the **tcsh** shell type.  You can verify your shell
type with 

```bash
echo $SHELL
```

If you are not in the tcsh type, the commands will not work.  I will
put in commands later for bash.

Now, assuming you are in tcsh, to add `uv` to your system path
and to disable the existing python version,
open the file `~/.tcshrc` and add the following lines at the end of that file.

```bash
setenv PATH "$HOME/.local/bin:$PATH"
unsetenv PYTHONPATH
```

Note that you can edit the file with any program in linux such as `vi`.

Now rerun the shell initialization:

```bash
source ./.tschrc
```

Note that you do not need to perform this command for subsequent shells.
This command will be run automatically.

You can verify installation with:

```bash
uv --version
```

---

## 2. Create a Virtual Environment

Navigate to the directory where you cloned the `hwdesign` repo.  Generally,
this is `~/hwdesign`.  
Inside that project directory:

```bash
uv venv
```

This creates a `.venv/` folder containing a private Python environment.

Activate it:  

```bash
source .venv/bin/activate.csh  # for tcsh
source .venv/bin/activate      # for bash
```

Your prompt should now show something like:

```
(.venv) <netid>@ecs02:~/project$
```

You can deactivate with:

```
deactivate
```

## 3. Install Your Project or Dependencies

From the `hwdesign` directory, while the virtual environment
is activated, install the packages with:

```bash
uv pip install -r requirements.txt
```

Then, install the `xilinxutils` package as editable:

```bash
uv pip install -e .
```


## 4. Running Python

Once the environment is activated:

```bash
uv run python your_script.py
```

You can also run the scripts like `sv_sim` with:

```
uv run sv_sim --source [source files] --tb [tb_files]
```

Everything runs inside your private environment, not the system Python.

---

## 5. Why We Use `uv` Instead of `pip`

The NYU EDA servers:

- Do **not** include `pip`
- Do **not** allow system‑wide package installation
- Use a system Python that students cannot modify

`uv` solves all of these problems:

- Installs into your **home directory**
- Requires **no sudo**
- Manages virtual environments automatically
- Works on macOS, Windows, and Linux
- Is significantly faster and more reliable than pip

    