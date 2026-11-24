import unittest

from line_count import count_lines


class CountLinesTest(unittest.TestCase):
    def test_empty_string(self):
        self.assertEqual(count_lines(""), 0)

    def test_single_line_without_trailing_newline(self):
        self.assertEqual(count_lines("abc"), 1)

    def test_single_line_with_trailing_newline(self):
        self.assertEqual(count_lines("abc\n"), 1)

    def test_multiple_lines_without_trailing_newline(self):
        self.assertEqual(count_lines("a\nb\nc"), 3)

    def test_multiple_lines_with_trailing_newline(self):
        self.assertEqual(count_lines("a\nb\nc\n"), 3)


if __name__ == "__main__":
    unittest.main()
