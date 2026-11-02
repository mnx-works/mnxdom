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
#include <string_view>
#include <vector>

#include "BaseTypes.h"
#include "ContentArray.h"
#include "Enumerations.h"

namespace mnx {

/**
 * @namespace mnx::text
 * @brief classes related to formatted text objects
 */
namespace text {

/**
 * @class TextContentObject
 * @brief Base class for formatted-text content objects.
 */
class TextContentObject : public ContentObject<TextContentObject>
{
public:
    using ContentObject::ContentObject;
    std::string_view defaultType() const override { return "text"; }
};

/**
 * @class Text
 * @brief Represents a formatted text fragment.
 */
class Text : public TextContentObject
{
public:
    /// @brief initializer class for #Text
    struct Required
    {
        std::string text;   ///< the text to be represented
    };

    /// @brief Constructor for existing Text objects
    Text(const std::shared_ptr<json>& root, json_pointer pointer)
        : TextContentObject(root, pointer)
    {
    }

    /// @brief Creates a new Text class as a child of a JSON element.
    /// @param parent The parent class instance.
    /// @param key The JSON key to use for embedding in parent.
    /// @param text The text to be represented.
    Text(Base& parent, std::string_view key, const std::string& text)
        : TextContentObject(parent, key)
    {
        set_text(text);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { text() }; }

    /// @brief Create a Required instance for #Text.
    static Required make(const std::string& text) { return { text }; }

    MNX_REQUIRED_PROPERTY(std::string, text);       ///< the text for this formatted chunk

    inline static constexpr std::string_view JsonSchemaTypeName = "text-chunk-text";
    inline static constexpr std::string_view ContentTypeValue = "text";    ///< type value that identifies the type within the content array
};

/**
 * @class Smufl
 * @brief Represents a formatted text fragment expressed as SMuFL glyphs.
 */
class Smufl : public TextContentObject
{
public:
    /// @brief initializer class for #Smufl
    struct Required
    {
        std::vector<std::string> glyphs;   ///< the smufl glyphs in this chunk
    };

    /// @brief Constructor for existing Smufl objects
    Smufl(const std::shared_ptr<json>& root, json_pointer pointer)
        : TextContentObject(root, pointer)
    {
    }

    /// @brief Creates a new Smufl class as a child of a JSON element.
    /// @param parent The parent class instance.
    /// @param key The JSON key to use for embedding in parent.
    /// @param glyphs The smufl glyphs in this chunk.
    Smufl(Base& parent, std::string_view key, const std::vector<std::string>& glyphs)
        : TextContentObject(parent, key)
    {
        create_glyphs().assign(glyphs);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { glyphs().toStdVector() }; }

    /// @brief Create a Required instance for #Smufl.
    static Required make(const std::vector<std::string>& glyphs) { return { glyphs }; }

    MNX_REQUIRED_CHILD(Array<std::string>, glyphs);     ///< the smufl glyphs in this chunk

    inline static constexpr std::string_view JsonSchemaTypeName = "text-chunk-smufl";
    inline static constexpr std::string_view ContentTypeValue = "smufl";    ///< type value that identifies the type within the content array
};

} // namespace text

/**
 * @class FormattedText
 * @brief Container for formatted-text content objects.
 */
class FormattedText : public ContentArray<text::TextContentObject>
{
public:
    using ContentArray<text::TextContentObject>::ContentArray;

    text::Smufl appendSmufl(const std::vector<std::string>& glyphs)
    { return appendWithType<text::Smufl>(glyphs); }

    text::Text appendText(const std::string& text)
    { return appendWithType<text::Text>(text); }
};

} // namespace mnx
