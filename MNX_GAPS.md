# MNX/mnxdom Gaps

This document records known gaps between the MNX specification and the
mnxdom API. It is intentionally limited to items that require either an API
design decision or clarification from the MNX community.

## Defaults for default children

`MNX_OPTIONAL_PROPERTY_WITH_DEFAULT` expresses defaults for scalar and enum
properties while preserving omission in serialized JSON. The current macro
system has no corresponding mechanism for an optional child object whose
default is itself a child value.

For example, `Tempo::location` is an optional `RhythmicPosition` child, but
MNX specifies that an omitted location means position zero. `mnxdom` cannot
represent that effective child default with the existing property macro
without deciding how a default wrapper for a child that is absent from the
JSON should behave. Until that API design is settled, callers must interpret
an absent default child according to the MNX specification.

## Dynamics `glyphs` and relative `value`

MNX schema version 41 split `dynamic-group` into four closed types. As a
result, `glyphs` is permitted only on immediate and accent dynamics, and
relative dynamics have no `value`. That conflicts with the specification's
intent: before the split, `glyphs` was described as overriding the glyphs
derived from a dynamic's value or relative value, gradual dynamics have an
optional starting `value`, and the specification's example of a relative
dynamic ("*più* p") requires a value.

`mnxdom` therefore exposes `glyphs` on every dynamic (`DynamicGroupBase`) and
an optional `value` on `DynamicRelative`. Until the upstream schema permits
them, documents that set `glyphs` on gradual or relative dynamics, or `value`
on relative dynamics, do not conform to the upstream schema.
