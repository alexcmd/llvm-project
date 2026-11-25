"""Small line counting helper for tooling."""


def count_lines(text: str) -> int:
    """Return the number of lines in ``text``.

    Rules:
      * An empty string has 0 lines.
      * Lines are separated by ``\\n`` only (``\\r`` and other separators
        are not treated as line breaks).
      * A trailing ``\\n`` does not start an additional line.
    """
    if text == "":
        return 0
    count = text.count("\n")
    if not text.endswith("\n"):
        count += 1
    return count
