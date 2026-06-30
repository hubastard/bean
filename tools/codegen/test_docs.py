"""Tests for SDK documentation extraction."""

from __future__ import annotations

import subprocess
import unittest

from bean.docs import load_framework_docs


def sdk_path() -> str:
    return subprocess.check_output(["xcrun", "--show-sdk-path"], text=True).strip()


class DocsTests(unittest.TestCase):
    def test_appkit_property_doc(self) -> None:
        docs = load_framework_docs(sdk_path(), "AppKit")
        text = docs.property_doc("NSWindow", "canHide")
        self.assertIsNotNone(text)
        assert text is not None
        self.assertIn("hidden", text.lower())

    def test_foundation_class_doc(self) -> None:
        docs = load_framework_docs(sdk_path(), "Foundation")
        self.assertGreater(len(docs.classes) + len(docs.methods) + len(docs.properties), 0)


if __name__ == "__main__":
    unittest.main()
