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
#include "util/MusicTheoryConversions.h"
#include "test_utils.h"

using namespace mnx;

TEST(TestSequences, MusicTheoryPitchConversions)
{
    struct StepMapping {
        NoteStep step;
        music_theory::NoteName noteName;
    };

    const StepMapping mappings[] = {
        { NoteStep::C, music_theory::NoteName::C },
        { NoteStep::D, music_theory::NoteName::D },
        { NoteStep::E, music_theory::NoteName::E },
        { NoteStep::F, music_theory::NoteName::F },
        { NoteStep::G, music_theory::NoteName::G },
        { NoteStep::A, music_theory::NoteName::A },
        { NoteStep::B, music_theory::NoteName::B }
    };

    for (const auto& mapping : mappings) {
        EXPECT_EQ(util::toMusicTheoryNoteName(mapping.step), mapping.noteName);
        EXPECT_EQ(util::toMnxNoteStep(mapping.noteName), mapping.step);
    }

    const auto pitch = sequence::Pitch::make(NoteStep::F, 3, -1);
    const auto theoryPitch = util::toMusicTheoryPitch(pitch);
    EXPECT_EQ(theoryPitch.noteName, music_theory::NoteName::F);
    EXPECT_EQ(theoryPitch.octave, 3);
    EXPECT_EQ(theoryPitch.alteration, -1);
}

void checkSequence(const part::Measure& measure, size_t contentIdx, int expectedTranKeyFifths,
    const sequence::Pitch::Required& expected1stTransPitch)
{
    ASSERT_FALSE(measure.sequences().empty());
    auto sequence = measure.sequences()[0];
    ASSERT_GT(sequence.content().size(), contentIdx);
    auto item = sequence.content()[contentIdx];
    ASSERT_EQ(item.type(), sequence::Event::ContentTypeValue);
    auto event = item.get<sequence::Event>();
    ASSERT_TRUE(event.notes());
    ASSERT_FALSE(event.notes()->empty());
    auto note = event.notes()->at(0);
    auto firstTransPitch = note.pitch().calcTransposed();
    EXPECT_EQ(firstTransPitch.step, expected1stTransPitch.step) << "transposed pitch step does not match expected value";
    EXPECT_EQ(firstTransPitch.octave, expected1stTransPitch.octave) << "transposed pitch octave does not match expected value";
    EXPECT_EQ(firstTransPitch.alter, expected1stTransPitch.alter) << "transposed pitch alter does not match expected value";

    auto globalMeasure = measure.getGlobalMeasure();
    auto part = measure.getEnclosingElement<Part>();
    ASSERT_TRUE(part);
    auto expectedTranKeySig = KeySignature::make(expectedTranKeyFifths);
    if (auto partTran = part->transposition()) {
        EXPECT_EQ(partTran->calcTransposedKey(globalMeasure.calcCurrentKeyFields()).fifths, expectedTranKeySig.fifths) << "transposed key does not match expected value";
    } else {
        EXPECT_EQ(globalMeasure.calcCurrentKeyFields().fifths, expectedTranKeySig.fifths) << "concert key does not match expected value";
    }
}

TEST(TestSequences, TransposedPitches)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "test_cases" / "key7-7-wrapped.json";
    auto doc = mnx::Document::create(inputPath);
    fullValidate(doc, inputPath);

    ASSERT_GE(doc.parts().size(), 4);
    ASSERT_GE(doc.global().measures().size(), 2);

    {
        auto part = doc.parts()[0];
        checkSequence(part.measures()[0], 0, +7, { NoteStep::G, 4, +1 });
        checkSequence(part.measures()[1], 0, -7, { NoteStep::C, 5, -1 });
    }
    {
        auto part = doc.parts()[1];
        checkSequence(part.measures()[0], 0, -3, { NoteStep::G, 4, 0 });
        checkSequence(part.measures()[1], 0, -5, { NoteStep::B, 4, -1 });
    }
    {
        auto part = doc.parts()[2];
        checkSequence(part.measures()[0], 0, +4, { NoteStep::A, 4, 0 });
        checkSequence(part.measures()[1], 0, +2, { NoteStep::D, 5, 0 });
    }
    {
        auto part = doc.parts()[3];
        checkSequence(part.measures()[0], 0, -2, { NoteStep::B, 4, -1 });
        checkSequence(part.measures()[1], 0, -4, { NoteStep::B, 4, -1 });
    }
}

