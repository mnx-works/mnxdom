/*
 * Copyright (C) 2024, Robert Patterson
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
#include "gtest/gtest.h"

#include "mnxdom.h"

#include "test_utils.h"

using namespace mnx;

TEST(Global, LyricLineMetadata)
{
    std::istringstream jsonString(R"(
        {
            "mnx": {
                "version": 1
            },
            "global": {
                "lyrics": {
                    "lineMetadata": {
                        "3": {
                            "label": "日本語",
                            "lang": "ja"
                        },
                        "2": {
                            "label": "Deutsch",
                            "lang": "de"
                        },
                        "1": {
                            "label": "English",
                            "lang": "en"
                        },
                        "4": {
                            "label": "Español",
                            "lang": "es"
                        }
                    },
                    "lineOrder": [
                        "1",
                        "2",
                        "3",
                        "4"
                    ]
                },
              "measures": []
            },
            "parts": []
        }
    )");
    Document doc(jsonString);
    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate and return no error";

    auto lyrics = doc.global().lyrics();
    ASSERT_TRUE(lyrics.has_value()) << "lyrics should have a value";
    auto lineMetaData = lyrics.value().lineMetadata();
    ASSERT_TRUE(lineMetaData.has_value()) << "lineMetaData should have a value";

    EXPECT_EQ(lineMetaData->at("2").lang(), "de");
    EXPECT_EQ(lineMetaData->at("3").label(), "日本語");

    size_t index = 0;
    bool got1 = false;
    bool got4 = false;
    for (const auto& it : lineMetaData.value()) {
        if (it.first == "1") {
            got1 = true;
            EXPECT_EQ(it.second.lang().value(), "en");
            EXPECT_EQ(it.second.label().value(), "English");
        } else if (it.first == "4") {
            got4 = true;
            EXPECT_EQ(it.second.lang().value(), "es");
            EXPECT_EQ(it.second.label().value(), "Español");
        }
        index++;
        ASSERT_TRUE(index <= 4) << "iteration loop is not stopping";        
        //std::cout << "\"" << it.first << "\": " << *it.second.lang() << " " << *it.second.label() << std::endl;
    }
    EXPECT_TRUE(got1 && got4) << "missing either item 1 or item 4";

    auto newLine = lineMetaData->append("15");
    newLine.set_label("Italiano");
    newLine.set_lang("it");
    EXPECT_EQ(lineMetaData->size(), 5u);
    EXPECT_EQ(lineMetaData->at("15").label(), "Italiano");
    EXPECT_EQ(lineMetaData->at("15").lang(), "it");

    for (auto it : *lineMetaData) {
        if (it.first == "3") {
            it.second.set_label("Nederlands");
            it.second.set_lang("nl");
        }
    }
    EXPECT_EQ(lineMetaData->at("3").label(), "Nederlands");
    EXPECT_EQ(lineMetaData->at("3").lang(), "nl");

    {
        const auto it = lineMetaData->find("222");
        EXPECT_EQ(it, lineMetaData->end()) << "find invalid key";
    }
    {
        const auto it = lineMetaData->find("2");
        EXPECT_NE(it, lineMetaData->end()) << "find valid key";
        EXPECT_EQ(it->second.label(), "Deutsch");
        EXPECT_EQ(it->second.lang(), "de");
    }
    {
        auto it = lineMetaData->find("222");
        EXPECT_EQ(it, lineMetaData->end()) << "find invalid key";
    }
    {
        auto it = lineMetaData->find("15");
        EXPECT_NE(it, lineMetaData->end()) << "find valid key";
        it->second.set_label("Français");
        it->second.set_lang("fr");
    }
    EXPECT_EQ(lineMetaData->at("15").label(), "Français");
    EXPECT_EQ(lineMetaData->at("15").lang(), "fr");

    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate and return no error";
    //std::cout << doc.dump(4) << std::endl;
}

TEST(Global, EndingTest)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "w3c_examples" / "repeats-alternate-endings-advanced.json";
    auto doc = mnx::Document::create(inputPath);
    EXPECT_TRUE(validateSchema(doc, inputPath)) << "schema validatation failed"; // empty group means semantic fails: tested elsewhere

    auto measures = doc.global().measures();
    ASSERT_GE(measures.size(), 2);
    {
        auto measure = measures[1];
        ASSERT_TRUE(measure.ending());
        auto ending = measure.ending().value();
        ASSERT_TRUE(ending.numbers());
        std::vector<int> nums = ending.numbers()->toStdVector();
        std::vector<int> expectedNums = { 1, 2 };
        EXPECT_EQ(nums, expectedNums);
    }
}

TEST(Global, VisibleMeasureNumberBasic)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "w3c_examples" / "hello-world.json";
    auto doc = mnx::Document::create(inputPath);
    EXPECT_TRUE(validateSchema(doc, inputPath)) << "schema validatation failed"; // empty group means semantic fails: tested elsewhere

    auto measures = doc.global().measures();
    ASSERT_GE(measures.size(), 1);
    {
        auto measure = measures[0];
        EXPECT_EQ(measure.calcVisibleNumber(), 1);
    }
}

TEST(Global, TempoFractionalBpm)
{
    Document doc;
    auto measures = doc.global().measures();
    auto measure = measures.append();
    auto tempos = measure.ensure_tempos();
    auto tempo = tempos.append(84.5, NoteValue::make(NoteValueBase::Quarter, 0));

    EXPECT_DOUBLE_EQ(tempo.bpm(), 84.5);
    EXPECT_TRUE(validation::schemaValidate(doc)) << "fractional bpm should validate against the MNX schema";

    tempo.set_bpm(120.0);
    EXPECT_DOUBLE_EQ(tempo.bpm(), 120.0);
    EXPECT_TRUE(validation::schemaValidate(doc)) << "whole-number bpm should validate against the MNX schema";

    // integer-encoded bpm from an existing document still reads back as a double
    std::istringstream jsonString(R"(
        {
            "mnx": { "version": 1 },
            "global": {
                "measures": [
                    {
                        "tempos": [
                            { "bpm": 132, "value": { "base": "quarter" } }
                        ]
                    }
                ]
            },
            "parts": []
        }
    )");
    Document parsed(jsonString);
    auto parsedTempo = parsed.global().measures()[0].tempos().value()[0];
    EXPECT_DOUBLE_EQ(parsedTempo.bpm(), 132.0);
}

TEST(Global, Schema46NavigationAndCaesura)
{
    Document doc;
    auto measure = doc.global().measures().append();
    auto coda = measure.ensure_coda(FractionValue{0, 1});
    coda.set_color("#123456");
    coda.set_glyph("coda");
    EXPECT_EQ(coda.location().fraction(), (FractionValue{0, 1}));
    EXPECT_EQ(coda.glyph(), "coda");
    auto caesura = measure.ensure_caesura();
    EXPECT_EQ(caesura.marks(), 2u);
    EXPECT_EQ(caesura.shape(), CaesuraShape::Normal);
    const std::pair<JumpType, std::string_view> jumps[] = {
        {JumpType::Dc, "dc"}, {JumpType::DcAlCoda, "dcAlCoda"},
        {JumpType::DcAlFine, "dcAlFine"}, {JumpType::DsAlCoda, "dsAlCoda"},
        {JumpType::DsAlFine, "dsAlFine"}, {JumpType::Segno, "segno"},
        {JumpType::ToCoda, "toCoda"}
    };
    for (const auto& [type, text] : jumps) {
        auto jump = measure.ensure_jump(type, FractionValue{1, 1});
        jump.set_type(type);
        EXPECT_EQ(json::parse(jump.dump())["type"], text);
        global::Jump copy(std::make_shared<json>(json::parse(jump.dump())), json_pointer{});
        EXPECT_EQ(copy.type(), type);
        EXPECT_TRUE(validation::schemaValidate(doc));
    }
    auto support = doc.mnx().ensure_support();
    EXPECT_FALSE(support.useLyricExtenders());
    EXPECT_FALSE(json::parse(support.dump()).contains("useLyricExtenders"));
    support.set_useLyricExtenders(true);
    EXPECT_TRUE(validation::schemaValidate(doc));
}
