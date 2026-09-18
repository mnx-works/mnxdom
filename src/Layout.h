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

#include "Enumerations.h"
#include "BaseTypes.h"
#include "ContentArray.h"

namespace mnx {

/**
 * @namespace mnx::layout
 * @brief classes related to the root layouts array
 */
namespace layout {
class Group;
class Staff;

class LayoutContentObject : public ContentObject<LayoutContentObject>
{
public:
    using ContentObject::ContentObject;
    std::string_view defaultType() const override { return ""; }
};

class LayoutContent : public ContentArray<LayoutContentObject>
{
public:
    using ContentArray<LayoutContentObject>::ContentArray;

    Group appendGroup();
    Staff appendStaff();
};

/**
 * @class StaffSource
 * @brief Represents a staff source for a staff in a layout in a score.
 */
class StaffSource : public ArrayElementObject
{
public:
    /// @brief initializer class for #StaffSource
    struct Required
    {
        std::string partId; ///< the part to use as a source
    };

    /// @brief Constructor for existing staff sources
    StaffSource(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }
    
    /// @brief Creates a new StaffSource class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param partId The part to use as a source
    StaffSource(Base& parent, std::string_view key, const std::string& partId)
        : ArrayElementObject(parent, key)
    {
        set_part(partId);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { part() }; }

    /// @brief Create a Required instance for #StaffSource.
    static Required make(const std::string& partId) { return { partId }; }

    MNX_OPTIONAL_PROPERTY(std::string, label);          ///< Text to appear to the left of the staff
    MNX_OPTIONAL_PROPERTY(LabelRef, labelref);          ///< The labelref to use (rather than label)
    MNX_REQUIRED_PROPERTY(std::string, part);           ///< Id of the part to draw from
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(int, staff, 1);  ///< The staff number within the part.
    MNX_OPTIONAL_PROPERTY(StemDirection, stem);         ///< The stem direction for this staff source.
    MNX_OPTIONAL_PROPERTY(std::string, voice);          ///< The voice id to draw from.

    inline static constexpr std::string_view JsonSchemaTypeName = "staff-source";     ///< required for mapping
};

/**
 * @class Staff
 * @brief Represents a single staff instance within an MNX layout.
 */
class Staff : public LayoutContentObject
{
public:
    using LayoutContentObject::LayoutContentObject;

    /// @brief Creates a new Staff class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Staff(Base& parent, std::string_view key)
        : LayoutContentObject(parent, key)
    {
        // required children
        create_sources();
    }

    MNX_OPTIONAL_PROPERTY(std::string, label);                  ///< Label to be rendered to the left of the staff
    MNX_OPTIONAL_PROPERTY(LabelRef, labelref);                  ///< The labelref to use (rather than label)
    MNX_REQUIRED_CHILD(Array<StaffSource>, sources);            ///< The sources for this staff.
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(LayoutSymbol, symbol, LayoutSymbol::NoSymbol); ///< The symbol down the left side.

    inline static constexpr std::string_view ContentTypeValue = "staff"; ///< type value that identifies the type within the content array

    inline static constexpr std::string_view JsonSchemaTypeName = "staff";     ///< required for mapping
};

/**
 * @class Group
 * @brief Represents a groups of staves within an MNX layout.
 */
class Group : public LayoutContentObject
{
public:
    using LayoutContentObject::LayoutContentObject;

    /// @brief Creates a new Group class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Group(Base& parent, std::string_view key)
        : LayoutContentObject(parent, key)
    {
        // required children
        create_content();
    }

    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(StaffGroupBarlineStyle, barlineStyle, StaffGroupBarlineStyle::Instrument); ///< barline override settings
    MNX_REQUIRED_CHILD(LayoutContent, content);      ///< Required child containing the layout content (groups and staves).
    MNX_OPTIONAL_PROPERTY(std::string, label);      ///< Label to be rendered to the left of the group
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(LayoutSymbol, symbol, LayoutSymbol::NoSymbol); ///< The symbol down the left side.

    /// @brief Calculates whether all staves in this group subtree use the same part id.
    [[nodiscard]] bool calcIsPartGroup() const;

    /// @brief Calculates the effective barline override.
    [[nodiscard]] StaffGroupBarlineOverride calcBarlineOverride() const;

    inline static constexpr std::string_view ContentTypeValue = "group"; ///< type value that identifies the type within the content array

    inline static constexpr std::string_view JsonSchemaTypeName = "staff-group";     ///< required for mapping
};

inline Group LayoutContent::appendGroup()
{ return appendWithType<Group>(); }

inline Staff LayoutContent::appendStaff()
{ return appendWithType<Staff>(); }

} // namespace layout

/**
 * @class Layout
 * @brief Represents the element of the layout array in an MNX document.
 */
class Layout : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    /// @brief Creates a new Layout class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Layout(Base& parent, std::string_view key)
        : ArrayElementObject(parent, key)
    {
        // required children
        create_content();
    }

    MNX_REQUIRED_CHILD(layout::LayoutContent, content);      ///< Required child containing the layout content (groups and staves).

    inline static constexpr std::string_view JsonSchemaTypeName = "system-layout";     ///< required for mapping
};

} // namespace mnx
