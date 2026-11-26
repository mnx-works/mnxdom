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

TEST(Document, Minimal)
{
    std::istringstream jsonString(R"(
        {
            "mnx": {
                "version": 1,
                "support": {
                    "useAccidentalDisplay" : true
                }
            },
            "global": {
              "measures": []
            },
            "parts": []
        }
    )");
    Document doc(jsonString);
    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate and return no error";

    const auto mnx = doc.mnx();
    EXPECT_EQ(mnx.version(), 1);
    ASSERT_TRUE(mnx.support().has_value());
    auto support = *mnx.support();
    EXPECT_TRUE(support.useAccidentalDisplay());
    support.set_useAccidentalDisplay(false);
    EXPECT_FALSE(support.useAccidentalDisplay());

    auto measures = doc.global().measures();
    EXPECT_EQ(measures.size(), 0u);
    measures.append();
    ASSERT_EQ(doc.global().measures().size(), 1u);
    measures[0].set_id("m3");  // use non const operator[]
    const auto measuresConst = doc.global().measures();
    const auto measure = measuresConst[0]; // use const operator[]
    EXPECT_EQ(measure.id(), "m3");
    EXPECT_EQ(doc.global().measures()[0].id(), "m3");

    EXPECT_EQ(doc.parts().size(), 0u);
}

TEST(Document, MinimalFromScratch)
{
    Document doc;
    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate and return no error";

    auto mnx = doc.mnx();
    EXPECT_EQ(mnx.version(), getMnxSchemaVersion());
    ASSERT_TRUE(mnx._x().has_value());
    const auto provenance = mnx.mnxdom();
    ASSERT_TRUE(provenance.has_value());
    const auto schemaVersion = provenance->mnxSchema();
    EXPECT_EQ(provenance->mnxSchema(), getMnxSchemaId());
    mnx.set_version(getMnxSchemaVersion() + 1);
    EXPECT_EQ(doc.mnx().version(), getMnxSchemaVersion() + 1);

    auto support = mnx.ensure_support();
    support.set_useAccidentalDisplay(true);

    EXPECT_TRUE(support.useAccidentalDisplay()) << "mnx has a support instance";
    doc.mnx().clear_support();
    EXPECT_THROW(static_cast<void>(support.useAccidentalDisplay()), json::out_of_range)
            << "document no longer has a support instance, so the support instance is stale";
    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate without a support instance";

    auto parts = doc.parts();
    EXPECT_EQ(parts.size(), 0u);
    auto part = parts.append();
    EXPECT_EQ(doc.parts().size(), 1u);
    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate after adding a part";
    auto measures = part.create_measures();
    measures.append();
    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate after adding a measure to a part";

    auto layouts = doc.ensure_layouts();
    layouts.append();
    auto layout = layouts[0];
    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate after adding a layout, even with no layout id";
    layout.set_id("layout0");
    auto content = layout.content();
    auto staff = content.appendStaff();
    staff.set_symbol(LayoutSymbol::Bracket);
    EXPECT_EQ(staff.symbol(), LayoutSymbol::Bracket);
    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate after adding a layout";
    //std::cout << doc.dump(4) << std::endl;
}

