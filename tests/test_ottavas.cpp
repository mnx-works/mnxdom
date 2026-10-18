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

#include "gtest/gtest.h"

#include "mnxdom.h"
#include "test_utils.h"

namespace {

struct EventExpectation
{
    std::string id;
    int expectedShift;
};

std::vector<EventExpectation> makeExpectations(const std::vector<EventExpectation>& values)
{
    return values;
}

void expectOttavaShifts(const std::filesystem::path& relativePath,
                        const std::vector<EventExpectation>& expectations,
                        const mnx::EntityMapPolicies* policies = nullptr)
{
    const auto inputPath = getInputPath() / relativePath;
    auto doc = mnx::Document::create(inputPath);
    fullValidate(doc, inputPath);
    if (policies) {
        doc.buildEntityMap(*policies);
    } else {
        doc.buildEntityMap();
    }
    int seen = 0;
    for (const auto& part : doc.parts()) {
        for (const auto measure : part.measures()) {
            for (const auto sequence : measure.sequences()) {
                auto hooks = mnx::util::SequenceWalkHooks{};
                hooks.onEvent = [&](const mnx::sequence::Event& event,
                                    const mnx::FractionValue& start,
                                    const mnx::FractionValue& duration,
                                    mnx::util::SequenceWalkContext& ctx) {
                    (void)start;
                    (void)duration;
                    (void)ctx;
                    if (!event.id()) {
                        return true;
                    }
                    const auto eventId = event.id().value();
                    auto it = std::find_if(expectations.begin(), expectations.end(),
                                           [&](const EventExpectation& exp) {
                                               return exp.id == eventId;
                                           });
                    if (it != expectations.end()) {
                        const int actual = doc.getEntityMap().getOttavaShift(event);
                        EXPECT_EQ(actual, it->expectedShift) << "Event " << eventId
                            << " in " << pathString(relativePath);
                        ++seen;
                    }
                    return true;
                };
                auto walked = mnx::util::walkSequenceContent(sequence, hooks);
                ASSERT_TRUE(walked);
            }
        }
    }
    EXPECT_EQ(seen, static_cast<int>(expectations.size()))
        << "Not all event expectations were encountered in " << pathString(relativePath);
}
} // namespace

TEST(Ottavas, GraceBoundaries)
{
    expectOttavaShifts(
        std::filesystem::path("test_cases") / "ottavas_grace_boundaries.json",
        makeExpectations({
            {"gr1", -1},
            {"gr2", -1},
            {"m1e2", -1},
            {"m1e3", 1},
            {"gr3", 1},
            {"m1e4", 1}
        }));
}

TEST(Ottavas, StaffVoiceSpecific)
{
    expectOttavaShifts(
        std::filesystem::path("test_cases") / "ottavas_staff_voice_specific.json",
        makeExpectations({
            {"rh1", -1},
            {"rh2", -1},
            {"rh3", -1},
            {"rh4", -3},
            {"mel1", -1},
            {"mel2", -1},
            {"lh1", 0},
            {"lh2", 0}
        }));
}

TEST(Ottavas, Overlapping)
{
    expectOttavaShifts(
        std::filesystem::path("test_cases") / "ottavas_overlapping.json",
        makeExpectations({
            {"m1e1", -1},
            {"m1e2", -1},
            {"m1e3", -1},
            {"m1e4", -3},
            {"m2e1", -1},
            {"m2e2", 0},
            {"m2e3", 0},
            {"m2e4", 0}
        }));
}

TEST(Ottavas, CrossMeasureInclusive)
{
    expectOttavaShifts(
        std::filesystem::path("test_cases") / "ottavas_cross_measure_ties.json",
        makeExpectations({
            {"tie_start", 0},
            {"tie_end", -1},
            {"cont1", 0},
            {"cont2", 0},
            {"cont3", 0}
        }));
}

TEST(Ottavas, ComplexScore)
{
    expectOttavaShifts(
        std::filesystem::path("test_cases") / "ottavas_complex.json",
        makeExpectations({
            {"fl1", 0},
            {"fl2", -1},
            {"fl3", -1},
            {"fl4", -1},
            {"fl5", -1},
            {"fl6", -1},
            {"fl7", 1},
            {"pn1", 0},
            {"pn3", -1},
            {"pn4", -1},
            {"pn5", 0},
            {"pn7", 0},
            {"pn9", -2},
            {"pn10", 1},
            {"pn13", 0},
            {"pn17", 1}
        }));
}

TEST(Ottavas, GraceAtMeasureEnd)
{
    const auto relative = std::filesystem::path("test_cases") / "ottavas_grace_measure_end.json";
    expectOttavaShifts(
        relative,
        makeExpectations({
            {"end_gr1", -1},
            {"end_gr2", -1}
        }));

    mnx::EntityMapPolicies policies;
    policies.ottavasRespectGraceTargets = false;
    expectOttavaShifts(
        relative,
        makeExpectations({
            {"end_gr1", -1},
            {"end_gr2", -1}
        }),
        &policies);
}

TEST(Ottavas, GraceWithSpaceHost)
{
    expectOttavaShifts(
        std::filesystem::path("test_cases") / "ottavas_space_host.json",
        makeExpectations({
            {"grs1", -1},
            {"grs2", -1}
        }));
}

TEST(Ottavas, PolicyGraceFallback)
{
    mnx::EntityMapPolicies policies;
    policies.ottavasRespectGraceTargets = false;
    expectOttavaShifts(
        std::filesystem::path("test_cases") / "ottavas_grace_boundaries.json",
        makeExpectations({
            {"gr3", 1}
        }),
        &policies);
}

TEST(Ottavas, PolicyVoiceFallback)
{
    mnx::EntityMapPolicies policies;
    policies.ottavasRespectVoiceTargets = false;
    expectOttavaShifts(
        std::filesystem::path("test_cases") / "ottavas_voice_policy.json",
        makeExpectations({
            {"up1", -1},
            {"low1", -1},
            {"low2", -1}
        }),
        &policies);
}
