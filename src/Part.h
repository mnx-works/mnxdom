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
#include "Dynamics.h"
#include "Sequence.h"

namespace mnx {

/**
 * @namespace mnx::part
 * @brief classes related to the root parts array
 */
namespace part {

/**
 * @class ArpeggioBase
 * @brief Contains information about both arpeggios and non arpeggios
 */
class ArpeggioBase : public ArrayElementObject
{
public:
    /// @brief Constructor for existing Beam objects
    ArpeggioBase(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new Beam class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param position The position within the measure.
    /// @param span The span of notes of the arpeggio.
    ArpeggioBase(Base& parent, std::string_view key, const FractionValue& position, const IdPair::Required& span)
        : ArrayElementObject(parent, key)
    {
        create_position(position);
        create_span(span.startId, span.endId);
    }

    MNX_REQUIRED_CHILD(
        RhythmicPosition, position,
        (FractionValue, value));            ///< the rhythmic position of the arpeggio
    MNX_REQUIRED_CHILD(
        IdPair, span,
        (const std::string&, startId),
        (const std::string&, endId));       ///< the span of the arpeggio
};

/**
 * @class Arpeggio
 * @brief Contains information about arpeggios
 */
class Arpeggio : public ArpeggioBase
{
public:
    using ArpeggioBase::ArpeggioBase;

    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, arrow, false);                                     ///< specifies if the arpeggio has an arrow
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(MarkingUpDownAuto, direction, MarkingUpDownAuto::Auto);  ///< the direction of the arpeggio
};

/**
 * @class NonArpeggio
 * @brief Contains information about non-arpeggios
 */
class NonArpeggio : public ArpeggioBase
{
public:
    using ArpeggioBase::ArpeggioBase;
};

/**
 * @class Beam
 * @brief Contains information about each level of beam
 */
class Beam : public ArrayElementObject
{
public:
    /// @brief Constructor for existing Beam objects
    Beam(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new Beam class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Beam(Base& parent, std::string_view key)
        : ArrayElementObject(parent, key)
    {
        create_events();
    }

    MNX_OPTIONAL_CHILD(Array<Beam>, beams);                 ///< the beams that comprise the next beam level (may be omitted)
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(BeamHookDirection, direction, BeamHookDirection::Auto); ///< the forced direction of a beam hook (if this beam contains one event).
    MNX_REQUIRED_CHILD(Array<std::string>, events);         ///< the events that comprise this beam level

    inline static constexpr std::string_view JsonSchemaTypeName = "beam";     ///< required for mapping
};

/**
 * @class Clef
 * @brief Represents a visible clef in the measure
 */
class Clef : public Object
{
public:
    /// @brief initializer class for #Clef
    struct Required
    {
        ClefSign clefSign{};                    ///< the type of clef symbol
        int staffPosition{};                    ///< staff position offset from center of staff
        OttavaAmountOrZero octaveAdjustment{};  ///< octave adjustment (NoTransposition when unset)
    };

    /// @brief Constructor for existing Clef instances
    Clef(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Clef class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param clefSign The type of clef symbol
    /// @param staffPosition Staff position offset from center of staff
    /// @param octaveAdjustment Octave adjustment (NoTransposition when unset)
    Clef(Base& parent, std::string_view key, ClefSign clefSign, int staffPosition, OttavaAmountOrZero octaveAdjustment = OttavaAmountOrZero::NoTransposition)
        : Object(parent, key)
    {
        set_sign(clefSign);
        set_staffPosition(staffPosition);
        set_or_clear_octave(octaveAdjustment);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { sign(), staffPosition(), octave() }; }

    /// @brief Create a Required instance for #Clef.
    static Required make(ClefSign clefSign, int staffPosition, OttavaAmountOrZero octaveAdjustment = OttavaAmountOrZero::NoTransposition)
    { return { clefSign, staffPosition, octaveAdjustment }; }

    MNX_OPTIONAL_PROPERTY(std::string, color);      ///< color to use when rendering the ending
    MNX_OPTIONAL_PROPERTY(std::string, glyph);      ///< the specific SMuFL glyph to use for rendering the clef
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(OttavaAmountOrZero, octave, OttavaAmountOrZero::NoTransposition);  ///< the number of octaves by which the clef transposes
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, showOctave, true); ///< if octave is non-zero, this value determines whether the octave should be displayed on the clef
    MNX_REQUIRED_PROPERTY(ClefSign, sign);          ///< the clef sign
    MNX_REQUIRED_PROPERTY(int, staffPosition);      ///< staff position offset from center of staff (in half-spaces)
};

/**
 * @class Ottava
 * @brief Represents an ottava starting with the next event in the sequence
 */
class       Ottava : public ArrayElementObject
{
public:
    /// @brief initializer class for #Ottava
    struct Required
    {
        OttavaAmount value{};        ///< the value (type) of ottava
        FractionValue position{};    ///< the start position of the ottava
        std::string endMeasureId;    ///< the global id of the end measure of the ottava
        FractionValue endPosition{}; ///< the position within the end measure
    };

