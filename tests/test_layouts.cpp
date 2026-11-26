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

TEST(Layouts, FullScoreDetect)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "test_cases" / "chamberorch.json";
    auto doc = mnx::Document::create(inputPath);
    EXPECT_TRUE(fullValidate(doc, inputPath));

    auto fullScoreLayout = doc.findFullScoreLayout();
    ASSERT_TRUE(fullScoreLayout);
    EXPECT_EQ(fullScoreLayout->id_or(""), "S0-ScrVw");
}

TEST(Layouts, BuildLayoutSpans)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "layoutspans.json";
    auto doc = mnx::Document::create(inputPath);
    EXPECT_TRUE(validateSchema(doc, inputPath)) << "schema validatation failed"; // empty group means semantic fails: tested elsewhere

    doc.buildEntityMapFromIndex(mnx::validation::schemaValidate(doc, withIdCollection).ids.value());
    auto layout = doc.getEntityMap().tryGet<mnx::Layout>("S0-ScrVw");
    ASSERT_TRUE(layout);

    auto spans = util::buildLayoutSpans(layout.value());
    // Expected (after sorting):
    // Group A: [0,3], depth 0
    // Staff S0: [0,0], depth 2
    // Group B: [1,2], depth 1
    // Staff S1: [1,1], depth 3
    // Staff S2: [2,2], depth 3
    // Staff S3: [3,3], depth 2
    ASSERT_EQ(spans->size(), 6u);

    const auto& s = *spans;

    EXPECT_EQ(s[0].kind, mnx::util::LayoutSpan::Kind::Group);
    EXPECT_EQ(s[0].startIndex, 0u);
    EXPECT_EQ(s[0].endIndex, 3u);
    EXPECT_EQ(s[0].depth, 0u);
    ASSERT_TRUE(s[0].label.has_value());
    EXPECT_EQ(*s[0].label, "A");

    EXPECT_EQ(s[1].kind, mnx::util::LayoutSpan::Kind::Staff);
    EXPECT_EQ(s[1].startIndex, 0u);
    EXPECT_EQ(s[1].endIndex, 0u);
    EXPECT_EQ(s[1].depth, 2u);
    ASSERT_TRUE(s[1].label.has_value());
    EXPECT_EQ(*s[1].label, "S0");

    EXPECT_EQ(s[2].kind, mnx::util::LayoutSpan::Kind::Group);
    EXPECT_EQ(s[2].startIndex, 1u);
    EXPECT_EQ(s[2].endIndex, 2u);
    EXPECT_EQ(s[2].depth, 1u);
    ASSERT_TRUE(s[2].label.has_value());
    EXPECT_EQ(*s[2].label, "B");

    EXPECT_EQ(s[3].kind, mnx::util::LayoutSpan::Kind::Staff);
    EXPECT_EQ(s[3].startIndex, 1u);
    EXPECT_EQ(s[3].endIndex, 1u);
    EXPECT_EQ(s[3].depth, 3u);
    ASSERT_TRUE(s[3].label.has_value());
    EXPECT_EQ(*s[3].label, "S1");

    EXPECT_EQ(s[4].kind, mnx::util::LayoutSpan::Kind::Staff);
    EXPECT_EQ(s[4].startIndex, 2u);
    EXPECT_EQ(s[4].endIndex, 2u);
    EXPECT_EQ(s[4].depth, 3u);
    ASSERT_TRUE(s[4].label.has_value());
    EXPECT_EQ(*s[4].label, "S2");

    EXPECT_EQ(s[5].kind, mnx::util::LayoutSpan::Kind::Staff);
    EXPECT_EQ(s[5].startIndex, 3u);
    EXPECT_EQ(s[5].endIndex, 3u);
    EXPECT_EQ(s[5].depth, 2u);
    ASSERT_TRUE(s[5].label.has_value());
    EXPECT_EQ(*s[5].label, "S3");

    ASSERT_TRUE(s[0].labelRef.has_value());
    EXPECT_EQ(s[0].labelRef->ref, LabelRefType::ShortName);
    EXPECT_EQ(s[0].labelRef->target, "P1");
    EXPECT_FALSE(s[1].labelRef.has_value()); // labelRef on S0 is on its source, not the staff
    ASSERT_TRUE(s[2].labelRef.has_value());   // group B: omitted target is left to the application
    EXPECT_EQ(s[2].labelRef->ref, LabelRefType::Auto);
    EXPECT_FALSE(s[2].labelRef->target);
    ASSERT_TRUE(s[3].labelRef.has_value());   // staff S1: omitted target falls back to the first source's part
    EXPECT_EQ(s[3].labelRef->ref, LabelRefType::ShortName);
    EXPECT_EQ(s[3].labelRef->target, "P2");

    EXPECT_EQ(s[0].barlineOverride, StaffGroupBarlineOverride::None);
    EXPECT_EQ(s[1].barlineOverride, StaffGroupBarlineOverride::None);
    EXPECT_EQ(s[2].barlineOverride, StaffGroupBarlineOverride::Unified);
    EXPECT_EQ(s[3].barlineOverride, StaffGroupBarlineOverride::None);
    EXPECT_EQ(s[4].barlineOverride, StaffGroupBarlineOverride::None);
    EXPECT_EQ(s[5].barlineOverride, StaffGroupBarlineOverride::None);
}

