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

#include "BaseTypes.h"
#include "ContentArray.h"

namespace mnx {
namespace part {

class DynamicGroupArray;
class DynamicAccent;
class DynamicGradual;
class DynamicImmediate;
class DynamicRelative;

/**
 * @class DynamicGroupBase
 * @brief Base class for dynamics.
 */
class DynamicGroupBase : public ContentObject<DynamicGroupBase>
{
public:
    std::string_view defaultType() const override { return ""; }
    /// @brief Constructor for existing Space objects
    DynamicGroupBase(const std::shared_ptr<json>& root, json_pointer pointer)
        : ContentObject(root, pointer)
    {
    }

    /// @brief Creates a new Space class as a child of a JSON element.
    /// @param parent The parent class instance.
    /// @param key The JSON key to use for embedding in parent.
    /// @param value The value of the dynamic
    /// @param position The position within the measure
    DynamicGroupBase(Base& parent, std::string_view key, const FractionValue& position)
        : ContentObject(parent, key)
    {
        create_position(position);
    }

    MNX_OPTIONAL_CHILD(Array<std::string>, glyphs);                 ///< One or more glyphs that specify the exact representation of the dynamic.
                                                                    ///< If present, they override the default representation derived from `value` (and `attackValue`.)
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(MultiStaffPlacement, placement, MultiStaffPlacement::Auto); ///< positioning of the dynamic relative to its part staves
    MNX_REQUIRED_CHILD(
        RhythmicPosition, position,
        (const FractionValue&, position));                          ///< The rhythmic position of the dynamic within the measure.
    MNX_OPTIONAL_PROPERTY(std::string, prefix);                     ///< Text preceding the dynamics representation, e.g., "più"
    MNX_OPTIONAL_PROPERTY(int, staff);                              ///< The staff (within the part) this dynamic applies to
    MNX_OPTIONAL_PROPERTY(std::string, suffix);                     ///< Text following the dynamics representation, e.g., "subito"
    MNX_OPTIONAL_PROPERTY(DynamicValue, value);                     ///< The value of the dynamic.
    MNX_OPTIONAL_PROPERTY(std::string, visuallyContinues);          ///< Points to the ID of the directly previous dynamic group.
    MNX_OPTIONAL_PROPERTY(std::string, voice);                      ///< Optionally specify the voice this dynamic applies to.

    /// @brief Calculates if this dynamic has immediate text.
    /// If this value returns false, then it should be a hairpin with no preceding dynamic.
    virtual bool calcHasImmediateText() const
    {
        return value() || !prefix_or({}).empty() || !suffix_or({}).empty()
            || (glyphs() && !glyphs().value().empty());
    }

    inline static constexpr std::string_view JsonSchemaTypeName = "dynamic-group";     ///< required for mapping
};

/**
 * @class DynamicAccent
 * @brief Immediate dynamics (e.g., ff, ppp)
 */
class DynamicAccent : public DynamicGroupBase
{
public:
    /// @brief initializer class for #DynamicAccent
    struct Required
    {
        DynamicValue value{};      ///< the value of the dynamic
        FractionValue position{};  ///< the position within the measure
    };

    /// @brief Constructor for existing Space objects
    DynamicAccent(const std::shared_ptr<json>& root, json_pointer pointer)
        : DynamicGroupBase(root, pointer)
    {}

    /// @brief Creates a new Space class as a child of a JSON element.
    /// @param parent The parent class instance.
    /// @param key The JSON key to use for embedding in parent.
    /// @param value The value of the dynamic
    /// @param position The position within the measure
    DynamicAccent(Base& parent, std::string_view key, DynamicValue value, const FractionValue& position)
        : DynamicGroupBase(parent, key, position)
    {
        set_value(value);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { value().value(), position().fraction() }; }

    /// @brief Create a Required instance for #DynamicAccent.
    static Required make(DynamicValue value, const FractionValue& position) { return { value, position }; }

    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(DynamicPrefix, accentPrefix, DynamicPrefix::s); ///< defines prefix ("r", "s", or "");
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(DynamicSuffix, accentSuffix, DynamicSuffix::z); ///< defines suffix ("z" or "");
    MNX_OPTIONAL_PROPERTY(DynamicValue, residualValue);             ///< If a dynamic is a sudden change, this is the value that remains
                                                                    ///< after the attack. For example, if the dynamic is "fp", #value is
                                                                    ///< "f" and this would be "p".

    bool calcHasImmediateText() const override
    {
        return DynamicGroupBase::calcHasImmediateText()
            || accentPrefix() != DynamicPrefix::None
            || accentSuffix() != DynamicSuffix::None
            || residualValue();
    }

    inline static constexpr std::string_view ContentTypeValue = "accent";    ///< type value that identifies the type of dynamic
};

/**
 * @class DynamicGradual
 * @brief Gradual dynamics (hairpins)
 */
class DynamicGradual : public DynamicGroupBase
{
public:
    /// @brief initializer class for #DynamicGradual
    struct Required
    {
        DynamicWedgeType wedgeType{}; ///< the type of hairpin dynamic
        FractionValue position{};     ///< the start position within the measure
        std::string endMeasureId;     ///< the end measure id
        FractionValue endPosition{};  ///< the end position within the measure
    };

    /// @brief Constructor for existing Space objects
    DynamicGradual(const std::shared_ptr<json>& root, json_pointer pointer)
        : DynamicGroupBase(root, pointer)
    {
    }

