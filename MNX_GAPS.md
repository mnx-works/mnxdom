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

## Legacy validator support for Draft 2020-12

The default json-schema-cpp backend enforces the vendored MNX schema's
`unevaluatedProperties` rules and the notehead custom-shape/glyph dependency.
Builds using `MNXDOM_USE_LEGACY_SCHEMA_VALIDATOR` use the Draft 7 validator. It
ignores `unevaluatedProperties` and `dependentSchemas`, so a stray property or a
custom notehead shape without a glyph can pass schema validation in those
builds.
