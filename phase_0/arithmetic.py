import random
import argparse


def random_positive_integer(rng: random.Random, max_bits: int = 64) -> int:
    """Generate a random positive integer using the given RNG."""
    if max_bits < 1:
        raise ValueError("max_bits must be at least 1")
    value = rng.getrandbits(max_bits)
    return value%100 or 1


def generate_expression(depth: int, rng: random.Random, type: str) -> str:
    # generate an arithmetic expression of this depth
    if depth <= 0:
        return "1"
    if type == "Exp":
        c = rng.choice([1, 2, 3])
        if c == 1:
            return str(random_positive_integer(rng))
        elif c == 2:
            left = generate_expression(depth -1,rng, "Exp")
            right = generate_expression(depth -1,rng, "Exp")
            op = generate_expression(1,rng,"op")
            return left + op + right
        else:
            inside = generate_expression(depth-1,rng,"Exp")
            return f'({inside})'
    if type == "op":
        c = rng.choice([1,2,3,0])
        ops = [' + ',' - ',' * ', ' / ']
        return ops[c]  
    return ""


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate and test random arithmetic expressions."
    )
    parser.add_argument("--seed", type=int, default=None, help="Random seed.")
    parser.add_argument("--count", type=int, default=10, help="Number of expressions.")
    parser.add_argument(
        "--depth",
        type=int,
        default=4,
        help="Maximum recursive generation depth.",
    )
    args = parser.parse_args()

    rng = random.Random(args.seed)

    for index in range(args.count):
        expression = generate_expression(args.depth, rng, "Exp")

        try:
            compile(expression, "<generated arithmetic>", "eval")
            value = eval(expression, {"__builtins__": {}}, {})
            verdict = f"ok -> {value}"
        except Exception as error:
            verdict = f"bad -> {type(error).__name__}: {error}"

        shown = expression if expression else "<empty>"
        print(f"{index + 1:02}. {shown}  [{verdict}]")


if __name__ == "__main__":
    main()