    /// @brief Creates a new Space class as a child of a JSON element.
    /// @param parent The parent class instance.
    /// @param key The JSON key to use for embedding in parent.
    /// @param wedgeType The wedge type (cresc or dim.)
    /// @param position The position within the measure
    /// @param end The end position of the gradual dynamic
    DynamicGradual(Base& parent, std::string_view key, DynamicWedgeType wedgeType, const FractionValue& position,
        const MeasureRhythmicPosition::Required& endPosition)
        : DynamicGroupBase(parent, key, position)
    {
        create_end(endPosition.measureId, endPosition.position);
        set_wedgeType(wedgeType);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const
    {
        return { wedgeType(), position().fraction(), end().measure(), end().position().fraction() };
    }

    /// @brief Create a Required instance for #DynamicGradual.
    static Required make(DynamicWedgeType wedgeType, const FractionValue& position, const MeasureRhythmicPosition::Required& endPosition)
    { return { wedgeType, position, endPosition.measureId, endPosition.position }; }

    MNX_REQUIRED_CHILD(MeasureRhythmicPosition, end,
        (const std::string&, measureId), (const FractionValue&, position));     ///< the end position of the hairpin dynamic; set graceIndex to 0 to include preceding grace notes at the end boundary
    MNX_OPTIONAL_PROPERTY(int, staffEnd);                                       ///< the staff (within the part) on which the hairpin ends
    MNX_REQUIRED_PROPERTY(DynamicWedgeType, wedgeType);                         ///< the type of hairpin dynamic

    inline static constexpr std::string_view ContentTypeValue = "gradual";      ///< type value that identifies the type of dynamic
};

/**
 * @class DynamicImmediate
 * @brief Immediate dynamics (e.g., ff, ppp)
 */
class DynamicImmediate : public DynamicGroupBase
{
public:
    /// @brief initializer class for #DynamicImmediate
    struct Required
    {
        DynamicValue value{};      ///< the value of the dynamic
        FractionValue position{};  ///< the position within the measure
    };

    /// @brief Constructor for existing Space objects
    DynamicImmediate(const std::shared_ptr<json>& root, json_pointer pointer)
        : DynamicGroupBase(root, pointer)
    {}

    /// @brief Creates a new Space class as a child of a JSON element.
    /// @param parent The parent class instance.
    /// @param key The JSON key to use for embedding in parent.
    /// @param value The value of the dynamic
    /// @param position The position within the measure
    DynamicImmediate(Base& parent, std::string_view key, DynamicValue value, const FractionValue& position)
        : DynamicGroupBase(parent, key, position)
    {
        set_value(value);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { value().value(), position().fraction() }; }

    /// @brief Create a Required instance for #DynamicImmediate.
    static Required make(DynamicValue value, const FractionValue& position) { return { value, position }; }

    inline static constexpr std::string_view ContentTypeValue = "immediate";    ///< type value that identifies the type of dynamic
};

/**
 * @class DynamicRelative
 * @brief Relative dynamics (e.g., più f)
 */
class DynamicRelative : public DynamicGroupBase
{
public:
    /// @brief initializer class for #DynamicRelative
    struct Required
    {
        DynamicRelativeValue relativeValue{}; ///< Whether the dynamic is relatively softer or louder
        FractionValue position{};             ///< the position within the measure
    };

    /// @brief Constructor for existing Space objects
    DynamicRelative(const std::shared_ptr<json>& root, json_pointer pointer)
        : DynamicGroupBase(root, pointer)
    {}

    /// @brief Creates a new Space class as a child of a JSON element.
    /// @param parent The parent class instance.
    /// @param key The JSON key to use for embedding in parent.
    /// @param relativeValue The relative change of the dynamic
    /// @param position The position within the measure
    DynamicRelative(Base& parent, std::string_view key, DynamicRelativeValue relativeValue, const FractionValue& position)
        : DynamicGroupBase(parent, key, position)
    {
        set_relativeValue(relativeValue);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { relativeValue(), position().fraction() }; }

    /// @brief Create a Required instance for #DynamicRelative.
    static Required make(DynamicRelativeValue relativeValue, const FractionValue& position) { return { relativeValue, position }; }

    MNX_REQUIRED_PROPERTY(DynamicRelativeValue, relativeValue);     ///< Whether the dynamic is relatively softer or louder

    inline static constexpr std::string_view ContentTypeValue = "relative";    ///< type value that identifies the type of dynamic
};

class DynamicGroupArray : public ContentArray<part::DynamicGroupBase>
{
public:
    using ContentArray<part::DynamicGroupBase>::ContentArray;

    part::DynamicAccent appendAccent(DynamicValue value, const FractionValue& position)
    { return appendWithType<part::DynamicAccent>(value, position); }

    part::DynamicGradual appendGradual(DynamicWedgeType wedgeType, const FractionValue& position,
        const MeasureRhythmicPosition::Required& endPosition)
    { return appendWithType<part::DynamicGradual>(wedgeType, position, endPosition); }

    part::DynamicImmediate appendImmediate(DynamicValue value, const FractionValue& position)
    { return appendWithType<part::DynamicImmediate>(value, position); }

    part::DynamicRelative appendRelative(DynamicRelativeValue relativeValue, const FractionValue& position)
    { return appendWithType<part::DynamicRelative>(relativeValue, position); }
};

} // namespace part
} // namespace mnx
