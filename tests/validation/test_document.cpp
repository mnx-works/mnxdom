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
#include <string>
#include <filesystem>
#include <iterator>

#include "gtest/gtest.h"
#include "mnxdom.h"
#include "test_utils.h"

TEST(Document, UnrelatedJson)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "validation" / "unrelated_json.json";
    auto doc = mnx::Document::create(inputPath);
    EXPECT_FALSE(mnx::validation::schemaValidate(doc));
    EXPECT_FALSE(mnx::validation::hasValidDocumentRoot(doc));
}

TEST(Document, ValidTopLevelRoot)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "validation" / "invalid_mnx.json";
    auto doc = mnx::Document::create(inputPath);
    EXPECT_FALSE(mnx::validation::schemaValidate(doc));
    EXPECT_TRUE(mnx::validation::hasValidDocumentRoot(doc));
}

TEST(Document, DuplicateArbitraryIds)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "duplicate_ids_arbitrary.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, {
        "ID \"m1\" already exists for type \"measure-global\" at /global/measures/0",          // reported at the positioned clef
        "ID \"P1\" already exists for type \"root\" at ",                                      // reported at the part
        "ID \"tempo1\" already exists for type \"tempo\" at /global/measures/0/tempos/0",     // reported at the staff source
        "ID \"seq2\" already exists for type \"note\" at /parts/0/measures/0/sequences/1/content/0/notes/0", // reported at the sequence
        "ID \"P1\" has type \"root\", but expected \"part\"."                                // the staff source's part reference resolves to the root
    });
    // Every duplicate is reported at the object carrying the duplicate id, in document (sorted-key) order.
    const auto result = mnx::validation::semanticValidate(doc);
    ASSERT_EQ(result.errors.size(), 5u);
    EXPECT_EQ(result.errors[0].pointer.to_string(), "/layouts/0/content/0/sources/0");
    EXPECT_EQ(result.errors[1].pointer.to_string(), "/parts/0");
    EXPECT_EQ(result.errors[2].pointer.to_string(), "/parts/0/measures/0/clefs/0");
    EXPECT_EQ(result.errors[3].pointer.to_string(), "/parts/0/measures/0/sequences/1");
    EXPECT_EQ(result.errors[4].pointer.to_string(), "/layouts/0/content/0/sources/0");
}

TEST(Document, DuplicateIdUnknownLocation)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "duplicate_id_unknown_location.json";
    auto doc = mnx::Document::create(inputPath);
    // The id under the unexpected key is walked first (nlohmann sorts object keys), so the global measure is the duplicate.
    expectSemanticError(doc, inputPath, "ID \"m1\" already exists for type \"<unknown>\" at /global/aaaUnexpected/0", /*skipSchema*/true);
    doc.buildEntityMap({}, [](const std::string&, const mnx::Base&) {});
    const auto entry = doc.getEntityMap().tryFind("m1");
    ASSERT_TRUE(entry.has_value());
    EXPECT_EQ(entry->location.to_string(), "/global/aaaUnexpected/0");
    EXPECT_TRUE(entry->typeName.empty());
    EXPECT_FALSE(doc.getEntityMap().exists<mnx::global::Measure>("m1"));
    EXPECT_THROW(doc.getEntityMap().get<mnx::global::Measure>("m1"), mnx::util::mapping_error);
}

TEST(Document, IdsInVendorExtensionsIgnored)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "test_cases" / "ids_in_vendor_extensions.json";
    auto doc = mnx::Document::create(inputPath);
    EXPECT_TRUE(fullValidate(doc, inputPath));
    doc.buildEntityMap();
    const auto& entityMap = doc.getEntityMap();
    EXPECT_EQ(entityMap.idCount(), 3u);
    ASSERT_TRUE(entityMap.tryFind("P1").has_value());
    EXPECT_EQ(entityMap.tryFind("P1")->typeName, "part");
    EXPECT_EQ(entityMap.tryFind("P1")->location.to_string(), "/parts/0");
    EXPECT_EQ(entityMap.tryFind("ev1")->typeName, "event");
    EXPECT_EQ(entityMap.tryFind("m1")->typeName, "measure-global");
}

