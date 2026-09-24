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
#include "CommonClasses.h"

namespace mnx {

/**
 * @namespace mnx::global
 * @brief classes related to the root global object
 */
namespace global {

/**
 * @class Barline
 * @brief Represents the barline for a global measure.
 */
class Barline : public Object
{
public:
    /// @brief initializer class for #Barline
    struct Required
    {
        BarlineType barlineType{}; ///< the barline type for this Barline
    };

    /// @brief Constructor for existing Barline objects
    Barline(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Barline class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param barlineType The barline type for this Barline
    Barline(Base& parent, std::string_view key, BarlineType barlineType)
        : Object(parent, key)
    {
        set_type(barlineType);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { type() }; }

    /// @brief Create a Required instance for #Barline.
    static Required make(BarlineType barlineType) { return { barlineType }; }

    MNX_REQUIRED_PROPERTY(BarlineType, type);  ///< the type of barline

    inline static constexpr std::string_view JsonSchemaTypeName = "barline";     ///< required for mapping
};

/**
 * @class Ending
 * @brief Represents an alternate ending (or "volta bracket") for a global measure.
 */
class Ending : public Object
{
public:
    /// @brief initializer class for #Ending
    struct Required
    {
        int duration{}; ///< the duration of the ending
    };

    /// @brief Constructor for existing Ending objects
    Ending(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Ending class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param duration The duration of the ending
    Ending(Base& parent, std::string_view key, int duration)
        : Object(parent, key)
    {
        set_duration(duration);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { duration() }; }

    /// @brief Create a Required instance for #Ending.
    static Required make(int duration) { return { duration }; }

    MNX_OPTIONAL_PROPERTY(std::string, color);      ///< color to use when rendering the ending
    MNX_REQUIRED_PROPERTY(int, duration);           ///< the type of barline
    MNX_OPTIONAL_CHILD(Array<int>, numbers);        ///< ending numbers
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, open, false);  ///< if this is an open (i.e., final) ending

    inline static constexpr std::string_view JsonSchemaTypeName = "ending";     ///< required for mapping
};

/**
 * @class Fine
 * @brief Represents an Fine object (as in "D.S. al Fine")
 */
class Fine : public Object
{
public:
    /// @brief initializer class for #Fine
    struct Required
    {
        FractionValue position{}; ///< the position of the Fine within the measure
    };

    /// @brief Constructor for existing Fine objects
    Fine(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Fine class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param position The position of the Fine within the measure
    Fine(Base& parent, std::string_view key, const FractionValue& position)
        : Object(parent, key)
    {
        create_location(position);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { location().fraction() }; }

    /// @brief Create a Required instance for #Fine.
    static Required make(const FractionValue& position) { return { position }; }

    MNX_OPTIONAL_PROPERTY(std::string, color);                  ///< color to use when rendering the fine direction
    MNX_REQUIRED_CHILD(RhythmicPosition, location,
        (const FractionValue&, position)); ///< the location of the fine direction

    inline static constexpr std::string_view JsonSchemaTypeName = "fine";     ///< required for mapping
};

/**
 * @class Jump
 * @brief Represents an Jump object (as in "D.S.")
 */
class Jump : public Object
{
public:
    /// @brief initializer class for #Jump
    struct Required
    {
        JumpType jumpType{};      ///< the jump type
        FractionValue position{}; ///< the position within the measure
    };

    /// @brief Constructor for existing Jump objects
    Jump(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Jump class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param jumpType The jump type
    /// @param position The position within the measure
    Jump(Base& parent, std::string_view key, JumpType jumpType, const FractionValue& position)
        : Object(parent, key)
    {
        set_type(jumpType);
        create_location(position);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { type(), location().fraction() }; }

    /// @brief Create a Required instance for #Jump.
    static Required make(JumpType jumpType, const FractionValue& position) { return { jumpType, position }; }

    MNX_REQUIRED_PROPERTY(JumpType, type);                      ///< the JumpType
    MNX_REQUIRED_CHILD(RhythmicPosition, location,
        (const FractionValue&, position)); ///< the location of the jump

    inline static constexpr std::string_view JsonSchemaTypeName = "jump";     ///< required for mapping
};

/**
 * @class RepeatEnd
 * @brief Represents the end of a repeat in an MNX document.
 */
class RepeatEnd : public Object
{
public:
    using Object::Object;

