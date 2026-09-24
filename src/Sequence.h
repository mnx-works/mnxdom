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
#include "CommonClasses.h"
#include "Enumerations.h"
#include "EventMarkings.h"

namespace mnx {
class Sequence; // forward declaration

/**
 * @namespace mnx::sequence
 * @brief classes related to sequences in the part measure
 */
namespace sequence {
class Event;
class Grace;
class MultiNoteTremolo;
class Space;
class Tuplet;

class SequenceContentObject : public ContentObject<SequenceContentObject>
{
public:
    using ContentObject::ContentObject;
    std::string_view defaultType() const override { return "event"; }
};

class SequenceContent : public ContentArray<SequenceContentObject>
{
public:
    using ContentArray<SequenceContentObject>::ContentArray;

    Event appendEvent(NoteValueBase base, unsigned dots = 0);
    Grace appendGrace();
    MultiNoteTremolo appendMultiNoteTremolo(int numberOfMarks, const NoteValueQuantity::Required& noteValueQuant);
    Space appendSpace(const FractionValue& duration);
    Tuplet appendTuplet(const NoteValueQuantity::Required& innerNoteValueQuant,
        const NoteValueQuantity::Required& outerNoteValueQuant);
};

/**
 * @class AccidentalEnclosure
 * @brief Represents the enclosure on an accidental.
 */
class AccidentalEnclosure : public Object
{
public:
    /// @brief initializer class for #AccidentalEnclosure
    struct Required
    {
        AccidentalEnclosureSymbol symbol{}; ///< the symbol to use for the enclosure
    };

    /// @brief Constructor for existing AccidentalEnclosure objects
    AccidentalEnclosure(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Pitch class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in the parent.
    /// @param symbol The symbol to use for the enclosure
    AccidentalEnclosure(Base& parent, std::string_view key, AccidentalEnclosureSymbol symbol)
        : Object(parent, key)
    {
        set_symbol(symbol);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { symbol() }; }

    /// @brief Create a Required instance for #AccidentalEnclosure.
    static Required make(AccidentalEnclosureSymbol symbol) { return { symbol }; }

    MNX_REQUIRED_PROPERTY(AccidentalEnclosureSymbol, symbol);      ///< The symbol to use for the enclosure

    inline static constexpr std::string_view JsonSchemaTypeName = "accidental-enclosure";     ///< required for mapping
};

/**
 * @class AccidentalDisplay
 * @brief Represents an explicit directive to show or hide an accidental.
 */
class AccidentalDisplay : public Object
{
public:
    /// @brief initializer class for #AccidentalDisplay
    struct Required
    {
        bool show{}; ///< whether to show or hide the accidental
    };

    /// @brief Constructor for existing AccidentalEnclosure objects
    AccidentalDisplay(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Pitch class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in the parent.
    /// @param show Whether to show or hide the accidental
    AccidentalDisplay(Base& parent, std::string_view key, bool show)
        : Object(parent, key)
    {
        set_show(show);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { show() }; }

    /// @brief Create a Required instance for #AccidentalDisplay.
    static Required make(bool show) { return { show }; }

    MNX_OPTIONAL_CHILD(AccidentalEnclosure, enclosure,
        (AccidentalEnclosureSymbol, symbol)); ///< The enclosure type (brackets or parentheses). Omit if none.
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, force, false); ///< Whether this accidental was set intentionally (e.g., a courtesy accidental).
    MNX_REQUIRED_PROPERTY(bool, show);                      ///< Whether to show or hide the accidental

    inline static constexpr std::string_view JsonSchemaTypeName = "accidental-display";     ///< required for mapping
};

/**
 * @class Rest
 * @brief Represents a rest within a musical event within a sequence.
 */
class Rest : public Object
{
public:
    using Object::Object;

    MNX_OPTIONAL_PROPERTY(int, staffPosition);              ///< The staff position of non-floating rests.

    inline static constexpr std::string_view JsonSchemaTypeName = "rest";     ///< required for mapping
};

/**
 * @class TransposeWritten
 * @brief Represents the options for how a note is written when transposed
 */
class TransposeWritten : public Object
{
public:
    using Object::Object;

    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(int, diatonicDelta, 0);  ///< the number of enharmonic transpositions to apply

