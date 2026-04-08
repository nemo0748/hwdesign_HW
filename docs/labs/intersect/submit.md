---
title: Submitting the Lab
parent: Line intersection
nav_order: 4
has_children: false
---

# Submitting the Lab

For NYU students that want to receive credit for the lab,
if you are running the Vitis and Vivado on your local machine, 
run the python command:

```bash
python run_tests.py --tests all --submit
```

Or, if you are on the NYU server:

```bash
uv run python run_tests.py --tests all --submit
```

This program will look at the test vectors and validate that the tests have passed.
The program will create a zip file `submission.zip` with several files.  Submit this zip folder on Gradescope on the lab assignment.  A Gradescope autograder will upload the grade.  If you are on the NYU server, you will need to copy the `submission.zip` file back to your local machine to submit.  


