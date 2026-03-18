---
title: Final Submission
parent: Project
nav_order: 2
has_children: false
---

# Final Submission

## Submission Format

Similar to the [planning steps](./planning.md), simply submit the link to your GitHub repository to the LLM grader.  
The GitHub repo should contain **all documentation, code, and results** needed for the grader to evaluate your project.

The LLM grader can only assess what it can read directly from your repository, so your repo must include:

- Clear, well‑organized Markdown files (`initial_plan.md`, `detailed_plan.md`, `README.md`, etc.)  
- All RTL/HLS source code and testbench files  
- Simulation outputs, logs, or result summaries  
- Any diagrams, tables, or explanations referenced in your write‑up  
- A structure that makes it easy to navigate and understand your design

If someone — **or an automated tool like the LLM grader** — opened your repository, they should be able to understand your design, reproduce your results, and follow your reasoning without needing anything outside the repo.

# Final Grading (25 points total)

The LLM grader will provide an **initial assessment** and
the instructor will make the **final determination** for all items.

---

## 1. IP Definition and Interface (6 points)

Evaluates whether the final implementation matches a clear, well‑defined specification.

- Is the purpose of the IP clearly defined and appropriate for the chosen task  
- Are any mathematical operations clearly defined  
- Is the hardware/software boundary well‑justified  
- Are the interfaces (AXI, streaming, custom, etc.) clearly documented  
- Have the initial planning documents been updated to reflect the final design  

It is completely acceptable if your design deviated from the original plan — that is the nature of research and learning. What matters is that the final documentation clearly explains the design you actually built.
---

## 2. Architecture and Implementation Quality (8 points)


This category emphasizes **architectural decisions** and the quality of the implementation.

- Is the architecture coherent, modular, and well‑reasoned 
- Are sub‑modules clearly defined with limited functionality in each module  
- Are communication paths and dataflow well‑designed  
- Are the control and configuration of each module sufficiently general?
- Are architectural choices (pipelining, buffering, parallelism, memory layout, etc.) justified   
- Is the code readable, maintainable, and appropriately commented  

Again, these documents can follow updated versions of the original planning documents.
---

## 3. Evaluation and Verification (6 points)

Assesses the rigor and completeness of the testbench and evaluation.

- Is the testbench comprehensive and does it cover corner cases  
- Are correctness checks clearly defined and implemented  
- Are performance, latency, or resource usage measured and interpreted  
- Are results reproducible from the repository  
- Does the evaluation provide automatable and reproducible tests
  
---

## 4. Presentation (5 points)

Includes both the **in‑class presentation** and the **repository presentation**.
Presentations can be in any form you want:  slides, readme, notebook, or PDF.

- Clear explanation of the problem, design, and results
- Does the presentation quickly provide key aspects of the design while also providing sufficient deatils  
- Are key metrics highlighted
