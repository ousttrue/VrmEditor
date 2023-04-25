import pathlib

HERE = pathlib.Path(__file__).parent


def main():
    for f in HERE.parent.rglob("*~"):
        if f.is_file():
            print(f)
            f.unlink()


if __name__ == "__main__":
    main()
