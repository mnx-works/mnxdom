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

TEST(Layouts, DuplicateId)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "duplicate_layouts.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "ID \"S0-ScrVw\" already exists for type \"system-layout\" at /layouts/0");
}

TEST(Layouts, NonexistentPartId)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "layout_with_bad_part.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "ID \"P-does-not-exist\" not found in ID mapping");
}

TEST(Layouts, EmptyPartId)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "layout_with_empty_part.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "Layout staff \"<no-id>\" has empty part id in source.", /*skipSchema*/ true);
}

TEST(Layouts, NonexistentStaffNumber)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "layout_invalid_staffnum.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath,  "ID \"P2\" not found in ID mapping");
}

TEST(Layouts, InvalidPartDistributionInStaff)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "orchestral_layout_bad.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "Layout staff \"<no-id>\" has one or more part voices specified multiple times");
}

TEST(Layouts, EmptyGroup)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "layoutspans.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticError(doc, inputPath, "Layout group contains no content");
}

TEST(Layouts, LabelRefBadTarget)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "errors" / "labelref_bad_target.json";
    auto doc = mnx::Document::create(inputPath);
    expectSemanticErrors(doc, inputPath, {
        "ID \"P-bad-group\" not found in ID mapping",
        "ID \"P-bad-staff\" not found in ID mapping",
        "ID \"P-bad-source\" not found in ID mapping"
    });
}

TEST(Layouts, LabelRefStringRejected)
{
    setupTestDataPaths();
    std::filesystem::path inputPath = getInputPath() / "test_cases" / "time-6-8.json";
    auto doc = mnx::Document::create(inputPath);
    ASSERT_TRUE(mnx::validation::schemaValidate(doc));

    // Schema v57 replaced the "labelref" string with a "labelRef" object.
    (*doc.root())[mnx::json_pointer("/layouts/0/content/0/sources/0/labelRef")] = "name";
    EXPECT_FALSE(mnx::validation::schemaValidate(doc));
    (*doc.root())[mnx::json_pointer("/layouts/0/content/0/sources/0/labelRef")] = mnx::json{{"ref", "bogus"}};
    EXPECT_FALSE(mnx::validation::schemaValidate(doc));
    (*doc.root())[mnx::json_pointer("/layouts/0/content/0/sources/0/labelRef")] = mnx::json{{"ref", "auto"}, {"target", "P1"}};
    EXPECT_TRUE(mnx::validation::schemaValidate(doc));
}
