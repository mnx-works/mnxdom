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
#pragma once

#include <utility>
#include <vector>

#include "BaseTypes.h"

namespace mnx {

class Document;
namespace validation {
struct SchemaValidationOptions;
struct SchemaValidationResult;
SchemaValidationResult schemaValidate(const Document&, const SchemaValidationOptions&);
} // namespace validation

/// Owning snapshot of the IDs discovered in one MNX document, including duplicates.
/// Use it with the same document. Collect it again after edits affecting IDs, paths or types.
class DocumentIdIndex
{
public:
    struct Item
    {
        std::string id;         ///< String value of the ID.
        json_pointer location;  ///< Pointer to the object carrying the ID.
        std::string typeName;   ///< Selected schema definition name, or empty if unknown.
    };

    [[nodiscard]] auto begin() const { return m_items.cbegin(); }
    [[nodiscard]] auto end() const { return m_items.cend(); }
    [[nodiscard]] size_t size() const { return m_items.size(); }

private:
    friend validation::SchemaValidationResult validation::schemaValidate(
        const Document&, const validation::SchemaValidationOptions&);

    explicit DocumentIdIndex(std::vector<Item> items) : m_items(std::move(items)) {}
    std::vector<Item> m_items;
};

} // namespace mnx
