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

namespace mnx {

/**
 * @enum AccidentalEnclosureSymbol
 * @brief The types of accidental enclosure symbols
 */
enum class AccidentalEnclosureSymbol
{
    Brackets,           ///< enclose accidental in brackets
    Parentheses         ///< enclose accidental in parentheses
};

/**
 * @enum AutoYesNo
 * @brief 3-state choice enum
 */
enum class AutoYesNo
{
    Auto,
    Yes,
    No
};

/**
 * @enum BarlineType
 * @brief The types of barlines supported.
 */
enum class BarlineType
{
    Regular,        ///< the default (normal barline)
    Dashed,         ///< dashed barline
    Dotted,         ///< dotted barline
    Double,         ///< two thin lines together
    Final,          ///< thin line followed by thick line
    Heavy,          ///< single thick line
    HeavyHeavy,     ///< two thick lines
    HeavyLight,     ///< thick ling followed by thin line
    NoBarline,      ///< no barline (barline is invisible)
    Short,          ///< length 2 spaces centered on middle staff line.
    Tick            ///< tick barline
};

/**
 * @enum  BeamHookDirection
 * @brief The types of jumps.
 */
enum class BeamHookDirection
{
    Auto,           ///< Automatic hook direction
    Left,           ///< Force hook left
    Right           ///< Force hook right
};

/**
 * @enum BreathMarkSymbol
 * @brief The symbols available for a breath mark
 * @note The first value is the MNX default: automatic choice by the consuming application.
 */
enum class BreathMarkSymbol
{
    Auto,    ///< automatically choose the breath mark symbol
    Comma,   ///< comma symbol
    Tick,    ///< tick symbol
    Upbow,   ///< upbow symbol
    Salzedo  ///< salzedo symbol
};

/**
 * @enum CaesuraShape
 * @brief The visual shape of a caesura
 * @note The first value is the MNX default.
 */
enum class CaesuraShape
{
    Normal,         ///< normal caesura (the default)
    Curved,         ///< curved caesura
    Short,          ///< short caesura
    Thick           ///< thick caesura
};

/**
 * @enum ClefSign
 * @brief The values available in a clef sign object
 */
enum class ClefSign
{
    CClef,          ///< C Clef
    FClef,          ///< F Clef
    GClef,          ///< G Clef
    PercussionClef  ///< Percussion clef
};

/**
 * @enum DirectionHint
 * @brief Hints whether a sequence is an upper or lower voice relative to other sequences in the same part measure
 * @note The first value is the default.
 */
enum class DirectionHint
{
    Auto,           ///< determined automatically by the consuming application
    Upper,          ///< upper voice
    Lower           ///< lower voice
};

/// @enum Dynamic Prefix
/// @brief Dynamic prefix values permitted by MNX
enum class DynamicPrefix
{
    None,           ///< empty string
    r,              ///< "rin-" as in rinforzando (rf)
    s,              ///< "s-" as in sforzando (sf)
};

/// @enum DynamicRelativeValue
/// @brief The possible relative changes in dynamic value
enum class DynamicRelativeValue
{
    Louder,             ///< relative increase
    Softer              ///< relative decrease
};

/// @enum Dynamic Suffix
/// @brief Dynamic suffix values permitted by MNX
enum class DynamicSuffix
{
    None,           ///< empty string
    z               ///< "-zato" as in forzato (fz)
};

/// @enum Dynamic Value
/// @brief Dynamic values permitted by MNX
enum class DynamicValue
{
    f,
    ff,
    fff,
    ffff,
    fffff,
    ffffff,
    mf,
    mp,
    n,
    p,
    pp,
    ppp,
    pppp,
    ppppp,
    pppppp,
};

/**
 * @enum DynamicWedgeType
 * @brief The types of dynamic wedges (hairpins)
 */
enum class DynamicWedgeType
{
    Decreasing,     ///< hairpin diminuendo
    Increasing      ///< hairpin crescendo
};

/**
 * @enum FermataDuration
 * @brief Specifies the subjective playback duration of a fermata.
 */
enum class FermataDuration
{
    Auto,           ///< unspecified (the default)
    Long,           ///< long fermata
    None,           ///< playback should ignore fermata
    Normal,         ///< normal length fermata
    Short,          ///< short fermata
    VeryLong,       ///< very long fermata
    VeryShort       ///< very short fermata
};

/**
 * @enum FermataSymbol
 * @brief Specifies the symbol style for the fermat.
 */
enum class FermataSymbol
{
    Normal,         ///< standard curved fermata symbol with single dot (the default)
    Angled,         ///< angled fermata symbol (often used to denote short duration)
    Curlew,         ///< double-curve (used by Britten in Curlew River to denote holding until a synchronization point)
    DoubleAngled,   ///< double-angled fermata symbol (often used to denote very short duration)
    DoubleDot,      ///< curved fermata symbol with two dots (used by Henze to denote long duration)
    DoubleSquare,   ///< double-square fermata symbol (often used to denote very long duration)
    HalfCurve,      ///< half-curve fermata symbol (used by Henze to denote short duration)
    Square          ///< square fermata symbol (often used to denote long duration)
};

/**
 * @enum FontStyle
 * @brief Specifies the style of text glyphs.
 */
enum class FontStyle
{
    Plain,          ///< plain (non-italic) text
    Italic          ///< italic text
};

/**
 * @enum FontWeight
 * @brief Specifies the weight of text glyphs.
 */
enum class FontWeight
{
    Plain,          ///< plain (non-bold) text
    Bold            ///< bold text
};

/**
 * @enum GraceType
 * @brief Options for how to perform grace notes.
 */
enum class GraceType
{
    StealPrevious,      ///< steal time from the previous non-grace.
    StealFollowing,     ///< steak time from the following non-grace.
    MakeTime            ///< add extra time for the grace note.
};

/**
 * @enum JumpType
 * @brief The types of jumps.
 */
enum class JumpType
{
    DsAlFine,       ///< D.S. al Fine
    Segno           ///< Dal Segno
};

/**
 * @enum LabelRef
 * @brief The values available in a labelref
 */
enum class LabelRef
{
    Name,           ///< the full name from the part (the default)
    ShortName       ///< the abbreviated name from the part
};

/**
 * @enum LayoutSymbol
 * @brief The symbols available to bracket a staff group
 */
enum class LayoutSymbol
{
    NoSymbol,       ///< the default (none)
    Brace,          ///< piano brace
    Bracket         ///< bracket
};

/**
 * @enum LineType
 * @brief Specifies the visual style of a line in (specifically) slurs
 */
enum class LineType
{
    Solid,   ///< A solid (unbroken) line
    Dashed,  ///< A dashed line
    Dotted,  ///< A dotted line
    Wavy     ///< A wavy line
};

/**
 * @enum LyricLineType
 * @brief The symbols available to bracket a staff group
 */
enum class LyricLineType
{
    End,            ///< the end of a word
    Middle,         ///< the middle of a word
    Start,          ///< the start of a word
    Whole           ///< this lyric is a whole word
};

/**
 * @enum MarkingUpDown
 * @brief Specifies up or down for a marking symbol
 */
enum class MarkingUpDown
{
    Down,           ///< down
    Up              ///< up
};

/**
 * @enum MarkingUpDownAuto
 * @brief Specifies up or down or auto for a marking symbol
 */
enum class MarkingUpDownAuto
{
    Auto,           ///< determined by consuming property
    Down,           ///< down (default)
    Up              ///< up
};

/**
 * @enum MultiStaffPlacement
 * @brief Specifies the vertical placement of a symbol with respect to its part's staves
 */
enum class MultiStaffPlacement
{
    Auto,       ///< the default value determined by implementation
    Above,      ///< symbol should be above its part staves
    Below,      ///< symbol should be below its part staves
    Between     ///< symbol should be between its part staves
};

/**
 * @enum NoteStep
 * @brief The diatonic note step values.
 *
 * These are maintained in order from [C..B] == [0..6] to allow correct math on them.
 */
enum class NoteStep : int
{
    C = 0,
    D = 1,
    E = 2,
    F = 3,
    G = 4,
    A = 5,
    B = 6
};

/**
 * @enum NoteValueBase
 * @brief The note values allowed in MNX
 */
enum class NoteValueBase
{
    Note4096th,
    Note2048th,
    Note1024th,
    Note512th,
    Note256th,
    Note128th,
    Note64th,
    Note32nd,
    Note16th,
    Eighth,
    Quarter,
    Half,
    Whole,
    Breve,
    Longa,
    Maxima,
    DuplexMaxima
};

/**
 * @enum OttavaAmount
 * @brief Valid values for ottava amount
 */
enum class OttavaAmount : int
{
    OctaveDown = -1,            ///< 8vb
    TwoOctavesDown = -2,        ///< 15mb
    ThreeOctavesDown = -3,      ///< 22mb
    OctaveUp = 1,               ///< 8va
    TwoOctavesUp = 2,           ///< 15ma
    ThreeOctavesUp = 3          ///< 22ma
};

/**
 * @enum OttavaAmountOrZero
 * @brief Valid values for octave displacment amount, including zero for no transposition.
 *
 * These values represent transposition intervals in octaves. Negative values
 * indicate downward transposition, positive values indicate upward transposition,
 * and zero indicates no transposition. This type may be used for clefs or other
 * elements where octave displacement is relevant.
 */
enum class OttavaAmountOrZero : int
{
    NoTransposition     = 0,   ///< No transposition
    ThreeOctavesDown    = -3,  ///< Transpose down three octaves
    TwoOctavesDown      = -2,  ///< Transpose down two octaves
    OctaveDown          = -1,  ///< Transpose down one octave
    OctaveUp            = 1,   ///< Transpose up one octave
    TwoOctavesUp        = 2,   ///< Transpose up two octaves
    ThreeOctavesUp      = 3    ///< Transpose up three octaves
};

/**
 * @enum Placement
 * @brief Specifies the vertical placement of a symbol or object relative to an event, staff, or other object
 */
enum class Placement
{
    Auto,       ///< the default value determined by implementation
    Above,      ///< glyph should be above its corresponding notation
    Below       ///< glyph should be below its corresponding notation
};

/**
 * @enum SlurTieSide
 * @brief Specifies the direction of a slur or tie. For a slur, it also specifies its placement
 * above or below the event it is attached to.
 * @note The first value is the MNX default.
 */
enum class SlurTieSide
{
    Auto,   ///< determined automatically by the consuming application (the default)
    Down,   ///< curved down, the tips face up
    Up,     ///< curved up, the tips face down
};

/**
 * @enum StaffGroupBarlineStyle
 * @brief The types of barline overrides for staff groups in layouts
 * @todo This appears to be a misnomer, and there is an outstanding issue 495 to clarify it.
 * Currently mnxdom treats these as override options. Of particular concern is the `instrument` option,
 * which adds useless complexity in the context of staff groups.
 */
enum class StaffGroupBarlineStyle
{
    Individual,     ///< currently interpreted as no override
    Instrument,     ///< currently interpreted as no override or unified, depending on whether the group is single- or multi-instrument
    Unified,        ///< override with unified barline
    Mensurstrich    ///< override with mensurstrich barline
};

/**
 * @enum StaffGroupBarlineOverride
 * @brief Resolved barline override setting for a layout staff group.
 * @note This enum is currently an mnxdom-only helper and is not an official MNX schema enum.
 * It models a proposed explicit override property that will replace StaffGroupBarlineStyle.
 */
enum class StaffGroupBarlineOverride
{
    None,           ///< no override
    Unified,        ///< override with unified barline
    Mensurstrich    ///< override with mensurstrich barline
};

/**
 * @enum StemDirection
 * @brief The values available in a labelref
 */
enum class StemDirection
{
    Down,           ///< stems down (default)
    Up              ///< stems up
};

/**
 * @enum TieTargetType
 * @brief The values that describe the target of a tie.
 */
enum class TieTargetType
{
    NextNote,       ///< The tie ends on the next adjacent note in the same voice. This is the most common case.
    Arpeggio,       ///< The tie is part of an arpeggio notated as consecutive ties.
    CrossJump,      ///< The tie crosses to a different ending or jump location.
    CrossVoice      ///< The tie end on the next adjacent note in a different voice.
};

/**
 * @enum TimeSignatureUnit
 * @brief Valid units for the lower numbers of time signatures
 */
enum class TimeSignatureUnit : int
{
    Whole = 1,
    Half = 2,
    Quarter = 4,
    Eighth = 8,
    Value16th = 16,
    Value32nd = 32,
    Value64th = 64,
    Value128th = 128
};

/**
 * @enum TimeSignatureDisplay
 * @brief Encodes visual glyphs for a time signature.
 */
enum class TimeSignatureDisplay
{
    Common,     ///< Use common time glyph instead of numbers
    Cut         ///< Use cut time glyph instead of numbers
};

/**
 * @enum TupletDisplaySetting
 * @brief Controls display of a tuplet's number or note value
 */
enum class TupletDisplaySetting
{
    Inner,
    NoNumber,
    Both
};

} // namespace mnx

