from typing import Optional
import pathlib
import sys
import re

INCLUDE_QUOTE = re.compile(r'^include\s*"([^"]*)"')
INCLUDE_LESS_THAN = re.compile(r'^include\s*<([^"]*)>')


class Printer:
    def __init__(self, chunk_root: Optional[pathlib.Path]) -> None:
        self.chunk_root = chunk_root
        self.root = None
        self.used = set()

    def process(self, path: pathlib.Path):
        if not self.root:
            self.root = path.parent

        # base_dir = path.parent
        # print(path)
        for l in path.read_text().splitlines():
            l = l.strip()
            if len(l) == 0:
                continue
            if l[0] != "#":
                continue

            # print(l)
            l = l[1:].strip()
            if l.split()[0] in ["endif", "else", "undef"]:
                continue

            if l.startswith("include"):
                # print(l)
                m = INCLUDE_QUOTE.match(l)
                if m:
                    print(f"{path.relative_to(self.root)} => {m.group(1)}")
                    self.process(path.parent / m.group(1))
                    continue
                m = INCLUDE_LESS_THAN.match(l)
                if m:
                    print(f"{path.relative_to(self.root)} => {m.group(1)}")
                    self.process(path.parent / m.group(1))
                    continue

                assert False
            else:
                if l in self.used:
                    pass
                else:
                    print(f"{path.relative_to(self.root)}: #{l}")
                    self.used.add(l)


if __name__ == "__main__":
    if len(sys.argv) == 1:
        print(f"usage: {sys.argv[0]} GLSL_FILE")
        sys.exit()

    p = Printer(None)
    p.process(pathlib.Path(sys.argv[1]))