TEST(TestSequences, TransposedPitchesNoWrap)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "test_cases" / "key7-7.json";
    auto doc = mnx::Document::create(inputPath);
    fullValidate(doc, inputPath);

    ASSERT_GE(doc.parts().size(), 4);
    ASSERT_GE(doc.global().measures().size(), 2);

    {
        auto part = doc.parts()[0];
        checkSequence(part.measures()[0], 0, +7, { NoteStep::C, 5, +1 });
        checkSequence(part.measures()[1], 0, -7, { NoteStep::C, 5, -1 });
    }
    {
        auto part = doc.parts()[1];
        checkSequence(part.measures()[0], 2, +9, { NoteStep::F, 5, +2 });
        checkSequence(part.measures()[1], 0, -5, { NoteStep::D, 5, -1 });
    }
    {
        auto part = doc.parts()[2];
        checkSequence(part.measures()[0], 0, +4, { NoteStep::E, 5, 0 });
        checkSequence(part.measures()[1], 0, -10, { NoteStep::E, 5, -2 });
    }
    {
        auto part = doc.parts()[3];
        checkSequence(part.measures()[0], 2, -2, { NoteStep::D, 6, 0 });
        checkSequence(part.measures()[1], 0, -4, { NoteStep::A, 5, -1 });
    }
}

TEST(TestSequences, TransposedPitchesEnharmonics)
{
    setupTestDataPaths();
    const auto inputPath = getInputPath() / "test_cases" / "enharmonics.mnx";
    auto doc = mnx::Document::create(inputPath);
    fullValidate(doc, inputPath);

    ASSERT_EQ(doc.parts().size(), 1);
    const auto part = doc.parts()[0];
    ASSERT_EQ(part.measures().size(), 8);

    const sequence::Pitch::Required expected[] = {
        { NoteStep::G, 5, -1 },
        { NoteStep::A, 5, -3 },
        { NoteStep::B, 5, -5 },
        { NoteStep::C, 6, -6 },
        { NoteStep::E, 4, +1 },
        { NoteStep::D, 4, +3 },
        { NoteStep::C, 4, +5 },
        { NoteStep::B, 3, +6 }
    };

    for (size_t i = 0; i < 8; ++i) {
        SCOPED_TRACE(::testing::Message() << "measure " << i);
        const auto measure = part.measures()[i];
        ASSERT_EQ(measure.sequences().size(), 1);
        const auto sequence = measure.sequences()[0];
        ASSERT_GE(sequence.content().size(), 1);

        const auto item = sequence.content()[0];
        ASSERT_EQ(item.type(), sequence::Event::ContentTypeValue);
        const auto event = item.get<sequence::Event>();
        ASSERT_TRUE(event.notes());
        ASSERT_FALSE(event.notes()->empty());

        const auto note = event.notes()->at(0);
        const auto actual = note.pitch().calcTransposed();
        EXPECT_EQ(actual.step, expected[i].step);
        EXPECT_EQ(actual.octave, expected[i].octave);
        EXPECT_EQ(actual.alter, expected[i].alter);
    }
}

