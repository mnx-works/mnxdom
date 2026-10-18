/*
 * Copyright (C) 2026, Robert Patterson
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

namespace {
Document extenderDocument()
{
    setupTestDataPaths();
    return Document::create(getInputPath() / "validation" / "lyric_extenders.json");
}

sequence::Event eventAt(Document& doc, size_t part, size_t measure, size_t event)
{
    return doc.parts()[part].measures()[measure].sequences()[0].content()[event].get<sequence::Event>();
}

sequence::EventLyricLineExtender sourceExtender(Document& doc)
{
    return eventAt(doc, 0, 0, 1).lyrics()->lines()->at("1").extender().value();
}

void expectValid(const Document& doc)
{
    const auto schema = validation::schemaValidate(doc);
    for (const auto& error : schema.errors) {
        ADD_FAILURE() << error.to_string();
    }
    const auto semantic = validation::semanticValidate(doc);
    for (const auto& error : semantic.errors) {
        ADD_FAILURE() << error.to_string();
    }
}
}

TEST(EventTiming, CachedInfoAndLookup)
{
    auto doc = extenderDocument();
    doc.buildEntityMap();
    const auto& map = doc.getEntityMap();
    const auto source = map.getEventInfo("p0m0e1");
    EXPECT_EQ(source.measureIndex, 0u);
    EXPECT_EQ(source.elapsedTime, FractionValue(1, 4));
    EXPECT_EQ(source.actualDuration, FractionValue(1, 4));
    EXPECT_EQ(source.graceIndex, 0u);
    EXPECT_EQ(source.event.id(), "p0m0e1");
    EXPECT_EQ(map.getEventPosition(source.event).fraction, source.elapsedTime);
    EXPECT_EQ(map.compareEventPositions(source, map.getEventInfo("p1m0e1")), 0);
    EXPECT_LT(map.compareEventPositions(source, map.getEventInfo("p0m1e0")), 0);
    EXPECT_GT(map.compareEventPositions(source, map.getEventInfo("p0m0e0")), 0);
    EXPECT_FALSE(map.tryGetEventInfo("missing"));
    EXPECT_THROW((void)map.getEventInfo("missing"), util::mapping_error);
    EXPECT_THROW((void)map.tryGetEventInfo("p0"), util::mapping_error);
    EXPECT_THROW((void)map.getEventInfo("p0"), util::mapping_error);
    auto anonymous = eventAt(doc, 0, 0, 0);
    anonymous.clear_id();
    doc.buildEntityMap();
    EXPECT_EQ(doc.getEntityMap().getEventInfo(anonymous).elapsedTime, FractionValue(0));
    sequence::Event detached(std::make_shared<json>(json::object()), json_pointer{});
    EXPECT_FALSE(doc.getEntityMap().tryGetEventInfo(detached));
    EXPECT_THROW((void)doc.getEntityMap().getEventInfo(detached), std::logic_error);
}

TEST(Extenders, AllowedTargets)
{
    for (const auto* target : {"p0m0e1", "p0m0e2", "p0m1e0", "p1m0e1", "p1m2e3"}) {
        auto doc = extenderDocument();
        sourceExtender(doc).set_target(target);
        expectValid(doc);
        EXPECT_TRUE(validation::semanticValidate(doc).warnings.empty());
    }
    auto doc = extenderDocument();
    sourceExtender(doc).clear_target();
    expectValid(doc);
    sourceExtender(doc).set_target("p0m1e0");
    eventAt(doc, 0, 0, 1).clear_id();
    expectValid(doc);
}

TEST(Extenders, InvalidTargetsAndHiddenTargets)
{
    for (const auto* target : {"missing", "p0", "p0m0e0", "p1m0e0"}) {
        auto doc = extenderDocument();
        sourceExtender(doc).set_target(target);
        EXPECT_TRUE(validation::schemaValidate(doc));
        const auto result = validation::semanticValidate(doc);
        ASSERT_EQ(result.errors.size(), 1u);
        EXPECT_EQ(result.errors[0].pointer, sourceExtender(doc).pointer());
        EXPECT_NE(result.errors[0].message.find(target), std::string::npos);
        sourceExtender(doc).set_show(false);
        expectValid(doc);
        EXPECT_TRUE(validation::semanticValidate(doc).warnings.empty());
    }
    auto doc = extenderDocument();
    auto note = eventAt(doc, 0, 0, 0).notes()->at(0);
    note.set_id("noteId");
    sourceExtender(doc).set_target("noteId");
    EXPECT_FALSE(validation::semanticValidate(doc));
}

TEST(Extenders, AdvisoryWarnings)
{
    for (const auto* target : {"p0m0e2", "p0m1e0", "p1m1e0", "p1m0e1", "p0m0e1"}) {
        auto doc = extenderDocument();
        doc.buildEntityMap();
        auto event = doc.getEntityMap().get<sequence::Event>(target);
        if (!event.lyrics() || !event.lyrics()->lines() || !event.lyrics()->lines()->contains("1")) {
            event.ensure_lyrics().ensure_lines().append("1", "Next");
        }
        sourceExtender(doc).set_target(target);
        const auto result = validation::semanticValidate(doc);
        EXPECT_TRUE(result);
        const bool warning = std::string_view(target) == "p0m0e2" || std::string_view(target) == "p0m1e0";
        EXPECT_EQ(result.warnings.size(), warning ? 1u : 0u);
        if (warning) {
            EXPECT_EQ(result.warnings[0].pointer, sourceExtender(doc).pointer());
            EXPECT_NE(result.warnings[0].message.find(target), std::string::npos);
        }
        sourceExtender(doc).set_show(false);
        EXPECT_TRUE(validation::semanticValidate(doc).warnings.empty());
    }
    auto doc = extenderDocument();
    eventAt(doc, 0, 1, 0).ensure_lyrics().ensure_lines().append("1", "Next");
    doc.parts()[0].measures()[0].sequences()[0].clear_voice();
    doc.parts()[0].measures()[1].sequences()[0].clear_voice();
    EXPECT_TRUE(validation::semanticValidate(doc).warnings.empty());
    doc.parts()[0].measures()[0].sequences()[0].set_voice("firstVoice");
    doc.parts()[0].measures()[1].sequences()[0].set_voice("otherVoice");
    EXPECT_TRUE(validation::semanticValidate(doc).warnings.empty());
    auto sameSequenceDoc = extenderDocument();
    sameSequenceDoc.parts()[0].measures()[0].sequences()[0].clear_voice();
    eventAt(sameSequenceDoc, 0, 0, 2).ensure_lyrics().ensure_lines().append("1", "Next");
    sourceExtender(sameSequenceDoc).set_target("p0m0e2");
    EXPECT_EQ(validation::semanticValidate(sameSequenceDoc).warnings.size(), 1u);
}

TEST(Navigation, RequiredDestinations)
{
    struct Case { JumpType type; bool segno; bool fine; bool coda; };
    const Case cases[] = {
        {JumpType::Dc, false, false, false}, {JumpType::DcAlFine, false, true, false},
        {JumpType::DcAlCoda, false, false, true}, {JumpType::Segno, true, false, false},
        {JumpType::DsAlFine, true, true, false}, {JumpType::DsAlCoda, true, false, true},
        {JumpType::ToCoda, false, false, true}
    };
    for (const auto& item : cases) {
        auto doc = extenderDocument();
        auto measures = doc.global().measures();
        auto jump = measures[2].ensure_jump(item.type, FractionValue(1));
        measures[0].ensure_segno(FractionValue(0));
        measures[0].ensure_fine(FractionValue(1));
        measures[0].ensure_coda(FractionValue(0));
        expectValid(doc);
        for (int destination = 0; destination < 3; ++destination) {
            const bool required = destination == 0 ? item.segno : destination == 1 ? item.fine : item.coda;
            if (destination == 0) measures[0].clear_segno();
            if (destination == 1) measures[0].clear_fine();
            if (destination == 2) measures[0].clear_coda();
            auto result = validation::semanticValidate(doc);
            EXPECT_EQ(result.errors.size(), required ? 1u : 0u);
            if (required) EXPECT_EQ(result.errors[0].pointer, jump.pointer());
            if (destination == 0) {
                measures[0].ensure_segno(FractionValue(0));
                measures[1].ensure_segno(FractionValue(0));
            }
            if (destination == 1) {
                measures[0].ensure_fine(FractionValue(1));
                measures[1].ensure_fine(FractionValue(1));
            }
            if (destination == 2) {
                measures[0].ensure_coda(FractionValue(0));
                measures[1].ensure_coda(FractionValue(0));
            }
            result = validation::semanticValidate(doc);
            EXPECT_EQ(result.errors.size(), required ? 1u : 0u);
            if (destination == 0) measures[1].clear_segno();
            if (destination == 1) measures[1].clear_fine();
            if (destination == 2) measures[1].clear_coda();
        }
    }
}

TEST(Navigation, LocationsAndInheritedMeter)
{
    for (const auto onset : {FractionValue(0), FractionValue(3, 4), FractionValue(1), FractionValue(5, 4)}) {
        Document doc;
        auto measures = doc.global().measures();
        measures.append().ensure_time(3, TimeSignatureUnit::Quarter);
        auto measure = measures.append();
        measure.ensure_coda(onset);
        measure.ensure_segno(onset);
        measure.ensure_fine(onset);
        measure.ensure_jump(JumpType::Dc, onset);
        const auto result = validation::semanticValidate(doc);
        EXPECT_EQ(result.errors.size(), onset >= FractionValue(0) && onset <= FractionValue(3, 4) ? 0u : 4u);
        measures[0].clear_time();
        EXPECT_TRUE(validation::semanticValidate(doc));
        measure.ensure_time(4, TimeSignatureUnit::Quarter);
        const auto finalResult = validation::semanticValidate(doc);
        EXPECT_EQ(finalResult.errors.size(), onset <= FractionValue(1) ? 0u : 4u);
    }
}

TEST(EventTiming, NestedTimingAndCanonicalGracePositions)
{
    setupTestDataPaths();
    auto doc = Document::create(getInputPath() / "validation" / "event_timing.json");
    expectValid(doc);
    doc.buildEntityMap();
    const auto& map = doc.getEntityMap();
    EXPECT_EQ(map.getEventInfo("nested0").elapsedTime, FractionValue(1, 8));
    EXPECT_EQ(map.getEventInfo("nested1").elapsedTime, FractionValue(11, 72));
    EXPECT_EQ(map.getEventInfo("nested0").actualDuration, FractionValue(1, 36));
    EXPECT_EQ(map.getEventInfo("triplet1").elapsedTime, FractionValue(5, 24));
    EXPECT_EQ(map.getEventInfo("triplet1").actualDuration, FractionValue(1, 12));
    EXPECT_EQ(map.getEventInfo("tremolo0").elapsedTime, FractionValue(3, 8));
    EXPECT_EQ(map.getEventInfo("tremolo0").actualDuration, FractionValue(1, 8));
    EXPECT_EQ(map.getEventInfo("tremolo1").elapsedTime, FractionValue(1, 2));
    EXPECT_EQ(map.getEventInfo("tail").elapsedTime, FractionValue(5, 8));
    EXPECT_EQ(map.getEventInfo("grace0").elapsedTime, FractionValue(5, 8));
    EXPECT_EQ(map.getEventInfo("grace0").actualDuration, FractionValue(0));
    EXPECT_EQ(map.getEventInfo("grace0").graceIndex, 2u);
    EXPECT_EQ(map.getEventInfo("grace1").graceIndex, 1u);
    EXPECT_EQ(map.getEventInfo("trailingGrace").elapsedTime, FractionValue(3, 4));
    EXPECT_EQ(map.getEventInfo("trailingGrace").actualDuration, FractionValue(0));
    EXPECT_EQ(map.getEventInfo("second1").measureIndex, 1u);
    std::vector<util::EntityMap::EventInfo> before;
    for (const auto* id : {"nested0", "tremolo1", "grace0", "grace1", "tail", "trailingGrace"}) {
        before.push_back(map.getEventInfo(id));
    }
    EntityMapPolicies policies;
    policies.ottavasRespectGraceTargets = false;
    doc.buildEntityMap(policies);
    for (const auto& info : before) {
        const auto after = doc.getEntityMap().getEventInfo(info.event);
        EXPECT_EQ(after.elapsedTime, info.elapsedTime);
        EXPECT_EQ(after.actualDuration, info.actualDuration);
        EXPECT_EQ(after.graceIndex, info.graceIndex);
        EXPECT_EQ(doc.getEntityMap().getEventPosition(info.event).graceIndex, info.graceIndex);
    }
    doc.buildEntityMap();
    const auto& rebuilt = doc.getEntityMap();
    for (const auto* id : {"nested0", "tremolo0", "grace0", "grace1", "tail"}) {
        auto event = rebuilt.get<sequence::Event>(id);
        auto line = event.ensure_lyrics().ensure_lines().append("1", "Ah");
        line.ensure_extender().set_target("second0");
    }
    expectValid(doc);
    auto grace = doc.getEntityMap().get<sequence::Event>("grace0");
    auto extender = grace.lyrics()->lines()->at("1").extender().value();
    extender.set_target("grace1");
    EXPECT_TRUE(validation::semanticValidate(doc));
    auto other = doc.getEntityMap().get<sequence::Event>("grace1");
    other.lyrics()->lines()->at("1").extender()->set_target("grace0");
    EXPECT_FALSE(validation::semanticValidate(doc));
}

TEST(EventTiming, OttavaPolicyDoesNotChangeCachedGracePositions)
{
    setupTestDataPaths();
    auto doc = Document::create(getInputPath() / "test_cases" / "ottavas_grace_boundaries.json");
    // Begin on the main event: preceding graces are excluded unless grace targeting is disabled.
    auto start = doc.parts()[0].measures()[0].ottavas()->at(1).position();
    start.create_fraction(FractionValue(3, 4));
    start.set_graceIndex(0);
    doc.buildEntityMap();
    auto before = doc.getEntityMap().getEventInfo("gr3");
    const auto shift = doc.getEntityMap().getOttavaShift(before.event);
    EntityMapPolicies policies;
    policies.ottavasRespectGraceTargets = false;
    doc.buildEntityMap(policies);
    auto after = doc.getEntityMap().getEventInfo("gr3");
    EXPECT_EQ(after.elapsedTime, before.elapsedTime);
    EXPECT_EQ(after.graceIndex, before.graceIndex);
    EXPECT_EQ(after.actualDuration, before.actualDuration);
    EXPECT_NE(doc.getEntityMap().getOttavaShift(after.event), shift);
    EXPECT_EQ(doc.getEntityMap().getOttavaShift(after.event), 1);
}

TEST(EventTiming, RhythmicPositionGraceOrder)
{
    const std::optional<unsigned> indices[] = {std::nullopt, 3u, 2u, 1u, 0u};
    for (size_t left = 0; left < std::size(indices); ++left) {
        for (size_t right = 0; right < std::size(indices); ++right) {
            RhythmicPosition lhs(FractionValue(1, 4));
            RhythmicPosition rhs(FractionValue(1, 4));
            if (indices[left]) lhs.set_graceIndex(*indices[left]);
            if (indices[right]) rhs.set_graceIndex(*indices[right]);
            const int expected = left < right ? -1 : left > right ? 1 : 0;
            EXPECT_EQ(RhythmicPosition::compare(lhs, rhs), expected);
            rhs.create_fraction(FractionValue(1, 2));
            EXPECT_LT(RhythmicPosition::compare(lhs, rhs), 0);
        }
    }
    RhythmicPosition grace(FractionValue(1));
    RhythmicPosition main(FractionValue(1));
    grace.set_graceIndex(2u);
    main.set_graceIndex(0u);
    EXPECT_LT(RhythmicPosition::compare(grace, main), 0);
    EXPECT_EQ(RhythmicPosition::compare(grace, main, true), 0);
}

TEST(EventTiming, OttavaSpanBetweenSpecificGraceNotes)
{
    setupTestDataPaths();
    auto doc = Document::create(getInputPath() / "test_cases" / "ottavas_grace_boundaries.json");
    auto span = doc.parts()[0].measures()[0].ottavas()->at(0);
    span.position().set_graceIndex(2);
    span.end().position().create_fraction(FractionValue(1, 4));
    span.end().position().set_graceIndex(1);
    expectValid(doc);
    doc.buildEntityMap();
    const auto& map = doc.getEntityMap();
    EXPECT_EQ(map.getOttavaShift(map.get<sequence::Event>("gr1")), -1);
    EXPECT_EQ(map.getOttavaShift(map.get<sequence::Event>("gr2")), -1);
    EXPECT_EQ(map.getOttavaShift(map.get<sequence::Event>("m1e2")), 0);
    EXPECT_EQ(map.getOttavaShift(map.get<sequence::Event>("m1e1")), 0);
}

TEST(Extenders, ActualTimeAndGraceOrder)
{
    setupTestDataPaths();
    struct Case { const char* source; const char* target; bool valid; };
    const Case cases[] = {
        {"nested2", "triplet1", true}, {"triplet1", "nested2", false},
        {"tremolo1", "tail", true}, {"tail", "tremolo1", false},
        {"grace0", "grace1", true}, {"grace1", "grace0", false},
        {"grace1", "tail", true}, {"tail", "grace1", false},
        {"tail", "second0", true}, {"second0", "tail", false}
    };
    for (const auto& item : cases) {
        SCOPED_TRACE(std::string(item.source) + " -> " + item.target);
        auto doc = Document::create(getInputPath() / "validation" / "event_timing.json");
        doc.buildEntityMap();
        auto source = doc.getEntityMap().get<sequence::Event>(item.source);
        source.ensure_lyrics().ensure_lines().append("1", "Ah").ensure_extender().set_target(item.target);
        EXPECT_TRUE(validation::schemaValidate(doc));
        const auto result = validation::semanticValidate(doc);
        EXPECT_EQ(bool(result), item.valid);
        EXPECT_EQ(result.errors.size(), item.valid ? 0u : 1u);
    }
}
