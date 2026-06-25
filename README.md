# Random Code Generator

A small project that touches on automata theory, formal languages, and compiler front-end concepts. If you're new to any of the underlying theory, the links below should get you up to speed before diving into the code.

## How to Run Each Phase

Requirements: Python 3 and a C++17 compiler such as `g++`.

Run all commands from the project root.

### Phase 0: Grammar Examples

```bash
python3 phase_0/balanced_parentheses.py --seed 1 --count 5 --max-depth 6
python3 phase_0/arithmetic.py --seed 1 --count 5 --depth 3
```

This phase shows simple random generation for balanced parentheses and arithmetic expressions.

### Phase 1: Simple Statements and AST

```bash
python3 phase_1/simple.py
g++ -std=c++17 -Wall -Wextra -pedantic phase_1/ast.cpp -o /tmp/phase1_ast
/tmp/phase1_ast
```

This phase generates simple assignments and print statements, then introduces an AST-based C++ generator. To check Phase 1 progress, run:

```bash
python3 phase_1/test_simple.py
```

### Phase 2: Symbol Table

```bash
g++ -std=c++17 -Wall -Wextra -pedantic phase_2/symbol_table.cpp -o /tmp/phase2_symbol_table
/tmp/phase2_symbol_table
```

This phase tracks declared variables so generated code can use names more safely.

### Phase 3: Loops and Conditionals

```bash
./phase_3/run_generator.sh 73 --statements 10 --expr-depth 3 --control-depth 2
python3 -m py_compile phase_3/code.py
```

This phase generates code with assignments, prints, boolean expressions, comparisons, `if`, and `while`.

### Phase 4: Functions, Calls, and Lists

```bash
./phase_4/run_generator.sh 73 --functions 5 --statements 8 --expr-depth 3 --control-depth 2
python3 -m py_compile phase_4/code.py
```

This phase adds function definitions, function calls, return statements, strings, lists, and typed values. To run generated code, use a timeout because random programs can contain long or endless loops:

```bash
timeout 5 python3 phase_4/code.py
```

## Learning Resources

### 1. DFA & Regular Languages (start here)

Stanford CS103 — *Mathematical Foundations of Computing* (Reingold-style intro):

