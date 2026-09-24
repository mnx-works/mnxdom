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
#include "Enumerations.h"

namespace mnx {
namespace sequence {

/**
 * @class EventMarkingBase
 * @brief Base class for event markings
 */
class EventMarkingBase : public Object
{
public:
    using Object::Object;

    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(Placement, placement, Placement::Auto);      ///< Whether the marking is above or below its associated event
};

/**
 * @class Accent
 * @brief Class that represents an accent marking on an event
 */
class Accent : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;

    inline static constexpr std::string_view JsonSchemaTypeName = "accent";     ///< required for mapping
};

/**
 * @class BowDirection
 * @brief Class that represents down- or up-bow marking
 */
class BowDirection : public EventMarkingBase
{
public:
    /// @brief initializer class for #BowDirection
    struct Required
    {
        MarkingUpDown direction{}; ///< whether it is a down-bow or up-bow marking
    };

    /// @brief Constructor for detatched instance
    /// @param direction Whether it is a down-bow or up-bow marking
    BowDirection(MarkingUpDown direction) : EventMarkingBase()
    {
        set_direction(direction);
    }

    /// @brief Constructor for existing BowDirection objects
    BowDirection(const std::shared_ptr<json>& root, json_pointer pointer)
        : EventMarkingBase(root, pointer)
    {
    }

    /// @brief Creates a new BowDirection class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param direction Whether it is a down-bow or up-bow marking
    BowDirection(Base& parent, std::string_view key, MarkingUpDown direction)
        : EventMarkingBase(parent, key)
    {
        set_direction(direction);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { direction() }; }

    /// @brief Create a Required instance for #BowDirection.
    static Required make(MarkingUpDown direction) { return { direction }; }

    MNX_REQUIRED_PROPERTY(MarkingUpDown, direction);     ///< whether it is a down bow or up bow

    inline static constexpr std::string_view JsonSchemaTypeName = "bow-direction";     ///< required for mapping
};

/**
 * @class BreathMark
 * @brief Class that represents a breath mark on an event
 */
class BreathMark : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;

    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(BreathMarkSymbol, symbol, BreathMarkSymbol::Auto); ///< The symbol for the breath mark.

    inline static constexpr std::string_view JsonSchemaTypeName = "breath-mark";     ///< required for mapping
};

/**
 * @class Caesura
 * @brief Class that represents a caesura. It is encoded on the event before the caesura happens.
 */
class Caesura : public Object
{
public:
    using Object::Object;

    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(unsigned, marks, 2);                             ///< The number of marks/strokes (1 or 2).
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(CaesuraShape, shape, CaesuraShape::Normal);      ///< The visual shape of the caesura.

    inline static constexpr std::string_view JsonSchemaTypeName = "caesura";     ///< required for mapping
};

/**
 * @class SoftAccent
 * @brief Class that represents a soft accent marking on an event
 */
class SoftAccent : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;

    inline static constexpr std::string_view JsonSchemaTypeName = "soft-accent";     ///< required for mapping
};

/**
 * @class Spiccato
 * @brief Class that represents a spiccato marking on an event
 */
class Spiccato : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;

    inline static constexpr std::string_view JsonSchemaTypeName = "spiccato";     ///< required for mapping
};

/**
 * @class Staccatissimo
 * @brief Class that represents a staccatissimo marking on an event
 */
class Staccatissimo : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;

    inline static constexpr std::string_view JsonSchemaTypeName = "staccatissimo";     ///< required for mapping
};

/**
 * @class Staccato
 * @brief Class that represents a staccato marking on an event
 */
class Staccato : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;

    inline static constexpr std::string_view JsonSchemaTypeName = "staccato";     ///< required for mapping
};

/**
 * @class Stress
 * @brief Class that represents a stress marking on an event
 */
class Stress : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;

    inline static constexpr std::string_view JsonSchemaTypeName = "stress-marking";     ///< required for mapping
};

