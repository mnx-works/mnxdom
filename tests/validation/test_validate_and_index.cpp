/*
 * Copyright (C) 2025, Robert Patterson
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include <tuple>

#include "gtest/gtest.h"
#include "mnxdom.h"
#include "test_utils.h"

namespace {

using IdRow = std::tuple<std::string, std::string, std::string>;

std::vector<IdRow> rows(const mnx::DocumentIdIndex& index)
{
    std::vector<IdRow> result;
    for (const auto& item : index) {
        result.emplace_back(item.id, item.location.to_string(), item.typeName);
    }
    return result;
}

void expectSameDiagnostics(const mnx::validation::ValidationResult& actual,
                           const mnx::validation::ValidationResult& expected)
{
    ASSERT_EQ(actual.errors.size(), expected.errors.size());
    for (size_t i = 0; i < actual.errors.size(); ++i) {
        EXPECT_EQ(actual.errors[i].pointer, expected.errors[i].pointer);
        EXPECT_EQ(actual.errors[i].instance, expected.errors[i].instance);
        EXPECT_EQ(actual.errors[i].message, expected.errors[i].message);
    }
}

TEST(SchemaValidation, IdSnapshotIsOptIn)
{
    mnx::Document doc;
    const auto diagnostics = mnx::validation::schemaValidate(doc);
    EXPECT_FALSE(diagnostics.ids.has_value());

    const auto indexed = mnx::validation::schemaValidate(doc, withIdCollection);
    ASSERT_TRUE(indexed.ids.has_value());
    EXPECT_EQ(indexed.ids->size(), 0u);
    expectSameDiagnostics(indexed, diagnostics);
}

TEST(SchemaValidation, CustomSchemaControlsValidityAndIdClassification)
{
    setupTestDataPaths();
    const auto doc = mnx::Document::create(getInputPath() / "test_cases" / "arpeggio_valid.json");
    const auto embedded = mnx::validation::schemaValidate(doc, withIdCollection);
    ASSERT_TRUE(embedded);
    ASSERT_GT(embedded.ids.value().size(), 0u);
    expectSameDiagnostics(embedded, mnx::validation::schemaValidate(doc));

    mnx::validation::SchemaValidationOptions options;
    options.jsonSchema = R"({
        "required": ["publisher"],
        "properties": {"parts": {"items": {"$ref": "#/$defs/publisher-part"}}},
        "$defs": {"publisher-part": {"properties": {"id": {"type": "string"}}}}
    })";
    const auto schemaOnly = mnx::validation::schemaValidate(doc, options);
    EXPECT_FALSE(schemaOnly.ids.has_value());
    options.collectIds = true;
    const auto rejected = mnx::validation::schemaValidate(doc, options);
    EXPECT_FALSE(rejected);
    expectSameDiagnostics(rejected, schemaOnly);
    const std::vector<IdRow> customIds {
        {"m1", "/global/measures/0", ""},
        {"P1", "/parts/0", "publisher-part"},
        {"ev1", "/parts/0/measures/0/sequences/0/content/0", ""},
        {"upper", "/parts/0/measures/0/sequences/0/content/0/notes/0", ""},
        {"ev2", "/parts/0/measures/0/sequences/1/content/0", ""},
        {"lower", "/parts/0/measures/0/sequences/1/content/0/notes/0", ""}
    };
    EXPECT_EQ(rows(rejected.ids.value()), customIds);
    EXPECT_FALSE(doc.hasEntityMap());

    for (const bool collectIds : {false, true}) {
        options.collectIds = collectIds;
        options.jsonSchema = "{";
        EXPECT_THROW(mnx::validation::schemaValidate(doc, options), mnx::json::parse_error);
        options.jsonSchema = R"({"$ref":"#/missing"})";
        EXPECT_THROW(mnx::validation::schemaValidate(doc, options), std::exception);
    }
}

TEST(SchemaValidation, RecoverableIdsInEncounterOrder)
{
    setupTestDataPaths();
    const auto doc = mnx::Document::create(getInputPath() / "validation" / "index_edge_cases.json");
    const auto embedded = mnx::validation::schemaValidate(doc, withIdCollection);
    EXPECT_FALSE(embedded);
    expectSameDiagnostics(embedded, mnx::validation::schemaValidate(doc));
    mnx::validation::SchemaValidationOptions options;
    options.jsonSchema = "{}";
    const auto schemaOnly = mnx::validation::schemaValidate(doc, options);
    EXPECT_FALSE(schemaOnly.ids.has_value());
    options.collectIds = true;
    const auto accepted = mnx::validation::schemaValidate(doc, options);
    EXPECT_TRUE(accepted);
    expectSameDiagnostics(accepted, schemaOnly);
    const std::vector<IdRow> expected {
        {"m1", "/global/measures/0", "measure-global"},
        {"unknown", "/global/unexpected/0", ""},
        {"span", "/parts/0/measures/0/arpeggios/0/span", ""},
        {"duplicate", "/parts/0/measures/0/clefs/0/clef", "clef"},
        {"duplicate", "/parts/0/measures/0", "part-measure"},
        {"event", "/parts/0/measures/0/sequences/0/content/0", "event"},
        {"n1", "/parts/0/measures/0/sequences/0/content/0/notes/0", "note"}
    };
    EXPECT_EQ(rows(embedded.ids.value()), expected);
    // The empty schema supplies no type names and does not describe kit as a dictionary.
    auto unclassified = expected;
    for (auto& item : unclassified) {
        std::get<2>(item).clear();
    }
    unclassified.insert(unclassified.begin() + 2, {"dictionary-value", "/parts/0/kit", ""});
    EXPECT_EQ(rows(accepted.ids.value()), unclassified);
    EXPECT_FALSE(doc.hasEntityMap());
}

TEST(SchemaValidation, DuplicateOwnershipAndDiagnostics)
{
    setupTestDataPaths();
    auto doc = mnx::Document::create(getInputPath() / "test_cases" / "arpeggio_valid.json");
    auto& measure = (*doc.root())["parts"][0]["measures"][0];
    measure["clefs"][0]["clef"]["id"] = "dup";
    measure["id"] = "dup";
    const auto input = mnx::validation::schemaValidate(doc, withIdCollection);
    EXPECT_TRUE(input);
    EXPECT_THROW(doc.buildEntityMapFromIndex(input.ids.value()), mnx::util::mapping_error);

    std::vector<std::pair<std::string, std::string>> diagnostics;
    const auto handler = [&](const std::string& message, const mnx::Base& location) {
        diagnostics.emplace_back(message, location.pointer().to_string());
    };
    doc.buildEntityMapFromIndex(input.ids.value(), {}, handler);
    ASSERT_EQ(diagnostics.size(), 1u);
    EXPECT_EQ(diagnostics.front().second, "/parts/0/measures/0");
    EXPECT_NE(diagnostics.front().first.find("/parts/0/measures/0/clefs/0/clef"), std::string::npos);
    const auto owner = doc.getEntityMap().tryFind("dup");
    ASSERT_TRUE(owner);
    EXPECT_EQ(owner->typeName, "clef");
    EXPECT_EQ(owner->location.to_string(), "/parts/0/measures/0/clefs/0/clef");
}

TEST(SchemaValidation, SupplementalMapsRespectImportPolicies)
{
    setupTestDataPaths();
    auto doc = mnx::Document::create(getInputPath() / "test_cases" / "ottavas_voice_policy.json");
    const auto input = mnx::validation::schemaValidate(doc, withIdCollection);
    ASSERT_TRUE(input);
    mnx::EntityMapPolicies policies;
    policies.ottavasRespectVoiceTargets = false;

    doc.buildEntityMapFromIndex(input.ids.value(), policies);
    const auto& mapping = doc.getEntityMap();
    EXPECT_EQ(mapping.getOttavaShift(mapping.get<mnx::sequence::Event>("low1")), -1);
    EXPECT_TRUE(mapping.tryGetEventInfo("low1"));
    for (const auto& item : input.ids.value()) {
        const auto found = mapping.tryFind(item.id);
        ASSERT_TRUE(found);
        EXPECT_EQ(found->location, item.location);
        EXPECT_EQ(found->typeName, item.typeName);
    }
}

} // namespace
