# mnxdom

Document object model for the MNX music interchange format. It is compatible with the C++17 standard.

- compatible with the C++17 standard and higher. (Currently tested with C++23.)
- uses [nlohmann\_json](https://github.com/nlohmann/json) as its JSON parser.
- allows structured access to MNX objects, without the need of quoted strings.
- validates against the MNX schema using [json-schema-validator](https://github.com/pboettch/json-schema-validator).
- no class serialization beyond nlohmann.
- every MNX class is a wrapper around the root JSON object and a pointer to the location of the instance.

### Documentation

[MNX Document Model for C++17](https://mnx-works.github.io/mnxdom/)

Here is a simple example of code that creates the [Hello World](https://w3c.github.io/mnx/docs/mnx-reference/examples/hello-world/) MNX JSON file.

```cpp
// create new document
auto doc = mnx::Document(); // automatically creates required child nodes
// global
auto globalMeasure = doc.global().measures().append();
// required fields are supplied when objects are created
globalMeasure.ensure_barline(mnx::BarlineType::Regular);
globalMeasure.ensure_time(4, mnx::TimeSignatureUnit::Quarter);
// parts
auto part = doc.parts().append();
auto measure = part.measures().append();
measure.ensure_clefs().append(mnx::ClefSign::GClef, -2, mnx::OttavaAmountOrZero::NoTransposition);
auto event = measure.sequences()
    .append()
    .content()
    .appendEvent(mnx::NoteValueBase::Whole);
event.ensure_notes().append(mnx::sequence::Pitch::make(mnx::NoteStep::C, 4));
// save to file
doc.save("hello-world.json", 4); // indent with 4 spaces
```

Note the naming conventions above. Children that MNX requires get a `create_NAME(...)` method, but a parent's constructor creates them for you, so you can normally just access them (`part.measures()`). Optional children get `ensure_NAME(...)`, which creates the child on first use and returns the existing one thereafter. Content arrays, such as a sequence's content or a layout's content, are heterogeneous and so provide typed helpers (`appendEvent`, `appendTuplet`, `appendStaff`, and so on) in place of a generic `append`.

Note that this code never uses references. Since every MNX class in the model is a lightweight wrapper around a root JSON object and a pointer to its location, copying them is extremely cheap. The class instance returned by any method is a temporary instance. Using references on return values can result in undefined behavior.

### Semantic validation and event timing

`mnx::validation::schemaValidate(doc)` checks the embedded MNX schema.
`mnx::validation::semanticValidate(doc)` checks relationships such as lyric
extender targets and jump destinations. Its result contains `errors` and
`warnings`, both with an instance, JSON pointer, and message. Warnings are
advisory: conversion of the result to `bool` succeeds whenever `errors` is empty.
Hidden lyric extenders (`show: false`) ignore their targets completely.

After `doc.buildEntityMap()`, use `doc.getEntityMap().getEventInfo(id)` (or pass
an event wrapper) to retrieve the event, global measure index, elapsed metric
time within the measure, actual duration, and grace index. Durations are exact
fractions of a whole note, adjusted for tuplets and tremolos; grace notes have
zero actual duration. `compareEventPositions(lhs, rhs)` compares these onsets,
including chronological grace order, without requiring measure IDs. These are
score positions, not playback times in seconds or positions after repeats.
Canonical event timing is independent of ottava policies. Rebuild the entity
map and reacquire timing information after editing the document.

### Setup Instructions

#### Via `FetchContent`

Include the top header in your source file.

```cpp
#include "mnxdom.h"
```

Add the library to your project with `FetchContent`:

```cmake
include(FetchContent)

FetchContent_Declare(
  mnxdom
  GIT_REPOSITORY https://github.com/mnx-works/mnxdom
  GIT_TAG main # Replace with the desired commit hash, tag, or branch
)
FetchContent_MakeAvailable(mnxdom)

# Also add somewhere:
target_link_libraries(project PRIVATE mnxdom) # replace "project" with your actual project name
```

#### Via a system installation

Install the library using:

```
sudo cmake --install build
```

This should install:

- Headers in `/usr/local/include/mnxdom`
- `pkg-config` file in `/usr/local/lib/pkgconfig/mnxdom.pc`

If you use `cmake` in your project, and assuming `pkg-config` on your system is configured to look for files in `/usr/local/lib/pkgconfig` too, you can use:

```cmake
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(mnxdom REQUIRED IMPORTED_TARGET mnxdom)
  target_link_libraries(project PRIVATE PkgConfig::mnxdom)
```

And include the top header in your source file with:

```cpp
#include <mnxdom/mnxdom.h>
```

#### Dependencies

The dependencies [pboettch/json-schema-validator](https://github.com/pboettch/json-schema-validator) and [nlohmann/json](https://json.nlohmann.me/) are required by mnxdom. If you wish mnxdom to use the system versions of either of these dependencies, configure mnxdom with the following CMake flags:

```sh
-DUSE_SYSTEM_NLOHMANN_JSON=ON
-DUSE_SYSTEM_JSON_SCHEMA_VALIDATOR=ON
```

mnxdom vendors the W3C MNX schema and example JSON files in `third_party/w3c-mnx`. These files are not fetched during the build. To update them from the current `main` branch of the W3C MNX repository, run:

```sh
scripts/update-w3c-mnx.sh
```

To update to a specific upstream commit or ref of the w3c repository, pass it as an argument. The resolved commit is recorded in `third_party/w3c-mnx/UPSTREAM_COMMIT`.

#### Package Builds

Package creators should configure mnxdom so that the build does not download dependencies from the internet. Use system-provided copies of nlohmann/json and json-schema-validator:

```sh
cmake -S . -B build \
  -DUSE_SYSTEM_NLOHMANN_JSON=ON \
  -DUSE_SYSTEM_JSON_SCHEMA_VALIDATOR=ON \
  -Dmnxdom_BUILD_TESTING=OFF
```

If package builds run the test suite, also use a system-provided GoogleTest:

```sh
cmake -S . -B build \
  -DUSE_SYSTEM_NLOHMANN_JSON=ON \
  -DUSE_SYSTEM_JSON_SCHEMA_VALIDATOR=ON \
  -DUSE_SYSTEM_GOOGLETEST=ON
```

The W3C MNX schema and examples are vendored in `third_party/w3c-mnx`, so CMake does not fetch them. Do not run `scripts/update-w3c-mnx.sh` during a package build; that script is for maintainers updating the vendored snapshot before a release.

mnxdom records its own version and, when available, its source commit in generated MNX provenance. When building from a source tree without `.git`, whoever builds mnxdom may provide the source revision explicitly. This includes package creators and client projects that vendor mnxdom or bring it in with CMake `FetchContent`:

```sh
-DMNXDOM_GIT_COMMIT=<mnxdom-commit>
```

If `MNXDOM_GIT_COMMIT` is not supplied and `.git` is unavailable, the commit field is omitted from the provenance rather than filled with a placeholder.

Package builds may also provide a package or build identifier for mnxdom itself:

```sh
-DMNXDOM_BUILD_ID=<mnxdom-build-id>
```

This value is recorded as `mnxdom.build` in provenance. Client application build metadata should be recorded separately in the client provenance fields.

### To Run the Tests

You need `cmake` to build and run the tests. From the directory containing this repository, configure the build directory:

```bash
cmake -S . build
```

To build the tests:

```bash
cmake --build build
```

To run the tests:

```bash
ctest --test-dir build
```

### To Build the Documentation

You need [Doxygen](https://doxygen.nl/index.html) to create the documentation. You can download and run the installer from the Doxygen site. Package managers can also install it. Once you have Doxygen you can build the documentation as follows.

```bash
cd docs
doxygen Doxyfile
```

You will then find an html website in `./docs/generated/html`. Use a web browser to view the site from your hard drive. Opening any of the `.html` pages gives you access to the entire site.