/**
 * @class StrongAccent
 * @brief Class that represents a strong accent marking on an event
 */
class StrongAccent : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;

    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(MarkingUpDownAuto, pointing, MarkingUpDownAuto::Auto);   ///< Specifies if the accent points upward or downward,
                                                                                                ///< irrespective of above/below placement.

    inline static constexpr std::string_view JsonSchemaTypeName = "strong-accent";     ///< required for mapping
};

/**
 * @class Tenuto
 * @brief Class that represents a tenuto marking on an event
 */
class Tenuto : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;

    inline static constexpr std::string_view JsonSchemaTypeName = "tenuto";     ///< required for mapping
};

/**
 * @class SingleNoteTremolo
 * @brief Class that represents single-note tremolo marking on an event
 */
class SingleNoteTremolo : public EventMarkingBase
{
public:
    /// @brief initializer class for #SingleNoteTremolo
    struct Required
    {
        unsigned marks{}; ///< the number of marks
    };

    /// @brief Constructor for existing SingleNoteTremolo objects
    SingleNoteTremolo(const std::shared_ptr<json>& root, json_pointer pointer)
        : EventMarkingBase(root, pointer)
    {
    }

    /// @brief Constructor for detached instance
    /// @param marks The number of marks
    SingleNoteTremolo(unsigned marks) : EventMarkingBase()
    {
        set_marks(marks);
    }

    /// @brief Creates a new SingleNoteTremolo class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param marks The number of marks
    SingleNoteTremolo(Base& parent, std::string_view key, unsigned marks)
        : EventMarkingBase(parent, key)
    {
        set_marks(marks);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { marks() }; }

    /// @brief Create a Required instance for #SingleNoteTremolo.
    static Required make(unsigned marks) { return { marks }; }

    MNX_REQUIRED_PROPERTY(unsigned, marks);     ///< the number of marks (a value from 0..8, inclusive)

    inline static constexpr std::string_view JsonSchemaTypeName = "tremolo-single";     ///< required for mapping
};

/**
 * @class Unstress
 * @brief Class that represents an unstress marking on an event
 */
class Unstress : public EventMarkingBase
{
public:
    using EventMarkingBase::EventMarkingBase;

    inline static constexpr std::string_view JsonSchemaTypeName = "unstress-marking";     ///< required for mapping
};

/**
 * @class EventMarkings
 * @brief Container for any markings on an event
 */
class EventMarkings : public Object
{
public:
    using Object::Object;

    MNX_OPTIONAL_CHILD(Accent, accent);                 ///< An accent mark
    MNX_OPTIONAL_CHILD(BowDirection, bowDirection,
        (MarkingUpDown, direction));                    ///< A bow direction mark
    MNX_OPTIONAL_CHILD(BreathMark, breath);             ///< A breath mark
    MNX_OPTIONAL_CHILD(Caesura, caesura);               ///< A caesura
    MNX_OPTIONAL_CHILD(SoftAccent, softAccent);         ///< A soft accent mark
    MNX_OPTIONAL_CHILD(Spiccato, spiccato);             ///< A spiccato mark
    MNX_OPTIONAL_CHILD(Staccatissimo, staccatissimo);   ///< A staccatissimo mark
    MNX_OPTIONAL_CHILD(Staccato, staccato);             ///< A staccato mark
    MNX_OPTIONAL_CHILD(Stress, stress);                 ///< A stress mark
    MNX_OPTIONAL_CHILD(StrongAccent, strongAccent);     ///< A strong accent mark
    MNX_OPTIONAL_CHILD(Tenuto, tenuto);                 ///< A stress mark
    MNX_OPTIONAL_CHILD(SingleNoteTremolo, tremolo,
        (unsigned, marks));                             ///< A single-note tremolo mark
    MNX_OPTIONAL_CHILD(Unstress, unstress);             ///< A stress mark

    inline static constexpr std::string_view JsonSchemaTypeName = "event-markings";     ///< required for mapping
};

} // namespace sequence
} // namespace mnx