- [Finite Automata, Part I](https://web.stanford.edu/class/archive/cs/cs103/cs103.1266/lectures/14/Lecture%20Slides.pdf)
- [Finite Automata, Part II](https://web.stanford.edu/class/archive/cs/cs103/cs103.1266/lectures/15/Lecture%20Slides.pdf)
- [Finite Automata, Part III](https://web.stanford.edu/class/archive/cs/cs103/cs103.1266/lectures/16/Lecture%20Slides.pdf)
- [Regular Expressions](https://web.stanford.edu/class/archive/cs/cs103/cs103.1266/lectures/17/Lecture%20Slides.pdf)
- [Nonregular Languages (Pumping Lemma)](https://web.stanford.edu/class/archive/cs/cs103/cs103.1266/lectures/18/Lecture%20Slides.pdf)

Bonus — Omer Reingold's CS154 slide decks (cleaner standalone DFA/NFA proofs):

- [DFA overview](https://omereingold.wordpress.com/wp-content/uploads/2020/09/5p-dfa-overview.pdf)
- [DFAs](https://omereingold.wordpress.com/wp-content/uploads/2020/09/6p-dfas.pdf)
- [Closure properties of regular languages](https://omereingold.wordpress.com/wp-content/uploads/2020/09/7p-dfaclosure1.pdf)
- [NFAs](https://omereingold.wordpress.com/wp-content/uploads/2020/09/8p-nfas.pdf)
- [NFA → DFA](https://omereingold.wordpress.com/wp-content/uploads/2020/09/9p-nfa2dfa.pdf)
- [Regular expressions](https://omereingold.wordpress.com/wp-content/uploads/2020/09/11p-regexp.pdf)
- [Pumping lemma](https://omereingold.wordpress.com/wp-content/uploads/2020/09/12p-pumping.pdf)
- [Minimizing DFAs](https://omereingold.wordpress.com/wp-content/uploads/2020/09/13p-minimizingdfa.pdf)
- [Myhill–Nerode](https://omereingold.wordpress.com/wp-content/uploads/2020/09/14p-myhill-nerode.pdf)

Once you can build a DFA for "strings containing `01`" and convert an NFA to a DFA on paper, move on.

### 2. Context-Free Grammars (CFGs)

- [Stanford CS103 — Context-Free Languages](https://web.stanford.edu/class/archive/cs/cs103/cs103.1266/lectures/19/Lecture%20Slides.pdf) — high-level intro.
- [UIUC CS373 — Context-Free Languages and Ambiguity (lec12)](https://courses.engr.illinois.edu/cs373/sp2013/Lectures/lec12.pdf)
- [UIUC CS373 — Grammar Simplification (lec14)](https://courses.engr.illinois.edu/cs373/sp2013/Lectures/lec14.pdf)
- [UIUC CS373 — Normal Forms & Closure (lec15)](https://courses.engr.illinois.edu/cs373/sp2013/Lectures/lec15.pdf)
- [Stanford CS143 — Introduction to Parsing (lecture05)](https://web.stanford.edu/class/cs143/lectures/lecture05.pdf) — CFGs from a compiler's perspective.

Practice: write a CFG for balanced parentheses, then for simple arithmetic with `+` and `*` respecting precedence.

### 3. Pushdown Automata (PDA)

Stanford's CS103 and the current CS154 don't publish PDA slides — they skip from CFGs to Turing machines. UIUC CS373 has the cleanest direct slide deck on PDAs I've found:

- [UIUC CS373 — Pushdown Automata (lec13)](https://courses.engr.illinois.edu/cs373/sp2013/Lectures/lec13.pdf)
- [UIUC CS373 — Pumping Lemma and non-CFLs (lec17)](https://courses.engr.illinois.edu/cs373/sp2013/Lectures/lec17.pdf)

### 4. Nondeterministic PDAs (NPDA)

The PDA slide deck above already presents the model nondeterministically — NPDAs are the *default* PDA model, and they are strictly more powerful than DPDAs (NPDAs recognize all CFLs; DPDAs only recognize the deterministic CFLs).

- [UIUC CS373 — Pushdown Automata (lec13)](https://courses.engr.illinois.edu/cs373/sp2013/Lectures/lec13.pdf) — see the nondeterministic transition definition and CFG↔PDA equivalence.
- [UIUC CS373 — Chomsky Hierarchy (lec18)](https://courses.engr.illinois.edu/cs373/sp2013/Lectures/lec18.pdf) — situates NPDAs against DFAs and TMs.

Reference for DPDA vs NPDA separation: <https://en.wikipedia.org/wiki/Deterministic_pushdown_automaton>.

### 5. Symbol Tables

Once you're past recognition and into actually compiling/interpreting code, you need a symbol table to track identifiers, scopes, and types.

- [Stanford CS143 — Lexical Analysis (lecture03)](https://web.stanford.edu/class/cs143/lectures/lecture03.pdf)
- [Stanford CS143 — Implementation of Lexical Analysis (lecture04)](https://web.stanford.edu/class/cs143/lectures/lecture04.pdf)
- [Stanford CS143 — Top-Down Parsing (lecture07)](https://web.stanford.edu/class/cs143/lectures/lecture07.pdf)
- [Stanford CS143 — Bottom-Up Parsing (lecture08)](https://web.stanford.edu/class/cs143/lectures/lecture08.pdf)
- [Stanford CS143 — Semantic Analysis & Type Checking I (lecture09)](https://web.stanford.edu/class/cs143/lectures/lecture09.pdf) — symbol tables and scopes are introduced here.
- [Stanford CS143 — Type Checking II (lecture10)](https://web.stanford.edu/class/cs143/lectures/lecture10.pdf)