namespace {
void collectIds(const mnx::json& node, const mnx::json_pointer& pointer, std::vector<std::pair<std::string, std::string>>& out)
{
    if (node.is_object()) {
        for (const auto& [key, value] : node.items()) {
            if (key == "_x") {
                continue;
            }
            if (key == "id" && value.is_string()) {
                out.emplace_back(value.get<std::string>(), pointer.to_string());
            } else {
                collectIds(value, pointer / key, out);
            }
        }
    } else if (node.is_array()) {
        for (size_t index = 0; index < node.size(); ++index) {
            collectIds(node[index], pointer / index, out);
        }
    }
}
} // namespace

TEST(Document, EntityMapClassifiesAllIds)
{
    const std::filesystem::path inputPath = MNX_W3C_EXAMPLES_PATH;
    ASSERT_TRUE(std::filesystem::exists(inputPath)) << "examples path does not exist: " << inputPath;
    int filesProcessed = 0;
    for (const auto& entry : std::filesystem::directory_iterator(inputPath)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".json") {
            continue;
        }
        auto doc = mnx::Document::create(entry.path());
        doc.buildEntityMap({}, [&](const std::string& message, const mnx::Base& location) {
            ADD_FAILURE() << entry.path().filename() << ": " << location.pointer().to_string() << ": " << message;
        });
        const auto& entityMap = doc.getEntityMap();
        std::vector<std::pair<std::string, std::string>> expected;
        collectIds(*doc.root(), mnx::json_pointer{}, expected);
        EXPECT_EQ(entityMap.idCount(), expected.size()) << entry.path().filename();
        size_t visited = 0;
        entityMap.forEachId([&](const std::string&, const mnx::util::EntityMap::IdEntry&) { visited++; });
        EXPECT_EQ(visited, expected.size()) << entry.path().filename();
        for (const auto& [id, pointer] : expected) {
            const auto found = entityMap.tryFind(id);
            ASSERT_TRUE(found.has_value()) << entry.path().filename() << ": id " << id;
            EXPECT_EQ(found->location.to_string(), pointer) << entry.path().filename() << ": id " << id;
            EXPECT_FALSE(found->typeName.empty()) << entry.path().filename() << ": id " << id << " at " << pointer << " was not classified";
        }
        filesProcessed++;
    }
    EXPECT_GT(filesProcessed, 0) << "no files processed!";
}

TEST(Document, EntityMapTypedLookups)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "duplicate_ids_arbitrary.json";
    auto doc = mnx::Document::create(inputPath);
    doc.buildEntityMap({}, [](const std::string&, const mnx::Base&) {});
    const auto& entityMap = doc.getEntityMap();
    EXPECT_TRUE(entityMap.exists<mnx::Sequence>("seq1"));
    EXPECT_TRUE(entityMap.exists<mnx::global::Tempo>("tempo1"));
    EXPECT_TRUE(entityMap.exists<mnx::layout::Staff>("layoutStaff1"));
    EXPECT_TRUE(entityMap.exists<mnx::layout::LayoutContentObject>("layoutStaff1"));
    EXPECT_TRUE(entityMap.exists<mnx::sequence::SequenceContentObject>("ev1"));
    EXPECT_TRUE(entityMap.exists<mnx::global::Measure>("m1"));         // first occurrence wins; the duplicate clef is not mapped
    EXPECT_FALSE(entityMap.exists<mnx::part::PositionedClef>("m1"));
    EXPECT_FALSE(entityMap.exists<mnx::global::Measure>("tempo1"));
    EXPECT_EQ(entityMap.get<mnx::Sequence>("seq1").pointer().to_string(), "/parts/0/measures/0/sequences/0");
    EXPECT_EQ(entityMap.get<mnx::sequence::SequenceContentObject>("ev1").type(), "event");
}
