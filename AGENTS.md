# AGENTS.md

## Project overview

`mnxdom` is a C++ document object model for the W3C MNX music interchange
format. The public API is primarily header-based: classes wrap a shared JSON
root and a JSON pointer to their object. `src/` contains the public headers,
enum mappings, implementation files, and schema/semantic validation.

The project requires C++20 or newer and is tested with C++20 and C++23. The
default build uses CMake `FetchContent` for nlohmann/json,
json-schema-cpp (or json-schema-validator with the legacy build option), and (when tests are enabled) GoogleTest.

## Repository layout

- `src/` — public DOM headers and library implementation.
- `src/validation/` — JSON Schema and semantic validation.
- `tests/` — GoogleTest unit tests and validation fixtures.
- `third_party/w3c-mnx/` — vendored MNX schema, generated schema bytes, and
  W3C example JSON files.
- `scripts/update-w3c-mnx.sh` — maintainer script for updating the vendored
  W3C MNX snapshot.
- `.agents/update-mnx-vendored-assets/SKILL.md` — reusable workflow for
  updating and auditing vendored MNX assets, examples, and schema compatibility.
- `MNX_GAPS.md` — known MNX/mnxdom integration gaps awaiting API or
  specification decisions.
- `.agents/music-theory-sync/SKILL.md` — changes to `src/music_theory/music_theory.hpp` must be synchronized with musxdom.
- `docs/` — Doxygen configuration and documentation assets.

## Building and testing

Configure and build from the repository root:

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Use a separate build directory when changing CMake options. The project treats
compiler warnings as errors (`-Wall -Wextra -Werror -Wpedantic` on Clang/GCC;
`/W4 /WX` on MSVC), so new code must compile cleanly under those settings.

The `Examples.All` test walks every `.json` file in the vendored examples
directory. Three layout-oriented examples are intentionally schema-only:
`orchestral-layout.json`, `organ-layout.json`, and `system-layouts.json` are
illustrative layout documents that do not satisfy mnxdom's whole-document
semantic invariants. Do not remove that distinction without confirming the
upstream examples have become semantically complete.

When changing validation, add or update focused fixtures under
`tests/data/inputs/` and tests under `tests/validation/`. Keep schema
validation and semantic validation conceptually separate: schema validation
checks the embedded MNX schema, while semantic validation checks relationships
that JSON Schema cannot express.

## Coding conventions

- Match the existing C++ style and naming: PascalCase types, camelCase JSON
  concepts exposed through the existing property macros, and `snake_case`
  method names for JSON properties.
- Prefer the existing `MNX_*_PROPERTY`, child, array, and content macros when
  adding model members so accessors behave consistently.
- Preserve the lightweight-wrapper model. Return DOM objects by value; do not
  introduce references to temporary wrapper objects.
- Use `MNX_OPTIONAL_PROPERTY_WITH_DEFAULT` when the MNX specification defines a
  default. This supplies an accessor default while leaving an omitted property
  omitted from serialized JSON unless explicitly set.
- Keep enum declaration order meaningful when the zero value is the MNX
  default. Add the corresponding string entry in `src/EnumerationMaps.cpp`.
- Keep document construction permissive. Do not add setter exceptions merely
  to duplicate schema range checks; callers can construct a document and use
  `mnx::validation::schemaValidate()` to receive the complete validation
  result. Protect only intrinsic C++ safety invariants where necessary.
- Keep comments and Doxygen descriptions aligned with the current upstream
  MNX schema and specification.

## Vendored MNX assets

The schema and examples are committed and are not downloaded during normal
builds. Maintainers update them with:

```sh
./scripts/update-w3c-mnx.sh main
```

The script updates `UPSTREAM_COMMIT`, `mnx-schema.json`, the generated
`mnx-schema.xxd`, and any changed example files. Review the full diff after an
update. Do not hand-edit generated schema bytes or run the vendoring script as
part of a package build.

Changes to the vendored schema can affect `Document`'s MNX version and
provenance, because the embedded schema `$id` is used by the library. Rebuild
and run the complete test suite after such changes.

## Documentation and packaging

Build API documentation with:

```sh
cd docs
doxygen Doxyfile
```

For package-oriented builds, use system dependencies and disable tests as
documented in `README.md`; package builds should not fetch dependencies or
update vendored assets from the network.

## Change checklist

Before handing off a change:

1. Check `git diff --check`.
2. Build with the repository's CMake configuration.
3. Run `ctest --test-dir build --output-on-failure`.
4. For schema or vendored-asset changes, inspect the schema ID, upstream pin,
   generated header, example count, and validation-test results.
5. Preserve unrelated user changes and do not reset or rewrite the worktree.
