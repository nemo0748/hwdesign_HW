---
title: AI workflow
parent: Project
nav_order: 1
has_children: false
---


## AI‑Development Workflow

One of the goals of the project is to help you become comfortable with a modern AI‑augmented engineering workflow. AI tools excel at **well‑defined, bounded tasks**, but they can produce incorrect or incoherent designs when used without structure or when the problem is underspecified. The key is to treat AI as a **collaborator**, not an oracle — you provide the architecture and constraints, and the AI helps you refine, implement, and test your ideas.

We will follow a methodology that mirrors what is rapidly becoming standard practice in top hardware and systems companies:

- **Plan with AI** to explore functionality, clarify requirements, and outline the high‑level architecture.  
  AI is especially good at helping you brainstorm design options, compare trade‑offs, and refine your initial concept into a coherent plan.

- **Define submodules, interfaces, and the testbench** in a structured way.  
  This includes specifying message formats, control signals, memory interfaces, and verification strategy.  
  AI can help you check for missing pieces, inconsistencies, or unclear assumptions.

- **Create an incremental build plan with unit testing.**  
  Break the design into small, testable steps.  
  AI can help you generate scaffolding code, test vectors, and sanity checks as you go.

- **Iterate with feedback from the LLM grader.**  
  Each submission (initial plan, detailed plan, final submission) is evaluated by the LLM grader, which provides structured, rubric‑based feedback.  
  This mirrors the iterative review cycles used in real engineering teams.

- **Use AI to assist with implementation, not replace your understanding.**  
  You remain responsible for the correctness of your design.  
  AI can help you write RTL, testbenches, and documentation — but only if you give it a clear, well‑structured plan.

This workflow is designed to make you faster, more organized, and more effective as a hardware designer.  
By the end of the project, you should feel comfortable using AI as a powerful engineering tool — one that amplifies your abilities rather than substituting for them.

-- 
Get started by going to  [planning](./planning.md)