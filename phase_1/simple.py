import random

vars = ["lol","damn","bruh","shit", "gay"]

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
        c = rng.choice([1, 2, 3,4])
        if c == 1:
            return str(random_positive_integer(rng))
        elif c == 2:
            left = generate_expression(depth -1,rng, "Exp")
            right = generate_expression(depth -1,rng, "Exp")
            op = generate_expression(1,rng,"op")
            return left + op + right
        elif c == 3:
            return rng.choice(vars)
        else:
            inside = generate_expression(depth-1,rng,"Exp")
            return f'({inside})'
    if type == "op":
        c = rng.choice([1,0])
        ops = [' + ',' - ']
        return ops[c]  
    return ""
        

def stmt_handler(rng) -> str:
    c = rng.choice([1,2])
    if c == 1:
        name = rng.choice(vars)
        exp = generate_expression(10,rng, "Exp")
        return name + ' = ' + exp
    else:
        name = rng.choice(vars)
        return f"print({name})"
rng = random.Random(10)  
for i in range(10):
    
    print(stmt_handler(rng))
