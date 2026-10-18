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

#include <string>
#include <vector>
#include <unordered_set>

#include "../util/EntityMap.h"

namespace mnx {

class Document;

/**
 * @namespace mnx::validation
 * @brief provides schema- and semantic-level validation functions for MNX documents
 */
namespace validation {

/// @struct ValidationResult
/// @brief Encapsulates a schema validation result
struct ValidationResult
{
    /// @struct Error
    /// @brief All the information about a specific error
    struct Error
    {
        /// @brief constructor
        Error(const json_pointer& ptr, const json& inst, const std::string& msg)
            : instance(inst), pointer(ptr), message(msg) {}
            
        json instance;          ///< instance containing the error
        json_pointer pointer;   ///< the location of the instance in the top level
        std::string message;    ///< a message describing the error

        /// @brief Converts the error to a string. (Matches the schema validator's default message.)
        /// @param indent The number of indents in the output message. Omit for none.
        std::string to_string(int indent = -1) const
        {
            return "At " + pointer.to_string() + " of " + instance.dump(indent) + " - " + message;
        }
    };
    std::vector<Error> errors;  ///< errors encountered

    /// @brief Allows a simple if check to see if the schema validated
    explicit operator bool() const { return errors.empty(); }
};

/// @struct SemanticValidationResult
/// @brief Returns the results of semantic validation
struct SemanticValidationResult : public ValidationResult
{
    using ValidationResult::ValidationResult;

    std::unordered_map<std::string, json_pointer> lyricLines; ///< list of lyric line ID values.
    ///< These are sourced either from `global.lyrics().lineOrder()` or `global().lyrics().lineMetadata()`.
    ///< The json_pointer is used only for reporting the location of duplicate values. It points to an element
    ///< in one of the two arrays, with preference given to `lineOrder` if it exists.

    std::vector<Error> warnings;                              ///< Advisory diagnostics; do not affect validation success
};

/// @brief Validates a document against a JSON schema
/// @param document The mnx::Document to validate
/// @param jsonSchema The JSON schema to validate against, or std::nullopt for the embedded schema
/// @return validation result
extern ValidationResult schemaValidate(const Document& document, const std::optional<std::string>& jsonSchema = std::nullopt);

/// @brief Validates the semantics of the input MNX document
/// @details Warnings are advisory and do not affect the result's conversion to bool.
/// @param document The document to validate
/// @return validation result
extern SemanticValidationResult semanticValidate(const Document& document);

/// @brief Detects if the input JSON has a valid document root. This allows client code to permit a
/// non-schema-conforming document to run anyway, perhaps with a warning to the user.
/// @param document The document to check.
/// @return True if the document root is a valid MNX document root.
extern bool hasValidDocumentRoot(const Document& document);
} // namespace validation
} // namespace mnx