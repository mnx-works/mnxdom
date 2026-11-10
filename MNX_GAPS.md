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

## Unknown properties in schema validation

The current nlohmann/json-schema-validator dependency implements Draft 7 and
ignores the Draft 2020-12 `unevaluatedProperties` keyword used by MNX. As a
result, `schemaValidate()` does not reject unknown properties on objects that
use that keyword. Full enforcement needs a validator with Draft 2020-12 support or
a deliberate compatibility layer; the vendored schema should remain intact.

## Notehead shape/glyph dependencies in schema validation

MNX schema version 52 uses `dependentRequired` to require an explicit `shape`
when a notehead supplies `glyph`, and `dependentSchemas` to require `glyph`
when `shape` is `custom`. The current Draft 7 validator silently ignores both
keywords, so `schemaValidate()` does not reject either missing dependency.
The DOM remains permissive, and mnxdom does not implement these schema rules
in its semantic validator or translate the schema locally.

Creating a notehead in mnxdom requires an explicit shape constructor argument,
including `NoteheadShape::Default`, and serializes it. The property itself
remains optional with a default: reading `shape()` from an existing notehead
without the property returns `NoteheadShape::Default` without changing the JSON.

This is related to the upstream Draft 2020-12 support work in
[json-schema-validator issue #385](https://github.com/pboettch/json-schema-validator/issues/385),
but that proposal focuses on `$ref` siblings and `unevaluatedProperties` and
explicitly excludes `dependentSchemas`. Enforcement requires upstream support
for both dependency keywords; the vendored MNX schema should remain intact.
