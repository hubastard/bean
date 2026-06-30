"""Tests for enum constant extraction and value resolution."""

from __future__ import annotations

import subprocess
import unittest

from bean.parser import ParsedEnum, ParsedEnumConstant, dedupe_enums, parse_header, _resolve_enum_constant_values


def sdk_path() -> str:
    return subprocess.check_output(["xcrun", "--show-sdk-path"], text=True).strip()


class EnumValueTests(unittest.TestCase):
    def test_ns_comparison_result_values(self) -> None:
        parsed = parse_header(sdk_path(), "Foundation/Foundation.h", copy_result=True)
        enums = {enum.name: enum for enum in dedupe_enums(parsed.enums)}
        comparison = enums["NSComparisonResult"]
        values = {constant.name: constant.value for constant in comparison.constants}
        self.assertEqual(values["NSOrderedAscending"], "-1")
        self.assertEqual(values["NSOrderedSame"], "0")
        self.assertEqual(values["NSOrderedDescending"], "1")

    def test_ek_authorization_status_values(self) -> None:
        parsed = parse_header(sdk_path(), "EventKit/EventKit.h", copy_result=True)
        enums = {enum.name: enum for enum in dedupe_enums(parsed.enums)}
        status = enums["EKAuthorizationStatus"]
        values = {constant.name: constant.value for constant in status.constants}
        self.assertEqual(values["EKAuthorizationStatusNotDetermined"], "0")
        self.assertEqual(values["EKAuthorizationStatusRestricted"], "1")
        self.assertEqual(values["EKAuthorizationStatusDenied"], "2")
        self.assertEqual(values["EKAuthorizationStatusFullAccess"], "3")
        self.assertEqual(values["EKAuthorizationStatusWriteOnly"], "4")
        self.assertEqual(values["EKAuthorizationStatusAuthorized"], "3")

    def test_sequential_fill_from_scratch(self) -> None:
        enum = ParsedEnum(
            name="SampleStatus",
            underlying="NSInteger",
            constants=[
                ParsedEnumConstant(name="SampleFirst", value="0"),
                ParsedEnumConstant(name="SampleSecond", value=None),
                ParsedEnumConstant(name="SampleThird", value=None),
            ],
        )
        _resolve_enum_constant_values(enum)
        values = {constant.name: constant.value for constant in enum.constants}
        self.assertEqual(values["SampleFirst"], "0")
        self.assertEqual(values["SampleSecond"], "1")
        self.assertEqual(values["SampleThird"], "2")


if __name__ == "__main__":
    unittest.main()