    MNX_OPTIONAL_PROPERTY(int, times);               ///< number of times to repeat

    inline static constexpr std::string_view JsonSchemaTypeName = "repeat-end";     ///< required for mapping
};

/**
 * @class RepeatStart
 * @brief Represents the start of a repeat in an MNX document.
 */
class RepeatStart : public Object
{
public:
    using Object::Object;

    inline static constexpr std::string_view JsonSchemaTypeName = "repeat-start";     ///< required for mapping
};

/**
 * @class Segno
 * @brief Represents a segno marker
 */
class Segno : public Object
{
public:
    /// @brief initializer class for #Segno
    struct Required
    {
        FractionValue position{}; ///< the position of the Segno within the measure
    };

    /// @brief Constructor for existing Segno objects
    Segno(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Segno class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param position The position of the Segno within the measure
    Segno(Base& parent, std::string_view key, const FractionValue& position)
        : Object(parent, key)
    {
        create_location(position);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { location().fraction() }; }

    /// @brief Create a Required instance for #Segno.
    static Required make(const FractionValue& position) { return { position }; }

    MNX_OPTIONAL_PROPERTY(std::string, color);      ///< color to use when rendering the segno
    MNX_OPTIONAL_PROPERTY(std::string, glyph);      ///< the SMuFL glyph name to be used when rendering this segno.
    MNX_REQUIRED_CHILD(RhythmicPosition, location,
        (const FractionValue&, position)); ///< location

    inline static constexpr std::string_view JsonSchemaTypeName = "segno";     ///< required for mapping
};

/**
 * @class Sound
 * @brief Represents a sound definition.
 */
class Sound : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    MNX_OPTIONAL_PROPERTY(unsigned, midiNumber);  ///< MIDI pitch of the sound from 0-127, where middle C is 60.
    MNX_OPTIONAL_PROPERTY(std::string, name);     ///< The name of the sound (presentable to the user)

    inline static constexpr std::string_view JsonSchemaTypeName = "sound";     ///< required for mapping
};

/**
 * @class Tempo
 * @brief Represents the tempo for a global measure.
 */
class Tempo : public ArrayElementObject
{
public:
    /// @brief initializer class for #Tempo
    struct Required
    {
        double bpm{};                    ///< the beats per minute
        NoteValue::Required noteValue{};   ///< the note value for the tempo
    };

    /// @brief Constructor for existing Tempo instances
    Tempo(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new Tempo class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param bpm The beats per minute
    /// @param noteValue The note value for the tempo
    Tempo(Base& parent, std::string_view key, double bpm, const NoteValue::Required& noteValue)
        : ArrayElementObject(parent, key)
    {
        set_bpm(bpm);
        create_value(noteValue.base, noteValue.dots);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { bpm(), value() }; }

    /// @brief Create a Required instance for #Tempo.
    static Required make(double bpm, const NoteValue::Required& noteValue) { return { bpm, noteValue }; }

    MNX_REQUIRED_PROPERTY(double, bpm);             ///< the beats per minute of this tempo marking
    MNX_OPTIONAL_CHILD(RhythmicPosition, location,
        (const FractionValue&, position)); ///< location within the measure of the tempo marking
    MNX_REQUIRED_CHILD(NoteValue, value,
        (NoteValueBase, base), (unsigned, dots)); ///< the note value for the tempo.

    inline static constexpr std::string_view JsonSchemaTypeName = "tempo";     ///< required for mapping
};

/**
 * @class Measure
 * @brief Represents a single global measure instance within an MNX document.
 */
class Measure : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    MNX_OPTIONAL_CHILD(Barline, barline,
        (BarlineType, barlineType)); ///< optional barline for this measure
    MNX_OPTIONAL_CHILD(Ending, ending,
        (int, duration)); ///< optional ending ("volta bracket") for this measure
    MNX_OPTIONAL_CHILD(Fermata, fermata);           ///< Information about a fermata aligned with the barline at the end of this measure.
    MNX_OPTIONAL_CHILD(Fine, fine,
        (const FractionValue&, position)); ///< optional fine direction for this measure
    MNX_OPTIONAL_CHILD(Jump, jump,
        (JumpType, jumpType), (const FractionValue&, position)); ///< optional jump direction for this measure
    MNX_OPTIONAL_CHILD(KeySignature, key,
        (int, fifths)); ///< optional key signature/key change for this measure
    MNX_OPTIONAL_PROPERTY(int, number);             ///< visible measure number. Use #calcVisibleNumber to get the correct value,
                                                    ///< as defined in the MNX specification, since it may be omitted here.
    MNX_OPTIONAL_CHILD(RepeatEnd, repeatEnd);       ///< if present, indicates that there is backwards repeat
    MNX_OPTIONAL_CHILD(RepeatStart, repeatStart);   ///< if present, indicates that a repeated section starts here
    MNX_OPTIONAL_CHILD(Segno, segno,
        (const FractionValue&, position)); ///< if present, indicates that a segno marker is here
    MNX_OPTIONAL_CHILD(Array<Tempo>, tempos);       ///< the tempo changes within the measure, if any
    MNX_OPTIONAL_CHILD(TimeSignature, time,
        (int, count), (TimeSignatureUnit, unit)); ///< if present, indicates a meter change

