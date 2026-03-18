---
title: Plannng the Project
parent: Project
nav_order: 2
has_children: false
---

## AI‑Based Planning of Your Project

### Overview

As described before, the project is designed around an [**AI‑assisted planning workflow**](./ai_workflow.md). 
Before you begin implementing your hardware IP, you will use AI tools to help you explore design options, refine your architecture, and iteratively improve your specification. To make this process concrete and time‑bounded, the planning phase is divided into **two structured submissions**, each supported by automated AI feedback:

- **Initial Plan** — a high‑level description of your IP’s purpose, behavior, and overall architecture, written with the help of AI to clarify the problem and surface design choices.
- **Detailed Plan** — a deeper technical specification that defines your module interfaces, configuration structure, testbench strategy, and incremental development path, again refined through AI‑guided iteration.

Both submissions will be evaluated using the [LLM grader](../aiautograder/), which provides rubric‑aligned feedback to help you strengthen your design. As usual, you can submit to the LLM grader as many times as you want until you are satisfied with the design.  You are expected to use AI tools actively throughout this process—not as a replacement for your own reasoning, but as a structured partner in exploring alternatives, checking consistency, and improving clarity.

---

## GitHub Repository

All project materials must be maintained in a **GitHub repository**.  
To begin, create a new repo for your team and place all planning documents, code, and results there.

Why GitHub?

- It provides **version control**, allowing you to track changes and collaborate effectively.
- It is the standard tool used in industry for hardware and software development.
- It keeps your documentation, code, and test results organized in one place.
- The **LLM grader uses web search** to inspect your repository directly, enabling it to read your planning documents, module definitions, and final implementation.

Your repo will become the central artifact of your project — treat it as a professional engineering repository.

---

## Initial Plan

Create a file named something like `initial_plan.md` in your repo.  
This document should contain **two main sections**, written clearly and concisely.
I will put some [examples](./examples/) if you want some ideas.  

### 1. IP Definition

Describe:

- **What your IP does**  
  The core functionality, the problem it solves, and the domain it applies to.

- **How it interacts with the PS and peripherals**  
  What data it receives, what it outputs, and how it is controlled.

- **The mathematical operations involved**  
  Be precise. Include formulas, pseudo‑code, or algorithmic descriptions as needed.

A good IP definition makes the intended behavior unambiguous.

### 2. IP Architecture

Describe:

- **The major sub‑modules**  
  What components your design will be broken into.

- **How the sub‑modules communicate**  
  Interfaces, dataflow, control signals, memory access, etc.

- **Why modularization matters**  
  Explain why you are decomposing the design in this way and how it supports incremental development and testing.

### Submitting the Initial Plan

To submit your initial plan to the LLM grader:

1. Commit `initial_plan.md` to your GitHub repo.
2. Copy the **GitHub URL** of the file.
3. Paste the URL into the LLM grader submission box.

The grader has been provided with the rubric for this stage and will give structured feedback.  
You are encouraged to use AI extensively while drafting this document — the clearer your plan, the more effective AI will be in later stages.

---

## Detailed Plan

After receiving feedback on your initial plan, create a second document (e.g., `detailed_plan.md`) that expands your design into a fully implementable specification. This submission combines three components:

### 1. Module Definitions

For each module:

- Describe its **function**.
- Specify its **inputs and outputs**.
- Define **message formats**, data widths, and control signals.
- Explain any **timing assumptions** or sequencing requirements.
- Optionally use a schema (e.g., pysilicon) to formalize message structures.

### 2. Testbench Definition

Describe your verification strategy:

- What scenarios will you test?
- What do you use as the Golden model?  You should consider using Python as in class. 
- How will you test functional correctness?
- How will you evaluate performance or latency?

A strong testbench plan makes the final implementation much easier.

### 3. Development Steps

Provide an incremental build plan:

- Break the project into **small, testable milestones**.
- Identify unit tests for each module.
- Describe integration steps.

This plan should be detailed enough that an AI assistant could help you implement each step.

### Submitting the Detailed Plan

As before:

1. Commit `detailed_plan.md` or some similar document to your GitHub repo.
2. Copy the GitHub URL path.
3. Submit the URL to the LLM grader.

The grader will evaluate the completeness and clarity of your specification and provide feedback before you begin implementation.

---

By following this planning process, you will create a clear, structured roadmap for your project — one that supports effective collaboration, AI‑assisted development, and a smooth path to your final implementation.

---

Go to [final submission](./final.md)