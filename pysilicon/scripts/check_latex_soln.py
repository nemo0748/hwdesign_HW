import argparse
from pathlib import Path
from xilinxutils.parselatex import load_schema, check_soln_core, parse_latex_soln  

def check_soln(soln_path: str, schema_path: str, output_path: str) -> None:
    """Wrapper that loads schema, parses LaTeX, and writes summary."""
    schema = load_schema(schema_path)
    parsed_items = parse_latex_soln(soln_path)
    check_soln_core(schema, parsed_items, output_path)


def main():
    parser = argparse.ArgumentParser(
        description="Check a LaTeX solution file against a schema and produce a parsed summary."
    )

    parser.add_argument(
        "solution",
        type=str,
        help="Path to the student's LaTeX solution file (e.g., my_soln.tex)"
    )

    parser.add_argument(
        "schema",
        type=str,
        help="Path to the schema CSV file (e.g., schema.csv)"
    )

    parser.add_argument(
        "-o", "--output",
        type=str,
        default="parsed.txt",
        help="Output file for the parsed summary (default: parsed.txt)"
    )

    args = parser.parse_args()

    check_soln(args.solution, args.schema, args.output)
    print(f"Parsed summary written to {args.output}")