TEST(TestSequences, WalkFullMeasureRest)
{
    auto doc = Document();
    auto globalMeasure = doc.global().measures().append();
    globalMeasure.ensure_time(4, TimeSignatureUnit::Quarter);
    auto part = doc.parts().append();
    auto measure = part.create_measures().append();
    auto sequence = measure.sequences().append();
    auto fullMeasure = sequence.ensure_fullMeasure();

    int fullMeasureCalls = 0;
    FractionValue fullMeasureStart = 0;
    FractionValue fullMeasureDuration = 0;
    util::SequenceWalkContext ctx;
    util::SequenceWalkHooks hooks;
    hooks.onFullMeasure = [&](const Sequence& callbackSequence,
                              const sequence::FullMeasureRest& callbackFullMeasure,
                              const FractionValue& startDuration,
                              const FractionValue& actualDuration,
                              util::SequenceWalkContext&) -> bool {
        ++fullMeasureCalls;
        EXPECT_EQ(callbackSequence.pointer(), sequence.pointer());
        EXPECT_EQ(callbackFullMeasure.pointer(), fullMeasure.pointer());
        fullMeasureStart = startDuration;
        fullMeasureDuration = actualDuration;
        return true;
    };

    const bool walked = util::walkSequenceContent(sequence, hooks, &ctx);
    ASSERT_TRUE(walked);
    EXPECT_EQ(fullMeasureCalls, 1);
    EXPECT_EQ(fullMeasureStart, FractionValue(0));
    EXPECT_EQ(fullMeasureDuration, static_cast<FractionValue>(globalMeasure.time().value()));
    EXPECT_EQ(ctx.elapsedTime, fullMeasureDuration);
}

TEST(TestSequences, PlacementsAndCaesura)
{
    Document doc;
    doc.global().measures().append().set_id("m1");
    auto part = doc.parts().append();
    part.set_id("P1");
    auto measure = part.measures().append();
    auto sequence = measure.sequences().append();

    EXPECT_EQ(sequence.directionHint(), DirectionHint::Auto) << "directionHint should default to auto";
    EXPECT_FALSE(nlohmann::json::parse(sequence.dump()).contains("directionHint")) << "default directionHint should not be serialized";
    sequence.set_directionHint(DirectionHint::Lower);
    EXPECT_EQ(nlohmann::json::parse(sequence.dump())["directionHint"], "lower");

    auto event = sequence.content().appendEvent(NoteValueBase::Quarter);
    event.set_id("e1");
    event.ensure_rest();
    auto markings = event.ensure_markings();

    auto caesura = markings.ensure_caesura();
    EXPECT_EQ(caesura.marks(), 2u) << "caesura marks should default to 2";
    EXPECT_EQ(caesura.shape(), CaesuraShape::Normal) << "caesura shape should default to normal";
    EXPECT_EQ(caesura.dump(), "{}") << "caesura defaults should not be serialized";
    caesura.set_marks(1);
    caesura.set_shape(CaesuraShape::Curved);
    EXPECT_EQ(nlohmann::json::parse(caesura.dump())["shape"], "curved");

    auto staccato = markings.ensure_staccato();
    EXPECT_EQ(staccato.placement(), Placement::Auto) << "marking placement should default to auto";
    staccato.set_placement(Placement::Above);
    EXPECT_EQ(nlohmann::json::parse(staccato.dump())["placement"], "above");

    auto fermata = event.ensure_fermata();
    EXPECT_EQ(fermata.placement(), Placement::Auto) << "fermata placement should default to auto";
    fermata.set_placement(Placement::Below);

    auto slur = event.ensure_slurs().append("e1");
    EXPECT_EQ(slur.side(), SlurTieSide::Auto) << "slur side should default to auto";
    EXPECT_EQ(slur.sideEnd(), SlurTieSide::Auto) << "slur sideEnd should default to auto";
    EXPECT_FALSE(nlohmann::json::parse(slur.dump()).contains("side")) << "default slur side should not be serialized";
    slur.set_side(SlurTieSide::Auto);
    EXPECT_EQ(nlohmann::json::parse(slur.dump())["side"], "auto");

    auto tuplet = sequence.content().appendTuplet(NoteValueQuantity::make(3, NoteValue::make(NoteValueBase::Eighth)),
        NoteValueQuantity::make(2, NoteValue::make(NoteValueBase::Eighth)));
    EXPECT_EQ(tuplet.placement(), Placement::Auto) << "tuplet placement should default to auto";
    tuplet.set_placement(Placement::Above);
    for (int i = 0; i < 3; i++) {
        tuplet.content().appendEvent(NoteValueBase::Eighth).ensure_rest();
    }

    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate with placements and caesura";
}