    inline static constexpr std::string_view JsonSchemaTypeName = "written";     ///< required for mapping
};

/**
 * @class Pitch
 * @brief Represents the pitch of a note
 */
class Pitch : public Object
{
public:
    /// @brief initializer class for #Pitch
    struct Required
    {
        NoteStep step{};    ///< the letter spelling of the note.
        int octave{};       ///< the octave number of the note (where C4 is middle C).
        int alter{};        ///< the chromatic alteration of the note (positive for sharp, negative for flat).
    };

    /// @brief Constructor for existing Pitch objects
    Pitch(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Pitch class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param step The letter spelling of the note.
    /// @param octave The octave number of the note (where C4 is middle C).
    /// @param alter The chromatic alteration of the note (positive for sharp, negative for flat).
    Pitch(Base& parent, std::string_view key, NoteStep step, int octave, int alter = 0)
        : Object(parent, key)
    {
        set_step(step);
        set_octave(octave);
        set_or_clear_alter(alter);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { step(), octave(), alter() }; }

    /// @brief Create a Required instance for #Pitch.
    static Required make(NoteStep step, int octave, int alter = 0) { return { step, octave, alter }; }

    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(int, alter, 0);  ///< chromatic alteration
    MNX_REQUIRED_PROPERTY(int, octave);         ///< the octave number
    MNX_REQUIRED_PROPERTY(NoteStep, step);      ///< the note step, (i.e., "A".."G")

    /// @brief Checks if the input pitch is the same as this pitch, including enharmonic equivalents
    /// @param src The value to compare with
    /// @return true if they are the same or enharmonically equivalent
    [[nodiscard]] bool isSamePitch(const Required& src) const;

    /// @brief Calculates the transposed version of this pitch, taking into account the part transposition
    /// for the part that contains this pitch.
    [[nodiscard]] Required calcTransposed() const;

    inline static constexpr std::string_view JsonSchemaTypeName = "pitch";     ///< required for mapping
};

/**
 * @class Slur
 * @brief Contains information about a tie on a note.
 */
class Slur : public ArrayElementObject
{
public:
    /// @brief initializer class for #Slur
    struct Required
    {
        std::string target; ///< the target note id of the slur
    };

    /// @brief Constructor for existing Slur objects
    Slur(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new Slur class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param target The target note id of the slur
    Slur(Base& parent, std::string_view key, const std::string& target)
        : ArrayElementObject(parent, key)
    {
        set_target(target);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { target() }; }

    /// @brief Create a Required instance for #Slur.
    static Required make(const std::string& target) { return { target }; }

    MNX_OPTIONAL_PROPERTY(std::string, endNote);            ///< the specific note ID this slur ends on
    MNX_OPTIONAL_PROPERTY(LineType, lineType);              ///< the type of line for the slur
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(SlurTieSide, side, SlurTieSide::Auto);    ///< the slur's side (direction and placement)
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(SlurTieSide, sideEnd, SlurTieSide::Auto); ///< the slur's side at its endpoint (if different than `side`)
    MNX_OPTIONAL_PROPERTY(std::string, startNote);          ///< the specific note ID this slur starts on
    MNX_REQUIRED_PROPERTY(std::string, target);             ///< the event ID this slur ends on (if present)

    inline static constexpr std::string_view JsonSchemaTypeName = "slur";     ///< required for mapping
};

/**
 * @class Tie
 * @brief Contains information about a tie on a note.
 */
class Tie : public ArrayElementObject
{
public:
    /// @brief Constructor for existing Tie objects
    Tie(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new Tie class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Tie(Base& parent, std::string_view key)
        : ArrayElementObject(parent, key)
    {}

    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, lv, false);    ///< Indicates the presence of an l.v. tie (instead of target).
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(SlurTieSide, side, SlurTieSide::Auto);    ///< Whether the tie curves upward or downward.
    MNX_OPTIONAL_PROPERTY(std::string, target);             ///< The note id of the tied-to note.
    MNX_OPTIONAL_PROPERTY(TieTargetType, targetType);       ///< The type of target for this tie (if present).

    inline static constexpr std::string_view JsonSchemaTypeName = "tie";     ///< required for mapping
};

/**
 * @class PerformOptions
 * @brief Controls whether a note is performed by consuming software.
 *
 * The MNX schema currently defines this as an empty object, with support for
 * the standard MNX global attributes inherited from @ref Object.
 */
class PerformOptions : public Object
{
public:
    using Object::Object;

