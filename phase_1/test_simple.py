"""
Phase 1 tester for simple.py.

Run from the repository root:

    python3 phase_1/test_simple.py

This tester checks whether simple.py is moving toward the Phase 1 goal:

    grammar idea -> AST-shaped implementation -> pretty-printed MiniPython
    -> generated code compiles

It intentionally reports multiple failures in one run so you can fix them
one by one.
"""

from __future__ import annotations

import ast
import contextlib
import importlib.util
import io
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


ROOT = Path(__file__).resolve().parent
SIMPLE_PATH = ROOT / "simple.py"


@dataclass
class CheckResult:
    name: str
    passed: bool
    message: str


def pass_check(name: str, message: str) -> CheckResult:
    return CheckResult(name, True, message)


def fail_check(name: str, message: str) -> CheckResult:
    return CheckResult(name, False, message)


def read_simple_source() -> str:
    return SIMPLE_PATH.read_text(encoding="utf-8")


def parse_simple_source() -> ast.Module:
    return ast.parse(read_simple_source(), filename=str(SIMPLE_PATH))


def check_file_exists() -> CheckResult:
    if SIMPLE_PATH.exists():
        return pass_check("simple.py exists", f"found {SIMPLE_PATH}")
    return fail_check("simple.py exists", "phase_1/simple.py was not found")


def check_python_syntax() -> CheckResult:
    try:
        parse_simple_source()
    except SyntaxError as error:
        return fail_check("Python syntax", f"syntax error: {error}")
    return pass_check("Python syntax", "simple.py can be parsed by Python")


def check_required_functions() -> CheckResult:
    tree = parse_simple_source()
    functions = {
        node.name
        for node in tree.body
        if isinstance(node, ast.FunctionDef)
    }
    required = {"generate_expression", "stmt_handler"}
    missing = sorted(required - functions)

    if missing:
        return fail_check(
            "Required functions",
            "missing function(s): " + ", ".join(missing),
        )

    return pass_check(
        "Required functions",
        "found generate_expression(...) and stmt_handler(...)",
    )


def check_ast_node_definitions() -> CheckResult:
    tree = parse_simple_source()
    classes = {
        node.name
        for node in tree.body
        if isinstance(node, ast.ClassDef)
    }
    expected = {"Program", "Assign", "Print", "IntLiteral", "Variable", "BinaryOp"}
    missing = sorted(expected - classes)

    if missing:
        return fail_check(
            "Phase 1 AST nodes",
            "missing AST class(es): "
            + ", ".join(missing)
            + ". Phase 1 should generate an AST before printing code.",
        )

    return pass_check("Phase 1 AST nodes", "all expected AST classes are present")


def check_no_import_side_effects() -> CheckResult:
    tree = parse_simple_source()

    side_effect_nodes = []
    for node in tree.body:
        if isinstance(node, (ast.For, ast.While)):
            side_effect_nodes.append((node.lineno, type(node).__name__))
        elif isinstance(node, ast.Expr) and isinstance(node.value, ast.Call):
            side_effect_nodes.append((node.lineno, "top-level function call"))

    if side_effect_nodes:
        locations = ", ".join(
            f"line {line}: {kind}" for line, kind in side_effect_nodes
        )
        return fail_check(
            "Import safety",
            "top-level execution found at " + locations
            + ". Put demo code under if __name__ == '__main__'.",
        )

    return pass_check("Import safety", "no obvious top-level demo loop found")


def import_simple_module() -> tuple[object | None, str, BaseException | None]:
    spec = importlib.util.spec_from_file_location("phase_1_simple_under_test", SIMPLE_PATH)
    if spec is None or spec.loader is None:
        return None, "", RuntimeError("could not create import spec")

    module = importlib.util.module_from_spec(spec)
    captured_stdout = io.StringIO()

    try:
        with contextlib.redirect_stdout(captured_stdout):
            spec.loader.exec_module(module)
    except BaseException as error:
        return None, captured_stdout.getvalue(), error

    return module, captured_stdout.getvalue(), None


def check_importable() -> CheckResult:
    _module, stdout, error = import_simple_module()

    if error is not None:
        return fail_check(
            "Import simple.py",
            f"import raised {type(error).__name__}: {error}",
        )

    if stdout:
        return fail_check(
            "Import simple.py",
            "import printed output. Modules should define functions/classes without running the generator.",
        )

    return pass_check("Import simple.py", "module imports cleanly with no output")


def check_generate_expression_callable() -> CheckResult:
    module, _stdout, error = import_simple_module()

    if error is not None:
        return fail_check(
            "generate_expression callable",
            f"cannot test because import failed with {type(error).__name__}: {error}",
        )

    try:
        expression = module.generate_expression(4, module.random.Random(7), "Exp")
        compile(expression, "<generated expression>", "eval")
    except Exception as error:
        return fail_check(
            "generate_expression callable",
            f"generated expression failed: {type(error).__name__}: {error}",
        )

    return pass_check(
        "generate_expression callable",
        f"generated valid Python expression: {expression}",
    )


def run_simple_script() -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        [sys.executable, str(SIMPLE_PATH)],
        check=False,
        capture_output=True,
        text=True,
        timeout=5,
    )


