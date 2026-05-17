# Random Code Generator

A small project that touches on automata theory, formal languages, and compiler front-end concepts. If you're new to any of the underlying theory, the links below should get you up to speed before diving into the code.

## Learning Resources

### 1. DFA & Regular Languages (start here)

If you've never seen finite automata before, start with these:

- **Stanford CS103 — Mathematical Foundations of Computing**
  Course page: <https://web.stanford.edu/class/cs103/>
  Look under the "Lectures" / "Handouts" sections for the DFA, NFA, and regular-language slides — they're some of the clearest introductions you'll find.
- **Hopcroft, Motwani & Ullman — *Introduction to Automata Theory, Languages, and Computation*** (the classic textbook).
  Author page with errata: <https://infolab.stanford.edu/~ullman/ialc.html>
- **Wikipedia primers** (good for definitions and examples):
  - DFA: <https://en.wikipedia.org/wiki/Deterministic_finite_automaton>
  - NFA: <https://en.wikipedia.org/wiki/Nondeterministic_finite_automaton>
  - Regular language: <https://en.wikipedia.org/wiki/Regular_language>
- **Neso Academy — Theory of Computation playlist** on YouTube (search "Neso Academy Theory of Computation"). Friendly, paced lectures for self-study.

Once you can build a DFA for "strings containing `01`" and convert an NFA to a DFA on paper, move on.

### 2. Context-Free Grammars (CFGs)

CFGs describe what regular expressions cannot — nested/balanced structure like parentheses, expression trees, JSON, etc.

- **Stanford CS143 — Compilers**, "Context-Free Grammars" and "Parsing" lectures:
  Course page: <https://web.stanford.edu/class/cs143/>
  The slide decks under "Lectures" cover CFGs, ambiguity, and derivations cleanly.
- **Stanford CS154 — Introduction to Automata and Complexity Theory**:
  <https://cs154.stanford.edu/>
- Wikipedia: <https://en.wikipedia.org/wiki/Context-free_grammar>

Practice: write a CFG for balanced parentheses, then for simple arithmetic with `+` and `*` respecting precedence.

### 3. Pushdown Automata (PDA)

A PDA = finite automaton + a stack. This is what gives it the power to recognize CFLs.

- **Stanford CS154 PDA lecture slides** (linked from the course page above).
- Wikipedia: <https://en.wikipedia.org/wiki/Pushdown_automaton>
- Hopcroft/Motwani/Ullman, Chapter 6 — the canonical treatment.

### 4. Nondeterministic PDAs (NPDA)

NPDAs are strictly more powerful than deterministic PDAs — they recognize *all* context-free languages, while DPDAs only recognize a strict subset (the deterministic CFLs).

- **Stanford CS154 slides on PDAs and CFLs** — the same deck typically introduces nondeterminism alongside PDAs:
  <https://cs154.stanford.edu/>
- Wikipedia comparison: <https://en.wikipedia.org/wiki/Deterministic_pushdown_automaton>
- Hopcroft/Motwani/Ullman, Section 6.4 (equivalence of PDAs and CFGs).

### 5. Symbol Tables

Once you're past recognition and into actually compiling/interpreting code, you need a symbol table to track identifiers, scopes, and types.

- **Stanford CS143 — "Semantic Analysis" lecture** (symbol tables, scoping):
  <https://web.stanford.edu/class/cs143/>
- **Aho, Lam, Sethi & Ullman — *Compilers: Principles, Techniques, and Tools*** (the "Dragon Book"), Chapter 2 and Chapter 7 cover symbol tables and scope management.
- Wikipedia: <https://en.wikipedia.org/wiki/Symbol_table>
- GeeksforGeeks has a concise overview — search "GeeksforGeeks symbol table compiler design".
