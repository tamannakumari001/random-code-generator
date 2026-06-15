"""
Phase 0: Balanced Parentheses Generator

This file is intentionally small and playful. It demonstrates how a
context-free grammar can generate recursive structure.

Grammar:

    S -> ""
    S -> "(" S ")" S

Meaning:

    A balanced-parentheses string is either empty, or it is:
    - an opening parenthesis
    - another balanced-parentheses string
    - a closing parenthesis
    - followed by another balanced-parentheses string

Examples:

    ""
    "()"
    "(())"
    "()()"
    "(()())"
"""

from __future__ import annotations

import argparse
import random


def generate_balanced(max_depth: int, rng: random.Random) -> str:
    """Generate a balanced-parentheses string using the grammar above."""
    if max_depth <= 0:
        return ""

    # Choose the empty production sometimes so recursion eventually stops.
    should_stop = rng.choice([True, False])
    if should_stop:
        return ""

    inside = generate_balanced(max_depth - 1, rng)
    after = generate_balanced(max_depth - 1, rng)
    return f"({inside}){after}"


def is_balanced(text: str) -> bool:
    """Check whether a string contains properly balanced parentheses."""
    depth = 0

    for char in text:
        if char == "(":
            depth += 1
        elif char == ")":
            depth -= 1

        if depth < 0:
            return False

    return depth == 0


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate random balanced-parentheses strings."
    )
    parser.add_argument("--seed", type=int, default=None, help="Random seed.")
    parser.add_argument("--count", type=int, default=10, help="Number of strings.")
    parser.add_argument(
        "--max-depth",
        type=int,
        default=10,
        help="Maximum recursive generation depth.",
    )
    args = parser.parse_args()

    rng = random.Random(args.seed)

    for index in range(args.count):
        generated = generate_balanced(args.max_depth, rng)
        verdict = "ok" if is_balanced(generated) else "bad"
        shown = generated if generated else "<empty>"
        print(f"{index + 1:02}. {shown}  [{verdict}]")


if __name__ == "__main__":
    main()
