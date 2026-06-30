"""Tests for apinotes loading and merge."""

from __future__ import annotations

import subprocess
import unittest

from bean.apinotes import find_apinotes_path, load_apinotes
from bean.merge_apinotes import apply_apinotes
from bean.parser import ParsedMethod, ParseResult
from bean.typemap import qual_type_to_c


def sdk_path() -> str:
    return subprocess.check_output(["xcrun", "--show-sdk-path"], text=True).strip()


class ApinotesTests(unittest.TestCase):
    def test_foundation_apinotes_path(self) -> None:
        sdk = sdk_path()
        path = find_apinotes_path(sdk, "Foundation")
        self.assertIsNotNone(path)
        assert path is not None
        self.assertTrue(path.name.endswith("Foundation.apinotes"))

    def test_nonswift_method_filtered(self) -> None:
        sdk = sdk_path()
        notes = load_apinotes(sdk, "Foundation")
        self.assertTrue(
            notes.is_nonswift_method("NSNumber", "numberWithChar:", True),
            "expected NSNumber numberWithChar: to be nonswift in Foundation.apinotes",
        )

    def test_apply_apinotes_skips_and_overrides(self) -> None:
        sdk = sdk_path()
        notes = load_apinotes(sdk, "Foundation")
        result = ParseResult(
            methods=[
                ParsedMethod(
                    class_name="NSNumber",
                    selector="numberWithChar:",
                    is_class_method=True,
                    return_type=qual_type_to_c("id"),
                    params=[("c", qual_type_to_c("char"))],
                ),
                ParsedMethod(
                    class_name="NSAppearance",
                    selector="currentAppearance",
                    is_class_method=True,
                    return_type=qual_type_to_c("id"),
                    params=[],
                ),
            ]
        )
        merged, skipped = apply_apinotes(result, notes)
        self.assertEqual(len(skipped), 1)
        self.assertEqual(skipped[0].selector, "numberWithChar:")
        self.assertEqual(len(merged.methods), 1)


if __name__ == "__main__":
    unittest.main()
