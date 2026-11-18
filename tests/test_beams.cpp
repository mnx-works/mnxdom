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

#include <map>

#include "gtest/gtest.h"

#include "mnxdom.h"
#include "test_utils.h"

TEST(Beams, SecondaryStartLevels)
{
    const auto inputPath = getInputPath() / std::filesystem::path("test_cases") / "beams_secondary_map.json";
    auto doc = mnx::Document::create(inputPath);
    fullValidate(doc, inputPath);
    doc.buildEntityMapFromIndex(mnx::validation::schemaValidate(doc, withIdCollection).ids.value());

    const std::map<std::string, int> expectedLevels{
        {"ev1", 1},
        {"ev2", 0},
        {"ev3", 3},
        {"ev4", 0},
        {"ev5", 2},
        {"ev6", 0},
        {"ev7", 3},
        {"ev8", 0}
    };

    int seen = 0;
    for (const auto part : doc.parts()) {
        for (const auto measure : part.measures()) {
            for (const auto sequence : measure.sequences()) {
                mnx::util::SequenceWalkHooks hooks;
                hooks.onEvent = [&](const mnx::sequence::Event& event,
                                    const mnx::FractionValue&,
                                    const mnx::FractionValue&,
                                    mnx::util::SequenceWalkContext&) -> bool {
                    if (!event.id()) {
                        return true;
                    }
                    if (!event.id()) {
                        ADD_FAILURE() << "Encountered unnamed event.";
                        return false;
                    }
                    const auto beam = doc.getEntityMap().tryGetBeam(event);
                    EXPECT_TRUE(beam.has_value()) << "Expected beam mapping for " << event.id().value();
                    const std::string eventId = event.id().value();
                    const int actual = doc.getEntityMap().getBeamStartLevel(eventId);
                    const auto it = expectedLevels.find(eventId);
                    if (it == expectedLevels.end()) {
                        ADD_FAILURE() << "Unexpected event " << eventId;
                        return false;
                    }
                    EXPECT_EQ(actual, it->second) << "Unexpected start level for " << eventId;
                    ++seen;
                    return true;
                };
                const bool walked = mnx::util::walkSequenceContent(sequence, hooks);
                ASSERT_TRUE(walked);
            }
        }
    }

    EXPECT_EQ(seen, static_cast<int>(expectedLevels.size())) << "Not all expected beam levels were seen.";
}