    inline static constexpr std::string_view JsonSchemaTypeName = "perform-options";     ///< required for mapping
};

/**
 * @class NoteBase
 * @brief Represents common elements between @ref Note and @ref KitNote
 */
class NoteBase : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    MNX_OPTIONAL_CHILD(PerformOptions, perform);                    ///< Controls whether this note is performed by consuming software.
    MNX_OPTIONAL_PROPERTY(int, staff);                              ///< Staff number override (e.g., for cross-staff notes.)
    MNX_OPTIONAL_CHILD(Array<Tie>, ties);                           ///< The (forward) ties, if any.
};

/**
 * @class KitNote
 * @brief Represents a single note in a (drum) kit within a musical event within a sequence.
 */
class KitNote : public NoteBase
{
public:
    /// @brief initializer class for #KitNote
    struct Required
    {
        std::string kitComponentId; ///< the ID within the kit for this part
    };

    /// @brief Constructor for existing Note objects
    KitNote(const std::shared_ptr<json>& root, json_pointer pointer)
        : NoteBase(root, pointer)
    {
    }

    /// @brief Creates a new Note class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param kitComponentId The ID within the kit for this part
    KitNote(Base& parent, std::string_view key, const std::string& kitComponentId)
        : NoteBase(parent, key)
    {
        set_kitComponent(kitComponentId);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { kitComponent() }; }

    /// @brief Create a Required instance for #KitNote.
    static Required make(const std::string& kitComponentId) { return { kitComponentId }; }

    MNX_REQUIRED_PROPERTY(std::string, kitComponent);               ///< The ID within the kit for this part.

    inline static constexpr std::string_view JsonSchemaTypeName = "kit-note";     ///< required for mapping
};

/**
 * @class Note
 * @brief Represents a single note (i.e., within a chord) within a musical event within a sequence.
 */
class Note : public NoteBase
{
public:
    /// @brief initializer class for #Note
    struct Required
    {
        Pitch::Required pitch{}; ///< the pitch of the note
    };

    /// @brief Constructor for existing Note objects
    Note(const std::shared_ptr<json>& root, json_pointer pointer)
        : NoteBase(root, pointer)
    {
    }

    /// @brief Creates a new Note class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param pitch The pitch fields to use
    Note(Base& parent, std::string_view key, const Pitch::Required& pitch)
        : NoteBase(parent, key)
    {
        create_pitch(pitch.step, pitch.octave, pitch.alter);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { pitch() }; }

    /// @brief Create a Required instance for #Note.
    static Required make(const Pitch::Required& pitch) { return { pitch }; }

    MNX_OPTIONAL_CHILD(
        AccidentalDisplay, accidentalDisplay,
        (bool, show)); ///< the forced show/hide state of the accidental
    MNX_REQUIRED_CHILD(Pitch, pitch, (NoteStep, step), (int, octave), (int, alter)); ///< the pitch of the note
    MNX_OPTIONAL_CHILD(TransposeWritten, written);                  ///< How to write this note when it is displayed transposed

    inline static constexpr std::string_view JsonSchemaTypeName = "note";     ///< required for mapping
};

/**
 * @class EventLyricLine
 * @brief Contains information about a lyric syllable from one lyric line on a note.
 */
class EventLyricLine : public ArrayElementObject
{
public:
    /// @brief initializer class for #EventLyricLine
    struct Required
    {
        std::string syllableText; ///< the syllable text
    };

    /// @brief Constructor for existing EventLyricLine objects
    EventLyricLine(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new EventLyricLine class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param syllableText The syllable text
    EventLyricLine(Base& parent, std::string_view key, const std::string& syllableText)
        : ArrayElementObject(parent, key)
    {
        set_text(syllableText);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { text() }; }

    /// @brief Create a Required instance for #EventLyricLine.
    static Required make(const std::string& syllableText) { return { syllableText }; }

    MNX_REQUIRED_PROPERTY(std::string, text);           ///< the syllable text
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(LyricLineType, type, LyricLineType::Whole);  ///< the type of syllable (in relation to the complete word)

    inline static constexpr std::string_view JsonSchemaTypeName = "event-lyric-line";     ///< required for mapping
};

/**
 * @class EventLyrics
 * @brief Contains information about the lyric syllables on the event.
 */
class EventLyrics : public Object
{
public:
    using Object::Object;

    MNX_OPTIONAL_CHILD(Dictionary<EventLyricLine>, lines);      ///< the syllables per lyric line

    inline static constexpr std::string_view JsonSchemaTypeName = "lyrics";     ///< required for mapping
};

/**
 * @class Event
 * @brief Represents a musical event within a sequence.
 */
class Event : public SequenceContentObject
{
public:
    /// @brief initializer class for #Event
    struct Required
    {
        NoteValue::Required duration{}; ///< Symbolic duration of the event.
    };