def check_script_runs() -> CheckResult:
    try:
        result = run_simple_script()
    except subprocess.TimeoutExpired:
        return fail_check("Run simple.py", "script timed out after 5 seconds")

    if result.returncode != 0:
        message = result.stderr.strip() or result.stdout.strip()
        return fail_check("Run simple.py", message)

    lines = [line for line in result.stdout.splitlines() if line.strip()]
    if not lines:
        return fail_check("Run simple.py", "script ran but produced no generated code")

    return pass_check("Run simple.py", f"script produced {len(lines)} line(s)")


def is_allowed_expression(node: ast.AST) -> bool:
    if isinstance(node, ast.Constant):
        return isinstance(node.value, int)

    if isinstance(node, ast.Name):
        return node.id.isidentifier()

    if isinstance(node, ast.BinOp):
        allowed_operator = isinstance(node.op, (ast.Add, ast.Sub, ast.Mult))
        return (
            allowed_operator
            and is_allowed_expression(node.left)
            and is_allowed_expression(node.right)
        )

    return False


def is_allowed_statement(node: ast.stmt) -> bool:
    if isinstance(node, ast.Assign):
        if len(node.targets) != 1:
            return False
        target = node.targets[0]
        return isinstance(target, ast.Name) and is_allowed_expression(node.value)

    if isinstance(node, ast.Expr) and isinstance(node.value, ast.Call):
        call = node.value
        return (
            isinstance(call.func, ast.Name)
            and call.func.id == "print"
            and len(call.args) == 1
            and not call.keywords
            and is_allowed_expression(call.args[0])
        )

    return False


def check_output_matches_minipython() -> CheckResult:
    try:
        result = run_simple_script()
    except subprocess.TimeoutExpired:
        return fail_check("MiniPython grammar", "script timed out after 5 seconds")

    if result.returncode != 0:
        message = result.stderr.strip() or result.stdout.strip()
        return fail_check("MiniPython grammar", "script failed before grammar check: " + message)

    generated_code = result.stdout

    try:
        parsed = ast.parse(generated_code, filename="<generated simple.py output>")
    except SyntaxError as error:
        return fail_check("MiniPython grammar", f"generated code has syntax error: {error}")

    bad_lines = [
        getattr(statement, "lineno", "?")
        for statement in parsed.body
        if not is_allowed_statement(statement)
    ]

    if bad_lines:
        return fail_check(
            "MiniPython grammar",
            "generated statement(s) outside Phase 1 grammar at line(s): "
            + ", ".join(map(str, bad_lines)),
        )

    return pass_check(
        "MiniPython grammar",
        "generated output only uses assignment, print, int, var, +, -, *",
    )


def check_generated_code_compiles() -> CheckResult:
    try:
        result = run_simple_script()
    except subprocess.TimeoutExpired:
        return fail_check("Generated code compiles", "script timed out after 5 seconds")

    if result.returncode != 0:
        message = result.stderr.strip() or result.stdout.strip()
        return fail_check(
            "Generated code compiles",
            "script failed before compile check: " + message,
        )

    try:
        compile(result.stdout, "<generated simple.py output>", "exec")
    except SyntaxError as error:
        return fail_check("Generated code compiles", f"compile failed: {error}")

    return pass_check("Generated code compiles", "generated output compiles as Python")


def check_generated_output_has_variety() -> CheckResult:
    try:
        result = run_simple_script()
    except subprocess.TimeoutExpired:
        return fail_check("Generated output variety", "script timed out after 5 seconds")

    if result.returncode != 0:
        message = result.stderr.strip() or result.stdout.strip()
        return fail_check(
            "Generated output variety",
            "script failed before variety check: " + message,
        )

    lines = [line for line in result.stdout.splitlines() if line.strip()]
    unique_lines = set(lines)

    if len(lines) > 1 and len(unique_lines) == 1:
        return fail_check(
            "Generated output variety",
            "all generated lines are identical; check RNG seeding and random choices",
        )

    return pass_check(
        "Generated output variety",
        f"{len(unique_lines)} unique line(s) out of {len(lines)} generated line(s)",
    )


def run_checks() -> list[CheckResult]:
    checks = [
        check_file_exists,
        check_python_syntax,
        check_required_functions,
        check_ast_node_definitions,
        check_no_import_side_effects,
        check_importable,
        check_generate_expression_callable,
        check_script_runs,
        check_output_matches_minipython,
        check_generated_code_compiles,
        check_generated_output_has_variety,
    ]

    results = []
    for check in checks:
        try:
            results.append(check())
        except Exception as error:
            results.append(
                fail_check(
                    check.__name__,
                    f"tester crashed during this check: {type(error).__name__}: {error}",
                )
            )

    return results


def main() -> int:
    results = run_checks()
    passed = sum(result.passed for result in results)
    failed = len(results) - passed

    print("Phase 1 simple.py tester")
    print("=" * 28)

    for result in results:
        marker = "PASS" if result.passed else "FAIL"
        print(f"[{marker}] {result.name}: {result.message}")

    print("=" * 28)
    print(f"Passed: {passed}")
    print(f"Failed: {failed}")

    return 0 if failed == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
