"""Compare field-driven heating with OpenFOAM's wall-boiling boundary condition."""

from pathlib import Path
import re

import numpy as np


def times(case):
    return sorted(
        (
            p.name
            for p in Path(case).iterdir()
            if p.is_dir() and re.fullmatch(r"[0-9.]+", p.name) and float(p.name) > 0
        ),
        key=float,
    )


def patch_entry(case, time, field, entry):
    """Read a scalar entry from the written wall patch (including mixed coefficients)."""
    text = (Path(case) / time / field).read_text()
    start = re.search(r"\bwall\s*\{", text).end()
    depth = 1
    end = start
    while depth:
        depth += (text[end] == "{") - (text[end] == "}")
        end += 1
    text = text[start : end - 1]
    uniform = re.search(rf"\b{entry}\s+uniform\s+([^;]+);", text)
    if uniform:
        return np.array([float(uniform[1])])
    values = re.search(
        rf"\b{entry}\s+nonuniform\s+List<scalar>\s+(\d+)\s*\((.*?)\)\s*;", text, re.S
    )
    if not values:
        raise AssertionError(f"Missing {entry} in {case}/{time}/{field}:wall")
    result = np.fromstring(values[2], sep=" ")
    if result.size != int(values[1]) or not result.size:
        raise AssertionError("Incomplete patch data")
    return result
