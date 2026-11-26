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

using namespace mnx;

TEST(Scores, FromScratch)
{
    Document doc;
    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate and return no error";

    static constexpr int numBars = 5;
    for (int x = 0; x < numBars; x++) {
        auto globalMeasure = doc.global().measures().append();
        doc.assignId(globalMeasure, "m" + std::to_string(x + 1));
    }
    EXPECT_EQ(doc.global().measures().size(), size_t(numBars));

    auto scores = doc.ensure_scores();

    auto score = scores.append("Full Score");
    auto pages = score.ensure_pages();
    auto page = pages.append();
    auto system = page.systems().append("m1");
    EXPECT_EQ(system.measure(), "m1");
    EXPECT_EQ(page.systems().append("m4").measure(), "m4");
    auto layoutChanges = system.ensure_layoutChanges();
    auto layoutChange = layoutChanges.append("layout1", mnx::MeasureRhythmicPosition::make("m2", FractionValue(3, 8)));
    EXPECT_EQ(layoutChange.layout(), "layout1");
    EXPECT_EQ(layoutChange.location().measure(), "m2");

    auto frac = layoutChange.location().position().fraction();
    EXPECT_EQ(frac.numerator(), 3u);
    EXPECT_EQ(frac.denominator(), 8u);

    frac.set_numerator(5);
    EXPECT_EQ(frac.numerator(), 5u);

    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate after adding a score";
    //std::cout << doc.dump(4) << std::endl;
}