TEST(Layouts, StaffGroupBarlineOverride)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "layoutspans.json";
    auto doc = mnx::Document::create(inputPath);
    EXPECT_TRUE(validateSchema(doc, inputPath)) << "schema validation failed";

    doc.buildEntityMapFromIndex(mnx::validation::schemaValidate(doc, withIdCollection).ids.value());
    auto layout = doc.getEntityMap().tryGet<mnx::Layout>("S0-ScrVw");
    ASSERT_TRUE(layout);
    ASSERT_EQ(layout->content().size(), 1u);

    auto groupA = layout->content()[0].get<layout::Group>();
    EXPECT_FALSE(groupA.calcIsPartGroup());
    EXPECT_EQ(groupA.calcBarlineOverride(), StaffGroupBarlineOverride::None);

    ASSERT_EQ(groupA.content().size(), 4u);
    auto groupB = groupA.content()[1].get<layout::Group>();
    EXPECT_TRUE(groupB.calcIsPartGroup());
    EXPECT_EQ(groupB.calcBarlineOverride(), StaffGroupBarlineOverride::Unified);

    groupB.set_barlineStyle(StaffGroupBarlineStyle::Unified);
    EXPECT_EQ(groupB.calcBarlineOverride(), StaffGroupBarlineOverride::Unified);

    groupB.set_barlineStyle(StaffGroupBarlineStyle::Mensurstrich);
    EXPECT_EQ(groupB.calcBarlineOverride(), StaffGroupBarlineOverride::Mensurstrich);

    groupB.set_barlineStyle(StaffGroupBarlineStyle::Individual);
    EXPECT_EQ(groupB.calcBarlineOverride(), StaffGroupBarlineOverride::None);
}

TEST(Layouts, SquareBracket)
{
    const auto doc = mnx::Document::create(std::filesystem::path(MNX_W3C_EXAMPLES_PATH) / "bracket-square.json");
    auto group = doc.layouts().value()[0].content().get<mnx::layout::Group>(0)
        .content().get<mnx::layout::Group>(1);
    EXPECT_EQ(group.symbol(), mnx::LayoutSymbol::Square);
    group.set_symbol(mnx::LayoutSymbol::Square);
    EXPECT_EQ(mnx::json::parse(group.dump())["symbol"], "square");
    EXPECT_TRUE(mnx::validation::schemaValidate(doc));
}

