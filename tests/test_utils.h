/*merged/
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
#pragma once

#include <vector>
#include <string>
#include <filesystem>

#include "mnxdom.h"

inline const mnx::validation::SchemaValidationOptions withIdCollection{true, std::nullopt};

inline std::filesystem::path getInputPath()
{ return std::filesystem::current_path() / "inputs"; }

inline std::filesystem::path getOutputPath()
{ return std::filesystem::current_path() / "outputs"; }

inline std::string pathString(const std::filesystem::path& path) {
    auto s = path.u8string();
    return std::string(reinterpret_cast<const char*>(s.data()), s.size());
}

void setupTestDataPaths();
void copyInputToOutput(const std::string& fileName, std::filesystem::path& outputPath);
void compareFiles(const std::filesystem::path& path1, const std::filesystem::path& path2);

void assertStringsInFile(const std::vector<std::string>& targets, const std::filesystem::path& filePath, const std::filesystem::path& extension = {});
inline void assertStringInFile(const std::string& target, const std::filesystem::path& filePath, const std::filesystem::path& extension = {})
{ assertStringsInFile(std::vector<std::string>({ target }), filePath, extension); }

bool validateSchema(const mnx::Document& doc, const std::filesystem::path& filePath);
bool fullValidate(const mnx::Document& doc, const std::filesystem::path& filePath);

void expectSemanticErrors(const mnx::Document& doc, const std::filesystem::path& filePath, const std::vector<std::string>& errors, bool skipSchema = false);
inline void expectSemanticError(const mnx::Document& doc, const std::filesystem::path& filePath, const std::string& error, bool skipSchema = false)
{ expectSemanticErrors(doc, filePath, { error }, skipSchema); }