    /// @brief Constructor for existing Event objects
    Event(const std::shared_ptr<json>& root, json_pointer pointer)
        : SequenceContentObject(root, pointer)
    {
    }

    /// @brief Creates a new Event class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param base The note value base for the event duration.
    /// @param dots Number of dots on the event duration.
    Event(Base& parent, std::string_view key, NoteValueBase base, unsigned dots = 0)
        : SequenceContentObject(parent, key)
    {
        create_duration(base, dots);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { duration() }; }

    /// @brief Create a Required instance for #Event.
    static Required make(NoteValueBase base, unsigned dots = 0) { return { { base, dots } }; }

    MNX_REQUIRED_CHILD(NoteValue, duration,
        (NoteValueBase, base), (unsigned, dots));           ///< Symbolic duration of the event.
    MNX_OPTIONAL_CHILD(Fermata, fermata);                   ///< Information about a fermata on the event, if any.
    MNX_OPTIONAL_CHILD(Array<KitNote>, kitNotes);           ///< KitNote array for percussion kit notation.
    MNX_OPTIONAL_CHILD(EventLyrics, lyrics);                ///< The lyric syllables on this event.
    MNX_OPTIONAL_CHILD(EventMarkings, markings);            ///< Articulation markings on this event.
    MNX_OPTIONAL_CHILD(Array<Note>, notes);                 ///< Note array
    MNX_OPTIONAL_CHILD(Rest, rest);                         ///< indicates this event is a rest.
    MNX_OPTIONAL_CHILD(Array<Slur>, slurs);                 ///< The slurs that start on this event.
    MNX_OPTIONAL_PROPERTY(int, staff);                      ///< Staff number override (e.g., for cross-staff events.)
    MNX_OPTIONAL_PROPERTY(StemDirection, stemDirection);    ///< Forced stem direction.

    /// @brief Finds a note in the event by its ID
    /// @param noteId The note ID to find
    /// @return The note if found, otherwise std::nullopt;
    [[nodiscard]] std::optional<Note> findNote(const std::string& noteId) const;

    /// @brief Returns true if this event is part of a grace note sequence.
    [[nodiscard]] bool isGrace() const;

    /// @brief Returns true if this event is part of a multi-note tremolo sequence.
    [[nodiscard]] bool isTremolo() const;

    /// @brief Returns the @ref Sequence instance for this event
    /// @throws std::logic_error if the json pointer does not contain a sequence (should be impossible)
    [[nodiscard]] Sequence getSequence() const;

    /// @brief Returns the index of the event (or its container) in the @ref Sequence instance for this event
    /// @throws std::logic_error if the json pointer does not contain a sequence (should be impossible)
    [[nodiscard]] size_t getSequenceIndex() const;

    /// @brief Calculates and returns the start time of this event within the measure.
    [[nodiscard]] FractionValue calcStartTime() const;

    inline static constexpr std::string_view ContentTypeValue = "event";    ///< type value that identifies the type within the content array
    inline static constexpr std::string_view JsonSchemaTypeName = "event";  ///< required for mapping
};

/**
 * @class Space
 * @brief Occupies metric space without showing anything.
 */
class Space : public SequenceContentObject
{
public:
    /// @brief initializer class for #Space
    struct Required
    {
        FractionValue duration{}; ///< duration of the space
    };

    /// @brief Constructor for existing Space objects
    Space(const std::shared_ptr<json>& root, json_pointer pointer)
        : SequenceContentObject(root, pointer)
    {
    }

    /// @brief Creates a new Space class as a child of a JSON element.
    /// @param parent The parent class instance.
    /// @param key The JSON key to use for embedding in parent.
    /// @param duration The duration of the space
    Space(Base& parent, std::string_view key, const FractionValue& duration)
        : SequenceContentObject(parent, key)
    {
        create_duration(duration);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { duration() }; }

    /// @brief Create a Required instance for #Space.
    static Required make(const FractionValue& duration) { return { duration }; }

    MNX_REQUIRED_CHILD(
        Fraction, duration,
        (const FractionValue&, value)); ///< Duration of space to occupy.

    inline static constexpr std::string_view ContentTypeValue = "space";   ///< type value that identifies the type within the content array