    /// @brief Calculates the barline type for this measure.
    /// @return barline().type() if barline() has a value. Otherwise the default (as defined in the MNX specification.)
    [[nodiscard]] BarlineType calcBarlineType() const;

    /// @brief Calculates the visible measure number for this measure.
    /// @return The visible measure number. MNX currenty does not provide a mechanism to exclude a measure from numbering.
    [[nodiscard]] int calcVisibleNumber() const;

    /// @brief Calculates the current time signature by searching backwards.
    /// @return The current time signature or std::nullopt if none found.
    [[nodiscard]] std::optional<TimeSignature> calcCurrentTime() const;

    /// @brief Calculates the current key signature by searching backwards.
    /// @return The current key signature or or std::nullopt if none found.
    [[nodiscard]] std::optional<KeySignature> calcCurrentKey() const;

    /// @brief Get the fields of the current key signature.
    /// @return The current key fields if found, otherwise the fields for key with no accidentals.
    [[nodiscard]] KeySignature::Required calcCurrentKeyFields() const;

    inline static constexpr std::string_view JsonSchemaTypeName = "measure-global";     ///< required for mapping
};

/**
 * @class LyricLineMetadata
 * @brief Represents a single item of lyric metadata. The user-defined key is the lyric line ID.
 */
class LyricLineMetadata : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    MNX_OPTIONAL_PROPERTY(std::string, label);  ///< Human readable lable for this lyric line. This could be used in a software UI, for example.
    MNX_OPTIONAL_PROPERTY(std::string, lang);   ///< RFC5646 language code identifying the language of this line of lyrics.

    inline static constexpr std::string_view JsonSchemaTypeName = "lyric-line-metadata";     ///< required for mapping
};

/**
 * @class LyricsGlobal
 * @brief Metadata for lyrics in this MNX document
 */
class LyricsGlobal : public Object
{
public:
    using Object::Object;

    MNX_OPTIONAL_CHILD(Dictionary<LyricLineMetadata>, lineMetadata); ///< Defines lyric line IDs and their metadata.
    MNX_OPTIONAL_CHILD(Array<std::string>, lineOrder);              ///< Lyric line IDs used in the document (e.g. verse numbers)

    inline static constexpr std::string_view JsonSchemaTypeName = "lyrics-global";     ///< required for mapping
};

} // namespace global

/**
 * @class Global
 * @brief Represents the global section of an MNX document, containing global measures.
 */
class Global : public Object
{
public:
    using Object::Object;

    /// @brief Creates a new Global class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Global(Base& parent, std::string_view key)
        : Object(parent, key)
    {
        create_measures();
    }

    MNX_OPTIONAL_CHILD(global::LyricsGlobal, lyrics);       ///< lyrics metadata
    MNX_REQUIRED_CHILD(Array<global::Measure>, measures);   ///< array of global measures.
    MNX_OPTIONAL_CHILD(Dictionary<global::Sound>, sounds);  ///< dictionary of sounds with user-defined keys

    inline static constexpr std::string_view JsonSchemaTypeName = "global";     ///< required for mapping
};

} // namespace mnx