#ifndef DOXYGEN_SHOULD_IGNORE_THIS

namespace nlohmann {

template<>
struct adl_serializer<mnx::TimeSignatureUnit>
{
    template<typename BasicJsonType>
    static mnx::TimeSignatureUnit from_json(const BasicJsonType& j)
    { return mnx::TimeSignatureUnit(j.template get<int>()); }

    template<typename BasicJsonType>
    static void to_json(BasicJsonType& j, const mnx::TimeSignatureUnit& value)
    { j = int(value); }
};

template<>
struct adl_serializer<mnx::OttavaAmount>
{
    template<typename BasicJsonType>
    static mnx::OttavaAmount from_json(const BasicJsonType& j)
    { return mnx::OttavaAmount(j.template get<int>()); }

    template<typename BasicJsonType>
    static void to_json(BasicJsonType& j, const mnx::OttavaAmount& value)
    { j = int(value); }
};

template<>
struct adl_serializer<mnx::OttavaAmountOrZero>
{
    template<typename BasicJsonType>
    static mnx::OttavaAmountOrZero from_json(const BasicJsonType& j)
    { return mnx::OttavaAmountOrZero(j.template get<int>()); }

    template<typename BasicJsonType>
    static void to_json(BasicJsonType& j, const mnx::OttavaAmountOrZero& value)
    { j = int(value); }
};

} // namespace nlohmann

#endif // DOXYGEN_SHOULD_IGNORE_THIS