    inline static constexpr std::string_view JsonSchemaTypeName = "space";     ///< required for mapping
};

/**
 * @class Grace
 * @brief Represents a grace note sequence within a sequence.
 */
class Grace : public SequenceContentObject
{
public:
    /// @brief Constructor for existing Grace objects
    Grace(const std::shared_ptr<json>& root, json_pointer pointer)
        : SequenceContentObject(root, pointer)
    {
    }

    /// @brief Creates a new Grace class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Grace(Base& parent, std::string_view key)
        : SequenceContentObject(parent, key)
    {
        create_content();
    }

    MNX_OPTIONAL_PROPERTY(std::string, color);                      ///< color to use when rendering the grace note sequence
    MNX_REQUIRED_CHILD(SequenceContent, content);                      ///< array of events
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(GraceType, graceType, GraceType::StealPrevious); ///< The playback type of the grace note.
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, slash, true);          ///< whether to show a slash on the grace note

    inline static constexpr std::string_view ContentTypeValue = "grace";   ///< type value that identifies the type within the content array

    inline static constexpr std::string_view JsonSchemaTypeName = "grace";     ///< required for mapping
};

/**
 * @class MultiNoteTremolo
 * @brief Represents a multi-note tremolo sequence within a sequence.
 */
class MultiNoteTremolo : public SequenceContentObject
{
public:
    /// @brief initializer class for #MultiNoteTremolo
    struct Required
    {
        int numberOfMarks{};                   ///< the number of marks (beams)
        NoteValueQuantity::Required noteValueQuant{}; ///< the note value quantity
    };

    /// @brief Constructor for existing Tuplet objects
    MultiNoteTremolo(const std::shared_ptr<json>& root, json_pointer pointer)
        : SequenceContentObject(root, pointer)
    {
    }

    /// @brief Creates a new MultiNoteTremolo class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param numberOfMarks The number of marks (beams)
    /// @param noteValueQuant The note value quantity
    MultiNoteTremolo(Base& parent, std::string_view key, int numberOfMarks, const NoteValueQuantity::Required& noteValueQuant)
        : SequenceContentObject(parent, key)
    {
        create_content();
        set_marks(numberOfMarks);
        create_outer(noteValueQuant.count, noteValueQuant.noteValue);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { marks(), outer() }; }

    /// @brief Create a Required instance for #MultiNoteTremolo.
    static Required make(int numberOfMarks, const NoteValueQuantity::Required& noteValueQuant)
    { return { numberOfMarks, noteValueQuant }; }

    MNX_REQUIRED_CHILD(SequenceContent, content);                      ///< array of events
    MNX_OPTIONAL_CHILD(
        NoteValue, individualDuration,
        (NoteValueBase, base), (unsigned, dots)); ///< optional value that specifies the individual duration of each event in the tremolo.
    MNX_REQUIRED_PROPERTY(int, marks);                              ///< the number of marks (beams)
    MNX_REQUIRED_CHILD(NoteValueQuantity, outer,
        (unsigned, count), (const NoteValue::Required&, noteValue)); ///< a half note tremolo would be 2 quarters here

    inline static constexpr std::string_view ContentTypeValue = "tremolo";   ///< type value that identifies the type within the content array

    inline static constexpr std::string_view JsonSchemaTypeName = "multi-note-tremolo";     ///< required for mapping
};

/**
 * @class Tuplet
 * @brief Represents a tuplet sequence within a sequence.
 */
class Tuplet : public SequenceContentObject
{
public:
    /// @brief initializer class for #Tuplet
    struct Required
    {
        NoteValueQuantity::Required innerNoteValueQuant{}; ///< inner amount
        NoteValueQuantity::Required outerNoteValueQuant{}; ///< outer amount
    };

    /// @brief Constructor for existing Tuplet objects
    Tuplet(const std::shared_ptr<json>& root, json_pointer pointer)
        : SequenceContentObject(root, pointer)
    {
    }

    /// @brief Creates a new Tuplet class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param innerNoteValueQuant Inner amount
    /// @param outerNoteValueQuant Outer amount
    Tuplet(Base& parent, std::string_view key, const NoteValueQuantity::Required& innerNoteValueQuant, const NoteValueQuantity::Required& outerNoteValueQuant)
        : SequenceContentObject(parent, key)
    {
        create_inner(innerNoteValueQuant.count, innerNoteValueQuant.noteValue);
        create_outer(outerNoteValueQuant.count, outerNoteValueQuant.noteValue);
        create_content();
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { inner(), outer() }; }