TEST(Layouts, LabelRef)
{
    auto doc = mnx::Document();
    auto layout = doc.ensure_layouts().append();
    auto group = layout.content().appendGroup();
    auto staff = group.content().appendStaff();
    auto source = staff.sources().append("P1");

    EXPECT_FALSE(group.labelRef());
    EXPECT_FALSE(staff.labelRef());
    EXPECT_FALSE(source.labelRef());

    auto sourceRef = source.ensure_labelRef();
    EXPECT_EQ(sourceRef.ref(), LabelRefType::Auto); // MNX default
    EXPECT_FALSE(sourceRef.target());
    EXPECT_EQ(json::parse(sourceRef.dump()), json::object()); // default is not serialized

    sourceRef.set_ref(LabelRefType::Name);
    EXPECT_EQ(json::parse(sourceRef.dump()), (json{{"ref", "name"}}));

    auto staffRef = staff.ensure_labelRef();
    staffRef.set_ref(LabelRefType::ShortName);
    staffRef.set_target("P1");
    EXPECT_EQ(json::parse(staffRef.dump()), (json{{"ref", "shortName"}, {"target", "P1"}}));

    group.ensure_labelRef().set_ref(LabelRefType::Auto);
    EXPECT_EQ(json::parse(group.labelRef()->dump()), (json{{"ref", "auto"}}));

    ASSERT_TRUE(staff.labelRef());
    EXPECT_EQ(staff.labelRef()->target(), "P1");
    staff.clear_labelRef();
    EXPECT_FALSE(staff.labelRef());
}

TEST(Layouts, LayoutSpanLabelRefIsIndependent)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "layoutspans.json";
    std::optional<std::vector<util::LayoutSpan>> spans;
    {
        auto doc = mnx::Document::create(inputPath);
        doc.buildEntityMapFromIndex(mnx::validation::schemaValidate(doc, withIdCollection).ids.value());
        auto layout = doc.getEntityMap().tryGet<mnx::Layout>("S0-ScrVw");
        ASSERT_TRUE(layout);
        spans = util::buildLayoutSpans(layout.value());
        ASSERT_TRUE(spans);
        ASSERT_EQ(spans->size(), 6u);

        // The staff fallback target is applied to the span, not the document.
        auto staffS1 = layout->content()[0].get<layout::Group>().content()[1].get<layout::Group>()
                           .content()[0].get<layout::Staff>();
        ASSERT_TRUE(staffS1.labelRef());
        EXPECT_FALSE(staffS1.labelRef()->target());
    }
    // The document is gone; spans remain valid and freely assignable.
    EXPECT_EQ((*spans)[0].labelRef->ref, LabelRefType::ShortName);
    EXPECT_EQ((*spans)[0].labelRef->target, "P1");
    EXPECT_EQ((*spans)[3].labelRef->target, "P2");
    (*spans)[0] = (*spans)[3];
    EXPECT_EQ((*spans)[0].labelRef->target, "P2");
    auto copy = *spans;
    *spans = copy;
    EXPECT_EQ((*spans)[3].labelRef->ref, LabelRefType::ShortName);
}

TEST(Layouts, BuildDefaultLayoutSpans)
{
    auto doc = mnx::Document();
    auto flute = doc.parts().append();
    doc.assignId(flute, "flute");
    flute.set_name("Flute");
    auto piano = doc.parts().append();
    doc.assignId(piano, "piano");
    piano.set_name("Piano");
    piano.set_staves(2);
    auto noId = doc.parts().append();
    noId.set_name("Unnamed Part");

    const auto spans = util::buildDefaultLayoutSpans(doc.parts());
    // flute staff, piano group, piano staff 1, piano staff 2, no-id staff
    ASSERT_EQ(spans.size(), 5u);

    EXPECT_EQ(spans[0].kind, util::LayoutSpan::Kind::Staff);
    EXPECT_FALSE(spans[0].label);
    ASSERT_TRUE(spans[0].labelRef);
    EXPECT_EQ(spans[0].labelRef->ref, LabelRefType::Auto);
    EXPECT_EQ(spans[0].labelRef->target, "flute");

    EXPECT_EQ(spans[1].kind, util::LayoutSpan::Kind::Group);
    EXPECT_FALSE(spans[1].label);
    ASSERT_TRUE(spans[1].labelRef);
    EXPECT_EQ(spans[1].labelRef->ref, LabelRefType::Auto);
    EXPECT_EQ(spans[1].labelRef->target, "piano");

    // the piano group carries the label, so its staves do not
    EXPECT_FALSE(spans[2].label);
    EXPECT_FALSE(spans[2].labelRef);
    EXPECT_FALSE(spans[3].label);
    EXPECT_FALSE(spans[3].labelRef);

    // without a part id, a label ref cannot identify the part, so the name is used
    EXPECT_FALSE(spans[4].labelRef);
    EXPECT_EQ(spans[4].label, "Unnamed Part");
}