    /// @brief Constructor for existing Space objects
    Ottava(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new Space class as a child of a JSON element.
    /// @param parent The parent class instance.
    /// @param key The JSON key to use for embedding in parent.
    /// @param value The value (type) of ottava
    /// @param position The start position of the ottava
    /// @param endPosition The end position of the ottava
    Ottava(Base& parent, std::string_view key, OttavaAmount value, const FractionValue& position, const MeasureRhythmicPosition::Required& endPosition)
        : ArrayElementObject(parent, key)
    {
        create_position(position);
        create_end(endPosition.measureId, endPosition.position);
        set_value(value);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const
    {
        return { value(), position().fraction(), end().measure(), end().position().fraction() };
    }

    /// @brief Create a Required instance for #Ottava.
    static Required make(OttavaAmount value, const FractionValue& position, const MeasureRhythmicPosition::Required& endPosition)
    { return { value, position, endPosition.measureId, endPosition.position }; }

    MNX_REQUIRED_CHILD(MeasureRhythmicPosition, end,
        (const std::string&, measureId), (const FractionValue&, position)); ///< The end of the ottava; omit graceIndex at the start to include preceding grace notes, and use graceIndex 0 at the end to include them there
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(Orientation, orient, Orientation::Auto); ///< Whether the ottava is above or below the staff                
    MNX_REQUIRED_CHILD(RhythmicPosition, position,
        (const FractionValue&, position)); ///< The start position of the ottava
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(int, staff, 1);              ///< The staff (within the part) this ottava applies to
    MNX_REQUIRED_PROPERTY(OttavaAmount, value);                     ///< The type of ottava (amount of displacement, in octaves)
    MNX_OPTIONAL_PROPERTY(std::string, voice);                      ///< Optionally specify the voice this ottava applies to.
};

/**
 * @class KitComponent
 * @brief Describes a single instrument of a drum kit associated with the part.
 */
class KitComponent : public ArrayElementObject
{
public:
    /// @brief initializer class for #KitComponent
    struct Required
    {
        int staffPosition{}; ///< the staff position of the kit component
    };

    /// @brief Constructor for existing KitComponent instances
    KitComponent(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new KitComponent class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param staffPosition The staff position of the kit component
    KitComponent(Base& parent, std::string_view key, int staffPosition)
        : ArrayElementObject(parent, key)
    {
        set_staffPosition(staffPosition);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { staffPosition() }; }

    /// @brief Create a Required instance for #KitComponent.
    static Required make(int staffPosition) { return { staffPosition }; }

    MNX_OPTIONAL_PROPERTY(std::string, name);       ///< Human-readable name of the kit component
    MNX_OPTIONAL_PROPERTY(std::string, sound);      ///< The sound ID in `global.sounds`.
    MNX_REQUIRED_PROPERTY(int, staffPosition);      ///< The staff position of the kit component, where 0 is the middle line.
};

/**
 * @class MeasureRepeatCounter
 * @brief Describes a counter over a @ref MeasureRepeat
 */
class MeasureRepeatCounter : public Object
{
public:
    /// @brief initializer class for #PartTransposition
    struct Required
    {
        int count{}; ///< The count value to display.
    };

    /// @brief Constructor for existing MeasureRepeatCounter instances
    MeasureRepeatCounter(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new MeasureRepeatCounter class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param count The count to display.
    MeasureRepeatCounter(Base& parent, std::string_view key, int count)
        : Object(parent, key)
    {
        set_count(count);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { count() }; }

    /// @brief Create a Required instance for #MeasureRepeatCounter.
    static Required make(int count) { return { count }; }

    MNX_REQUIRED_PROPERTY(int, count);  ///< The count to display.
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(MultiStaffOrientation, orient, MultiStaffOrientation::Auto); ///< Where to display the count.
};

/**
 * @class MeasureRepeat
 * @brief Encodes a simile marking, which means "repeat all music in the previous X measures."
 */
class MeasureRepeat : public Object
{
public:
    /// @brief initializer class for #PartTransposition
    struct Required
    {
        int number{}; ///< The number of measures to repeat.
    };

    /// @brief Constructor for existing MeasureRepeat instances
    MeasureRepeat(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new MeasureRepeat class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param number The number of measures to repeat.
    MeasureRepeat(Base& parent, std::string_view key, int number)
        : Object(parent, key)
    {
        set_number(number);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { number() }; }

    /// @brief Create a Required instance for #MeasureRepeat.
    static Required make(int number) { return { number }; }

    MNX_OPTIONAL_CHILD(MeasureRepeatCounter, counter,
        (int, count));                              ///< Encodes a counter that should be displayed with the measure repeat glyph.
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(AutoYesNo, displayNumber, AutoYesNo::Auto); ///< Whether a number is displayed above the measure repeat glyph.
    MNX_REQUIRED_PROPERTY(int, number);             ///< The number of measures to repeat.
    MNX_OPTIONAL_PROPERTY(int, staffPosition);      ///< THe staff position on which to display the glyph.
};

/**
 * @class PartTransposition
 * @brief Describes a part's instrument transposition
 */
class PartTransposition : public Object
{
public:
    /// @brief initializer class for #PartTransposition
    struct Required
    {
        Interval::Required interval{}; ///< the transposition interval for the part
    };

    /// @brief Constructor for existing Clef instances
    PartTransposition(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Clef class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param interval The transposition interval for the part
    PartTransposition(Base& parent, std::string_view key, const Interval::Required& interval)
        : Object(parent, key)
    {
        create_interval(interval.staffDistance, interval.halfSteps);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { interval() }; }

    /// @brief Create a Required instance for #PartTransposition.
    static Required make(const Interval::Required& interval) { return { interval }; }

    MNX_REQUIRED_CHILD(Interval, interval,
        (int, staffDistance), (int, halfSteps)); ///< the transposition interval
    MNX_OPTIONAL_PROPERTY(int, keyFifthsFlipAt);    ///< the number of sharps (positive) or flats (negative) at which to simplify the key signature
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, prefersWrittenPitches, false); ///< if true, this instrument prefers displaying written pitches even in the context of
                                                    ///< concert-pitch score. (Examples could be piccolo, double base, glockenspiel, etc.)

    /// @brief Calculates and returns the transposed keyFifths value for the input key.
    /// @param concertKey The concert key to calculate from.
    KeySignature::Required calcTransposedKey(const KeySignature::Required& concertKey) const;
};

/**
 * @class PositionedClef
 * @brief Represents a positioned clef for the measure
 */
class PositionedClef : public ArrayElementObject
{
public:
    /// @brief initializer class for #PositionedClef
    struct Required
    {
        ClefSign clefSign{};                                     ///< the type of clef symbol
        int staffPosition{};                                     ///< staff position offset from center of staff
        OttavaAmountOrZero octaveAdjustment{};                    ///< the octave adjustment
    };

    /// @brief Constructor for existing PositionedClef instances
    PositionedClef(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new PositionedClef class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param clefSign The type of clef symbol
    /// @param staffPosition Staff position offset from center of staff
    /// @param octaveAdjustment Optional octave adjustment
    PositionedClef(Base& parent, std::string_view key, ClefSign clefSign, int staffPosition, OttavaAmountOrZero octaveAdjustment = OttavaAmountOrZero::NoTransposition)
        : ArrayElementObject(parent, key)
    {
        create_clef(clefSign, staffPosition, octaveAdjustment);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { clef().sign(), clef().staffPosition(), clef().octave() }; }

    /// @brief Create a Required instance for #PositionedClef.
    static Required make(ClefSign clefSign, int staffPosition, OttavaAmountOrZero octaveAdjustment = OttavaAmountOrZero::NoTransposition)
    { return { clefSign, staffPosition, octaveAdjustment }; }

    MNX_REQUIRED_CHILD(Clef, clef,
        (ClefSign, clefSign), (int, staffPosition), (OttavaAmountOrZero, octaveAdjustment)); ///< the beats per minute of this tempo marking
    MNX_OPTIONAL_CHILD(RhythmicPosition, position,
        (const FractionValue&, position)); ///< location within the measure of the tempo marking
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(int, staff, 1);  ///< the staff number (for multistaff parts)
};

/**
 * @class PositionedStaffConfig
 * @brief Represents a positioned staff configuration for the measure
 */
class PositionedStaffConfig : public ArrayElementObject
{
public:
    /// @brief Constructor for existing PositionedStaffConfig instances
    PositionedStaffConfig(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new PositionedStaffConfig class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    PositionedStaffConfig(Base& parent, std::string_view key)
        : ArrayElementObject(parent, key)
    {
        create_config();
    }

    MNX_REQUIRED_CHILD(StaffConfig, config);        ///< The staff configuration
    MNX_OPTIONAL_CHILD(RhythmicPosition, position,
        (const FractionValue&, position)); ///< location within the measure of the staff configuration change (default: start of measure)
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(int, staff, 1);  ///< the staff number (for multistaff parts)
};

/**
 * @class Measure
 * @brief Represents a single measure in a part in an MNX document. It contains the majority of the musical information in its sequences.
 */
class Measure : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    /// @brief Creates a new Measure class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Measure(Base& parent, std::string_view key)
        : ArrayElementObject(parent, key)
    {
        // required children
        create_sequences();
    }

    MNX_OPTIONAL_CHILD(Array<Arpeggio>, arpeggios);         ///< the arpeggios in this measure
    MNX_OPTIONAL_CHILD(Array<Beam>, beams);                 ///< the beams in this measure
    MNX_OPTIONAL_CHILD(Array<PositionedClef>, clefs);       ///< the clef changes in this bar
    MNX_OPTIONAL_CHILD(DynamicGroupArray, dynamics);        ///< the dynamics in this measure
    MNX_OPTIONAL_CHILD(MeasureRepeat, measureRepeat,
        (int, number));                                     ///< If this part measure is a measure repeat, this includes data about that repeat.
    MNX_OPTIONAL_CHILD(Array<NonArpeggio>, nonArpeggios);   ///< the non-arpeggios in this measure
    MNX_OPTIONAL_CHILD(Array<Ottava>, ottavas);             ///< the ottavas in this measure
    MNX_REQUIRED_CHILD(Array<Sequence>, sequences);         ///< sequences that contain all the musical details in each measure
    MNX_OPTIONAL_CHILD(Array<PositionedStaffConfig>, staffConfigs);  ///< Staff config changes in this measure.

    /// @brief Returns the global measure for this part measure.
    /// @throws std::logic_error if the global measure does not exist.
    [[nodiscard]] mnx::global::Measure getGlobalMeasure() const;

    /// @brief Caculates the time signature at this measure.
    /// @return The time signagure or std::nullopt if none.
    [[nodiscard]] std::optional<TimeSignature> calcCurrentTime() const;

    inline static constexpr std::string_view JsonSchemaTypeName = "part-measure";     ///< required for mapping
};

} // namespace part

/**
 * @class Part
 * @brief Represents a single part in an MNX document.
 */
class Part : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    /// @brief Creates a new Part class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Part(Base& parent, std::string_view key)
        : ArrayElementObject(parent, key)
    {
        // required children
        create_measures();
    }

    MNX_OPTIONAL_CHILD(Dictionary<part::KitComponent>, kit); ///< The definition of a kit of (usually percussion) instruments that are used by the part.
    MNX_REQUIRED_CHILD(Array<part::Measure>, measures); ///< Contains all the musical data for this part
    MNX_OPTIONAL_PROPERTY(std::string, name);           ///< Specifies the user-facing full name of this part
    MNX_OPTIONAL_PROPERTY(std::string, shortName);      ///< Specifies the user-facing abbreviated name of this part
    MNX_OPTIONAL_PROPERTY(std::string, smuflFont);      ///< Name of SMuFL-font for notation elements in the part (can be overridden by children)
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(int, staves, 1); ///< The number of staves in this part.
    MNX_OPTIONAL_CHILD(part::PartTransposition, transposition,
        (const Interval::Required&, interval)); ///< the instrument transposition for the part

    inline static constexpr std::string_view JsonSchemaTypeName = "part";     ///< required for mapping
};

} // namespace mnx
