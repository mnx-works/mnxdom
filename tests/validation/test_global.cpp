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

TEST(Global, DuplicateMeasures)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "duplicate_measures.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "ID \"m1\" already exists for type \"measure-global\" at /global/measures/0");
}

TEST(Global, InvalidLyricLineIds)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "nonexistent_lyric_line_id.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, { "Lyric line ID \"line1\" is not listed in global.lyrics.lineOrder.", "Lyric line ID \"line2\" is not listed in global.lyrics.lineOrder.", });
}

TEST(Global, LyricLineOrder)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "lyrics_missing_global.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "Event has lyrics, but global lyrics are not defined.");

    // Creating global lyrics creates the required lineOrder.
    auto lyrics = doc.global().ensure_lyrics();
    EXPECT_EQ(lyrics.lineOrder().size(), 0u);
    EXPECT_FALSE(mnx::validation::schemaValidate(doc)); // lineOrder requires at least one item
    lyrics.lineOrder().push_back("1");
    lyrics.lineOrder().push_back("2");
    // lineMetadata may describe a subset of lineOrder.
    lyrics.ensure_lineMetadata().append("2").set_lang("nl");
    const auto input = mnx::validation::schemaValidate(doc, withIdCollection);
    ASSERT_TRUE(input);
    const auto semantic = mnx::validation::semanticValidate(doc, input.ids.value());
    for (const auto& error : semantic.errors) {
        ADD_FAILURE() << error.to_string();
    }
    EXPECT_EQ(semantic.lyricLines.size(), 2u);
    doc.buildEntityMapFromIndex(input.ids.value());
    EXPECT_EQ(doc.getEntityMap().getLyricLineOrder(), (std::vector<std::string>{ "1", "2" }));

    struct Case {
        const char* lyrics;
        bool valid;
    };
    const Case cases[] = {
        { R"({"lineOrder":["1","2"]})", true },
        { R"({"lineOrder":["2","1"],"lineMetadata":{"1":{"lang":"en"}}})", true },
        { R"({})", false },                       // lineOrder is required
        { R"({"lineMetadata":{"1":{}}})", false }, // lineOrder is required
        { R"({"lineOrder":[]})", false },          // minItems
        { R"({"lineOrder":["1","1"]})", false }    // uniqueItems
    };
    for (const auto& test : cases) {
        SCOPED_TRACE(test.lyrics);
        (*doc.root())["global"]["lyrics"] = mnx::json::parse(test.lyrics);
        EXPECT_EQ(bool(mnx::validation::schemaValidate(doc)), test.valid);
    }
}

TEST(Global, MeasureIndices)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "measures_mismatch.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "contains a different number of measures (4) than are defined globally (3)");

    auto measures = doc.global().measures();
    int firstIndex = 1;
    for (const auto measure : measures) {
        EXPECT_EQ(measure.calcVisibleNumber(), firstIndex++);
    }
}

TEST(Global, TimeSigFraction)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "test_cases" / "time-6-8.json";
    auto doc = mnx::Document::create(inputPath);
    EXPECT_TRUE(fullValidate(doc, inputPath)) << "full validation";

    auto measures = doc.global().measures();
    ASSERT_FALSE(measures.empty());
    auto measureTime = measures[0].calcCurrentTime();
    ASSERT_TRUE(measureTime);
    auto measureDura = mnx::FractionValue(measureTime.value());
    EXPECT_EQ(measureDura.numerator(), 6);
    EXPECT_EQ(measureDura.denominator(), 8);
}

TEST(Global, DuplicateLyricLineOrder)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "duplicate_lyric_line_order.json";
    auto doc = mnx::Document::create(inputPath);
    auto lineOrder = doc.global().lyrics().value().lineOrder();
    EXPECT_EQ(lineOrder.size(), 3u) << "wrapping existing JSON does not remove duplicates";

    const auto result = mnx::validation::schemaValidate(doc);
    EXPECT_FALSE(result) << "schema should reject duplicate lineOrder values";
    ASSERT_EQ(result.errors.size(), 1u);
    EXPECT_EQ(result.errors[0].pointer.to_string(), "/global/lyrics/lineOrder");

    EXPECT_TRUE(lineOrder.set(0, "1")) << "writing the value already at the index is allowed";
    EXPECT_FALSE(lineOrder.set(1, "1")) << "value exists at another index";
}