TEST(Document, MnxdomProvenance)
{
    Document doc;

    auto mnx = doc.mnx();
    auto provenance = mnx.ensure_mnxdom();
    EXPECT_EQ(provenance.provenanceSchemaVersion(), 1);
    EXPECT_EQ(provenance.mnxSchema(), getMnxSchemaId());

    auto mnxdom = provenance.mnxdom();
    EXPECT_EQ(mnxdom.version(), MNXDOM_VERSION);
    const std::string commit = MNXDOM_GIT_COMMIT;
    if (commit.empty()) {
        EXPECT_FALSE(mnxdom.commit().has_value());
    } else {
        EXPECT_EQ(mnxdom.commit().value_or(""), commit);
    }
    const std::string build = MNXDOM_BUILD_ID;
    if (build.empty()) {
        EXPECT_FALSE(mnxdom.build().has_value());
    } else {
        EXPECT_EQ(mnxdom.build().value_or(""), build);
    }

    EXPECT_FALSE(provenance.createdAt().empty());
    provenance.set_documentId("urn:uuid:123e4567-e89b-12d3-a456-426614174000");

    auto client = provenance.ensure_client();
    client.set_name("MyNotationApp");
    client.set_version("2.4.1");

    auto source = provenance.ensure_source();
    source.set_format("musicxml");
    source.set_version("4.0");
    source.set_filename("source.musicxml");

    ASSERT_TRUE(mnx.mnxdom().has_value());
    EXPECT_FALSE(mnx.mnxdom()->createdAt().empty());
    EXPECT_EQ(mnx.mnxdom()->client()->name(), "MyNotationApp");
    EXPECT_TRUE(validation::schemaValidate(doc)) << "schema should validate with mnxdom provenance";
}

TEST(Document, MissingRequiredFields)
{
    std::istringstream jsonString(R"(
        {
            "mnx": {
            },
            "global": {
            },
            "parts": []
        }
    )");
    Document doc(jsonString);
    EXPECT_FALSE(validation::schemaValidate(doc)) << "schema should not validate";

    auto mnx = doc.mnx();
    EXPECT_THROW(static_cast<void>(mnx.version()), std::runtime_error);
    mnx.set_version(getMnxSchemaVersion());
    EXPECT_EQ(doc.mnx().version(), getMnxSchemaVersion());
    EXPECT_FALSE(validation::schemaValidate(doc)) << "after adding version, schema should still not validate";

    auto global = doc.global();
    EXPECT_THROW(static_cast<void>(global.measures()), std::out_of_range);
    global.create_measures();
    EXPECT_EQ(doc.global().measures().size(), 0u);
    EXPECT_TRUE(validation::schemaValidate(doc)) << "after adding global, schema should validate";

    EXPECT_EQ(doc.parts().size(), 0u);
    auto part = doc.parts().append();
    EXPECT_TRUE(validation::schemaValidate(doc)) << "after adding part, schema should validate";
    EXPECT_EQ(part.staves(), 1);
    ASSERT_EQ(doc.parts().size(), 1u);
    doc.parts()[0].set_staves(3);
    doc.parts()[0].set__c("This is a comment.");
    EXPECT_EQ(part.staves(), 3) << "detached instance should reflect changed value from doc";
    EXPECT_TRUE(validation::schemaValidate(doc)) << "after adding comment, schema should validate";
}

namespace {
template <typename T>
concept HasGlobalAttributes = requires(const T& t) {
    t._c();
    t._x();
    t.id();
};
} // namespace

TEST(Document, GlobalAttributes)
{
    // Schema objects without global-attrs do not expose _c, _x, or id.
    static_assert(!HasGlobalAttributes<PlainObject>);
    static_assert(!HasGlobalAttributes<IdPair>);
    static_assert(!HasGlobalAttributes<Dictionary<part::KitComponent>>);
    static_assert(std::is_same_v<decltype(std::declval<const Object&>()._x()), std::optional<PlainObject>>);
    static_assert(HasGlobalAttributes<Object>);
    static_assert(std::is_default_constructible_v<PlainObject>);
    static_assert(std::is_default_constructible_v<Object>); // implicit; calls PlainObject()
    EXPECT_TRUE(Object().empty()); // detached, empty JSON object
    static_assert(HasGlobalAttributes<Part>);

    Document doc;
    auto part = doc.parts().append();
    EXPECT_FALSE(part.getExtension("vendor"));
    part.setExtension("vendor", json{{"key", 1}});
    ASSERT_TRUE(part._x());
    EXPECT_EQ(part.getExtension("vendor"), (json{{"key", 1}}));
    part.clearExtension("vendor");
    EXPECT_FALSE(part.getExtension("vendor"));
    EXPECT_FALSE(part._x()); // empty _x is removed
}
