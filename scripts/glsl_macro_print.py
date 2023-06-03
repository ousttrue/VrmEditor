from typing import Optional
import pathlib
import sys
import re

INCLUDE_QUOTE = re.compile(r'^include\s*"([^"]*)"')
INCLUDE_LESS_THAN = re.compile(r'^include\s*<([^"]*)>')


class Printer:
    def __init__(
        self, chunk_root: Optional[pathlib.Path], *, print_sampler=True
    ) -> None:
        self.chunk_root = chunk_root
        self.root = None
        self.used = set()
        self.print_sampler = print_sampler

    def print_macro(self, msg: str):
        if not self.print_sampler:
            print(msg)

    def print_line(self, msg: str):
        if self.print_sampler:
            if re.match(r'^\s*uniform\s+sampler', msg):
                print(msg)

    def process(self, path: pathlib.Path, indent=""):
        if not self.root:
            self.root = path.parent

        if path.exists():
            lines = path.read_text().splitlines()
        elif self.chunk_root:
            chunk_path = pathlib.Path(self.chunk_root / (path.name + ".glsl.js"))
            lines = chunk_path.read_text().splitlines()[1:-1]
        else:
            assert False

        for l in lines:
            l = l.strip()
            if len(l) == 0:
                continue

            self.print_line(l)
            if l[0] != "#":
                continue

            l = l[1:].strip()
            if l.split()[0] in ["endif", "else", "undef"]:
                continue

            if l.startswith("include"):
                # print(l)
                m = INCLUDE_QUOTE.match(l)
                if m:
                    self.print_macro(
                        f"{indent}{path.relative_to(self.root)} => {m.group(1)}"
                    )
                    self.process(path.parent / m.group(1), indent + "  ")
                    continue
                m = INCLUDE_LESS_THAN.match(l)
                if m:
                    self.print_macro(
                        f"{indent}{path.relative_to(self.root)} => {m.group(1)}"
                    )
                    self.process(path.parent / m.group(1), indent + "  ")
                    continue

                assert False
            else:
                if l in self.used:
                    pass
                else:
                    self.print_macro(f"{indent}{path.relative_to(self.root)}: #{l}")
                    self.used.add(l)


if __name__ == "__main__":
    if len(sys.argv) == 1:
        print(f"usage: {sys.argv[0]} GLSL_FILE")
        sys.exit()

    p = Printer(pathlib.Path(sys.argv[2]) if len(sys.argv) >= 3 else None)
    p.process(pathlib.Path(sys.argv[1]))
