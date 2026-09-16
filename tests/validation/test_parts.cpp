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

TEST(Parts, DuplicateId)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "duplicate_parts.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "ID \"P1\" already exists for type \"part\" at /parts/0");
}

TEST(Parts, MeasuresMismatch)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "measures_mismatch.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "contains a different number of measures (4) than are defined globally (3)");
}

TEST(Parts, MissingInitialClefs)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "missing_initial_clef.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "Missing clef at the beginning of staff 1 in part P1 (first measure).");
}

TEST(Parts, MeasureRepeatOverlap)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "measure_repeat_overlap.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath,
        "Measure repeat in part P1 occurs at measure index 3, which is already covered by the measure repeat at index 2.");
}

TEST(Parts, MeasureRepeatUnderflow)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "measure_repeat_underflow.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath,
        "Measure repeat in part P1 at measure index 0 has a repeat count of 1, which reaches back before the start of the part.");
}

TEST(Parts, MeasureRepeatOverrun)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "measure_repeat_overrun.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath,
        "Measure repeat in part P1 at measure index 2 spans 2 measures, which extends past the end of the part (3 measures).");
}

TEST(Parts, ClefInvalidStaff)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "clef_invalid_staff.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "Clef references non-existent staff 2 in part P1.");
}

TEST(Parts, StaffConfigInvalidStaff)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "staff_config_invalid_staff.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "Staff config references non-existent staff 2 in part P1.");
}

TEST(Parts, InvalidLyricsLineIds)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "lyrics_invalid_lineids.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, { "ID \"1\" not found in ID mapping", "ID \"2\" not found in ID mapping" });
}

TEST(Parts, EventErrors)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "event_errors.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, {
        "ID \"ev5n1\" already exists for type \"note\" at",
        "ID \"ev6\" already exists for type \"event\" at",
        "ID \"ev7\" already exists for type \"event\" at",
        "Event \"ev7\" is a rest but also has notes"
    });
}

TEST(Parts, FullMeasureWithNonEmptyContent)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "sequence_fullmeasure_nonempty_content.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "has full measure rest but content is not empty");
}

TEST(Parts, DuplicateSequenceVoice)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "duplicate_sequence_voice.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "Voice \"layer1\" already exists at");
}

TEST(Parts, EmptySequenceVoice)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "empty_sequence_voice.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "has empty voice identifier");
}

TEST(Parts, TieErrors)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "bad_ties.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, {
        "Tie points to a note in a different part",
        "Tie points to a note with a different pitch"
    });
}

TEST(Parts, EnharmonicTies)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "test_cases" / "enharmonic_ties.json";
    auto doc = mnx::Document::create(inputPath);
    EXPECT_TRUE(fullValidate(doc, inputPath)) << "full validation";
}

TEST(Parts, BeamToGraceError)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "8th_plus_grace.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, {
        "Event \"ev7\" attempts to beam a grace note to a non grace note",
        "Event \"ev8\" attempts to beam a grace note to a non grace note"
    });
}

TEST(Parts, BeamAcrossVoicesError)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "layer1-2.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, {
        "Event \"ev10\" attempts to beam events from different voices together",
        "Event \"ev11\" attempts to beam events from different voices together"
    });
}

TEST(Parts, AlterationError)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "big_alter.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, {
        "Note \"note1\" has alteration value 4. MNX files are limited to +/-3."
    });
}

TEST(Parts, BeamWrongMeasure)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "beam_wrong_measure.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, {
        "First event in beam is not in the same measure as the beam."
    });
}

TEST(Parts, BeamOutOfOrder)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "beam_out_of_order.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, {
        "Beam events are out of sequence."
    });
}

TEST(Parts, ArpeggioSpanPositionError)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "arpeggio_bad_position.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, {
        "Arpeggio span notes must be in the same rhythmic position."
    });
}

TEST(Parts, NonArpeggioPitchOrderError)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "non_arpeggio_bad_order.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, {
        "Non-arpeggio span start note must be lower in sounded pitch than the end note."
    });
}

TEST(Parts, ValidArpeggioSpan)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "test_cases" / "arpeggio_valid.json";
    auto doc = mnx::Document::create(inputPath);
    EXPECT_TRUE(fullValidate(doc, inputPath)) << "full validation";
}

TEST(Parts, ValidArpeggioSpanImplicitPrimaryGraceIndex)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "test_cases" / "arpeggio_valid_implicit_grace_index.json";
    auto doc = mnx::Document::create(inputPath);
    EXPECT_TRUE(fullValidate(doc, inputPath)) << "full validation";
}

TEST(Parts, ValidArpeggioSameSequenceImplicitPrimaryGraceIndex)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "test_cases" / "arpeggio_same_sequence_implicit_primary_valid.json";
    auto doc = mnx::Document::create(inputPath);
    EXPECT_TRUE(fullValidate(doc, inputPath)) << "full validation";
}

TEST(Parts, ValidArpeggioSameSequenceImplicitGraceIndex)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "test_cases" / "arpeggio_same_sequence_implicit_grace_index_valid.json";
    auto doc = mnx::Document::create(inputPath);
    EXPECT_TRUE(fullValidate(doc, inputPath)) << "full validation";
}

TEST(Parts, ArpeggioSameSequenceGraceIndexMismatch)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "arpeggio_same_sequence_grace_index_mismatch.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, {
        "Arpeggio span notes must be in the same rhythmic position."
    });
}

TEST(Parts, DynamicsMissingRequiredAttributesByType)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "dynamics_missing_required_attrs.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, {
        "Dynamic of type \"immediate\" requires field \"value\".",
        "Dynamic of type \"gradual\" requires field \"end\".",
        "Dynamic of type \"gradual\" requires field \"wedgeType\".",
        "Dynamic of type \"relative\" requires field \"relativeValue\".",
        "Dynamic of type \"accent\" requires field \"value\"."
    });
}
