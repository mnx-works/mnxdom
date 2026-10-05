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

TEST(Parts, StaffConfigs)
{
    Document doc;
    doc.global().measures().append().set_id("m1");
    auto part = doc.parts().append();
    part.set_id("P1");
    part.set_staves(2);
    auto measure = part.measures().append();
    EXPECT_FALSE(measure.staffConfigs()) << "staffConfigs should be omitted until created";

    auto staffConfigs = measure.ensure_staffConfigs();
    auto staffConfig = staffConfigs.append();
    EXPECT_EQ(staffConfig.staff(), 1) << "staff should default to 1";
    EXPECT_FALSE(staffConfig.position()) << "position should default to start of measure";
    EXPECT_EQ(staffConfig.config().lines(), 5u) << "lines should default to 5";
    EXPECT_EQ(staffConfig.config().dump(), "{}") << "default lines should not be serialized";

    staffConfig.set_staff(2);
    staffConfig.config().set_lines(1);
    staffConfig.ensure_position(FractionValue(1, 4));
    EXPECT_EQ(staffConfig.staff(), 2);
    EXPECT_EQ(staffConfig.config().lines(), 1u);
    ASSERT_TRUE(staffConfig.position());
    EXPECT_EQ(staffConfig.position()->fraction(), FractionValue(1, 4));

    staffConfigs.append().config().set_lines(0);
    EXPECT_EQ(staffConfigs.size(), 2u);
    EXPECT_EQ(staffConfigs[1].config().lines(), 0u) << "0 lines is a valid value";

    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate after adding staff configs";
}

TEST(Parts, ClefSignAndHide)
{
    Document doc;
    doc.global().measures().append().set_id("m1");
    auto part = doc.parts().append();
    part.set_id("P1");
    auto measure = part.measures().append();

    auto clef = measure.ensure_clefs().append(ClefSign::PercussionClef, 0).clef();
    EXPECT_EQ(clef.sign(), ClefSign::PercussionClef);
    EXPECT_EQ(nlohmann::json::parse(clef.dump())["sign"], "P") << "percussion clef should serialize as \"P\"";
    EXPECT_FALSE(clef.hide()) << "hide should default to false";
    EXPECT_FALSE(nlohmann::json::parse(clef.dump()).contains("hide")) << "default hide should not be serialized";

    clef.set_hide(true);
    EXPECT_TRUE(clef.hide());

    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate with a hidden percussion clef";
}

TEST(Parts, Placements)
{
    Document doc;
    doc.global().measures().append().set_id("m1");
    auto part = doc.parts().append();
    part.set_id("P1");
    auto measure = part.measures().append();

    auto ottava = measure.ensure_ottavas().append(OttavaAmount::OctaveUp, FractionValue(0),
        MeasureRhythmicPosition::make("m1", FractionValue(1, 4)));
    EXPECT_EQ(ottava.placement(), Placement::Auto) << "ottava placement should default to auto";
    ottava.set_placement(Placement::Below);
    EXPECT_EQ(nlohmann::json::parse(ottava.dump())["placement"], "below");

    auto counter = measure.ensure_measureRepeat(1).ensure_counter(2);
    EXPECT_EQ(counter.placement(), MultiStaffPlacement::Auto) << "counter placement should default to auto";
    counter.set_placement(MultiStaffPlacement::Between);
    EXPECT_EQ(nlohmann::json::parse(counter.dump())["placement"], "between");

    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate with placements";
}

TEST(Parts, DynamicsByType)
{
    Document doc;
    doc.global().measures().append().set_id("m1");
    auto part = doc.parts().append();
    part.set_id("P1");
    auto dynamics = part.measures().append().ensure_dynamics();

    auto immediate = dynamics.appendImmediate(DynamicValue::p, FractionValue(0));
    immediate.set_id("dImmediate");
    immediate.ensure_glyphs().push_back("dynamicPiano");
    EXPECT_EQ(immediate.value(), DynamicValue::p);
    EXPECT_TRUE(immediate.calcHasImmediateText());

    auto gradual = dynamics.appendGradual(DynamicWedgeType::Increasing, FractionValue(1, 4),
        MeasureRhythmicPosition::make("m1", FractionValue(3, 4)));
    gradual.set_id("dGradual");
    EXPECT_FALSE(gradual.value()) << "gradual value is optional";
    EXPECT_FALSE(gradual.calcHasImmediateText()) << "bare hairpin has no immediate text";
    gradual.set_value(DynamicValue::p);
    EXPECT_TRUE(gradual.calcHasImmediateText());
    gradual.set_visuallyContinues("dImmediate");

    auto relative = dynamics.appendRelative(DynamicRelativeValue::Softer, FractionValue(3, 4));
    relative.set_id("dRelative");
    relative.set_prefix("più");
    EXPECT_TRUE(relative.calcHasImmediateText());

    auto accent = dynamics.appendAccent(DynamicValue::f, FractionValue(3, 4));
    accent.set_id("dAccent");
    accent.set_residualValue(DynamicValue::p);
    EXPECT_EQ(accent.value(), DynamicValue::f);
    EXPECT_TRUE(accent.calcHasImmediateText());

    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate each dynamic type";

    doc.buildEntityMap();
    const auto& entities = doc.getEntityMap();
    for (const auto* id : { "dImmediate", "dGradual", "dRelative", "dAccent" }) {
        EXPECT_NO_THROW(entities.get<part::DynamicGroupBase>(id)) << "base lookup should accept " << id;
    }
    EXPECT_EQ(entities.get<part::DynamicImmediate>("dImmediate").value(), DynamicValue::p);
    EXPECT_EQ(entities.get<part::DynamicGradual>("dGradual").wedgeType(), DynamicWedgeType::Increasing);
    EXPECT_EQ(entities.get<part::DynamicRelative>("dRelative").relativeValue(), DynamicRelativeValue::Softer);
    EXPECT_EQ(entities.get<part::DynamicAccent>("dAccent").residualValue(), DynamicValue::p);
    EXPECT_THROW(entities.get<part::DynamicRelative>("dImmediate"), util::mapping_error);

    // mnxdom exposes these ahead of the MNX schema; see MNX_GAPS.md.
    relative.set_value(DynamicValue::p);
    EXPECT_EQ(relative.value(), DynamicValue::p);
    gradual.ensure_glyphs().push_back("dynamicPiano");
    ASSERT_TRUE(gradual.glyphs());
    EXPECT_EQ(gradual.glyphs()->size(), 1u);
}