    /// @brief Create a Required instance for #Tuplet.
    static Required make(const NoteValueQuantity::Required& innerNoteValueQuant, const NoteValueQuantity::Required& outerNoteValueQuant)
    { return { innerNoteValueQuant, outerNoteValueQuant }; }

    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(AutoYesNo, bracket, AutoYesNo::Auto); ///< bracket style
    MNX_REQUIRED_CHILD(SequenceContent, content);                      ///< array of events, tuplets, and grace notes
    MNX_REQUIRED_CHILD(NoteValueQuantity, inner,
        (unsigned, count), (const NoteValue::Required&, noteValue)); ///< Inner quantity: **3 quarters in the time** of 2 quarters
    MNX_REQUIRED_CHILD(NoteValueQuantity, outer,
        (unsigned, count), (const NoteValue::Required&, noteValue)); ///< Outer quantity: 3 quarters in the time **of 2 quarters**
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(Placement, placement, Placement::Auto); ///< Whether the tuplet's brackets and numbers are above or below its content
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(TupletDisplaySetting, showNumber, TupletDisplaySetting::Inner); ///< How and whether to show the tuplet number(s)
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(TupletDisplaySetting, showValue, TupletDisplaySetting::NoNumber); ///< How and whether to show the tuplet note value(s)
    MNX_OPTIONAL_PROPERTY(int, staff);                              ///< Staff number override (e.g., for cross-staff notes.)

    /// @brief Return the triplet ratio as a FractionValue
    [[nodiscard]] FractionValue ratio() const
    { return outer() / inner(); }

    inline static constexpr std::string_view ContentTypeValue = "tuplet";   ///< type value that identifies the type within the content array

    inline static constexpr std::string_view JsonSchemaTypeName = "tuplet";     ///< required for mapping
};

/**
 * @class FullMeasureRest
 * @brief Represents a page in a score.
 */
class FullMeasureRest : public Object
{
public:
    using Object::Object;

    /// @brief Creates a new Page class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    FullMeasureRest(Base& parent, std::string_view key)
        : Object(parent, key)
    {}

    MNX_OPTIONAL_CHILD(Fermata, fermata);           ///< Information about a fermata on the full measure rest, if any.
    MNX_OPTIONAL_PROPERTY(int, staffPosition);      ///< the forced staff position of the full-measure rest
    MNX_OPTIONAL_CHILD(NoteValue, visualDuration,
        (NoteValueBase, base), (unsigned, dots));   ///< the visual duration of the full-measure rest (defaults to importer defaults).

    inline static constexpr std::string_view JsonSchemaTypeName = "full-measure-rest";     ///< required for mapping
};
inline Event SequenceContent::appendEvent(NoteValueBase base, unsigned dots)
{ return appendWithType<Event>(base, dots); }

inline Grace SequenceContent::appendGrace()
{ return appendWithType<Grace>(); }

inline MultiNoteTremolo SequenceContent::appendMultiNoteTremolo(int numberOfMarks, const NoteValueQuantity::Required& noteValueQuant)
{ return appendWithType<MultiNoteTremolo>(numberOfMarks, noteValueQuant); }

inline Space SequenceContent::appendSpace(const FractionValue& duration)
{ return appendWithType<Space>(duration); }

inline Tuplet SequenceContent::appendTuplet(const NoteValueQuantity::Required& innerNoteValueQuant,
    const NoteValueQuantity::Required& outerNoteValueQuant)
{ return appendWithType<Tuplet>(innerNoteValueQuant, outerNoteValueQuant); }

} // namespace sequence

/**
 * @class Sequence
 * @brief A sequence of events and other items in this measure for a voice in a part
 */
class Sequence : public ArrayElementObject
{
public:
    /// @brief Constructor for existing Sequence objects
    Sequence(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }

    /// @brief Creates a new Sequence class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Sequence(Base& parent, std::string_view key)
        : ArrayElementObject(parent, key)
    {
        create_content();
    }

    MNX_REQUIRED_CHILD(sequence::SequenceContent, content);          ///< the content of the sequence
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(DirectionHint, directionHint, DirectionHint::Auto); ///< Hint whether this sequence is an upper or lower voice relative to other sequences in the part measure.
    MNX_OPTIONAL_CHILD(sequence::FullMeasureRest, fullMeasure); ///< If present, this sequence is a forced full-measure rest.
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(int, staff, 1);  ///< the staff number for this sequence
    MNX_OPTIONAL_PROPERTY(std::string, voice);          ///< the unique (per measure) voice label for this sequence.

    inline static constexpr std::string_view JsonSchemaTypeName = "sequence";     ///< required for mapping
};

} // namespace mnx
