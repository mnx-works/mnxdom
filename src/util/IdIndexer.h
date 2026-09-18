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

#include <functional>
#include <string>

#include "../BaseTypes.h"

namespace mnx::util::detail {

#ifndef DOXYGEN_SHOULD_IGNORE_THIS

/// @brief An `id` value discovered while walking a document.
struct IndexedId
{
    std::string id;             ///< the id value
    json_pointer location;      ///< the object that carries the id
    std::string typeName;       ///< the schema `$defs` name of the object, or empty if it could not be classified
};

/// @brief Walks every object in @p root and reports each string-valued `id` property.
///
/// The walk follows the MNX schema in tandem with the instance so that each id can be
/// classified by the name of the schema definition that describes its object. Objects
/// the schema cannot classify (because the document deviates from the schema) are still
/// reported, with an empty type name. Dictionary keys named `id` (e.g. in `kit` or
/// `sounds`) are not ids and are not reported. Vendor extension (`_x`) subtrees are skipped.
///
/// @param root The document root.
/// @param schema The MNX JSON schema (see mnx::getMnxSchemaJson).
/// @param sink Called once for each id found, in document order.
void indexDocumentIds(const json& root, const json& schema, const std::function<void(const IndexedId&)>& sink);

#endif // DOXYGEN_SHOULD_IGNORE_THIS

} // namespace mnx::util::detail
