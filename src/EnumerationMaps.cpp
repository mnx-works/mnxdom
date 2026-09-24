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
#include <string>
#include <unordered_map>

#include "mnxdom.h"

#define MNX_ENUM_MAPPING(EnumType, ...)                                       \
template <>                                                                   \
const std::unordered_map<std::string, EnumType>                               \
EnumStringMapping<EnumType>::stringToEnum() {                                 \
    static const std::unordered_map<std::string, EnumType> map = __VA_ARGS__;  \
    return map;                                                               \
}                                                                             \
static_assert(true, "") // forces semicolon after macro invocation

namespace mnx {

#ifndef DOXYGEN_SHOULD_IGNORE_THIS

MNX_ENUM_MAPPING(AccidentalEnclosureSymbol, {
    { "brackets",       AccidentalEnclosureSymbol::Brackets },
    { "parentheses",    AccidentalEnclosureSymbol::Parentheses }
});

MNX_ENUM_MAPPING(AutoYesNo, {
    { "auto", AutoYesNo::Auto },
    { "yes",  AutoYesNo::Yes },
    { "no",   AutoYesNo::No }
});

MNX_ENUM_MAPPING(BarlineType, {
    { "regular",     BarlineType::Regular },
    { "dashed",      BarlineType::Dashed },
    { "dotted",      BarlineType::Dotted },
    { "double",      BarlineType::Double },
    { "final",       BarlineType::Final },
    { "heavy",       BarlineType::Heavy },
    { "heavyHeavy",  BarlineType::HeavyHeavy },
    { "heavyLight",  BarlineType::HeavyLight },
    { "noBarline",   BarlineType::NoBarline },
    { "short",       BarlineType::Short },
    { "tick",        BarlineType::Tick }
});

MNX_ENUM_MAPPING(BeamHookDirection, {
    { "auto",       BeamHookDirection::Auto },
    { "left",       BeamHookDirection::Left },
    { "right",      BeamHookDirection::Right }
});

MNX_ENUM_MAPPING(BreathMarkSymbol, {
    { "auto",       BreathMarkSymbol::Auto },
    { "comma",      BreathMarkSymbol::Comma },
    { "tick",       BreathMarkSymbol::Tick },
    { "upbow",      BreathMarkSymbol::Upbow },
    { "salzedo",    BreathMarkSymbol::Salzedo }
});

MNX_ENUM_MAPPING(CaesuraShape, {
    { "normal",     CaesuraShape::Normal },
    { "curved",     CaesuraShape::Curved },
    { "short",      CaesuraShape::Short },
    { "thick",      CaesuraShape::Thick }
});

MNX_ENUM_MAPPING(ClefSign, {
    { "C",          ClefSign::CClef },
    { "F",          ClefSign::FClef },
    { "G",          ClefSign::GClef },
    { "P",          ClefSign::PercussionClef }
});

MNX_ENUM_MAPPING(DirectionHint, {
    { "auto",       DirectionHint::Auto },
    { "upper",      DirectionHint::Upper },
    { "lower",      DirectionHint::Lower }
});

MNX_ENUM_MAPPING(DynamicPrefix, {
    { "",           DynamicPrefix::None },
    { "r",          DynamicPrefix::r },
    { "s",          DynamicPrefix::s }
});

MNX_ENUM_MAPPING(DynamicRelativeValue, {
    { "louder",     DynamicRelativeValue::Louder },
    { "softer",     DynamicRelativeValue::Softer }
});

MNX_ENUM_MAPPING(DynamicSuffix, {
    { "",           DynamicSuffix::None },
    { "z",          DynamicSuffix::z }
});

MNX_ENUM_MAPPING(DynamicValue, {
    { "f",          DynamicValue::f },
    { "ff",         DynamicValue::ff },
    { "fff",        DynamicValue::fff },
    { "ffff",       DynamicValue::ffff },
    { "fffff",      DynamicValue::fffff },
    { "ffffff",     DynamicValue::ffffff },
    { "mf",         DynamicValue::mf },
    { "mp",         DynamicValue::mp },
    { "n",          DynamicValue::n },
    { "p",          DynamicValue::p },
    { "pp",         DynamicValue::pp },
    { "ppp",        DynamicValue::ppp },
    { "pppp",       DynamicValue::pppp },
    { "ppppp",      DynamicValue::ppppp },
    { "pppppp",     DynamicValue::pppppp },
});

MNX_ENUM_MAPPING(DynamicWedgeType, {
    { "decreasing", DynamicWedgeType::Decreasing },
    { "increasing", DynamicWedgeType::Increasing }
});

MNX_ENUM_MAPPING(FermataDuration, {
    { "auto",       FermataDuration::Auto },
    { "long",       FermataDuration::Long },
    { "none",       FermataDuration::None },
    { "normal",     FermataDuration::Normal },
    { "short",      FermataDuration::Short },
    { "veryLong",   FermataDuration::VeryLong },
    { "veryShort",  FermataDuration::VeryShort }
});

MNX_ENUM_MAPPING(FermataSymbol, {
    { "normal",         FermataSymbol::Normal },
    { "angled",         FermataSymbol::Angled },
    { "curlew",         FermataSymbol::Curlew },
    { "doubleAngled",   FermataSymbol::DoubleAngled },
    { "doubleDot",      FermataSymbol::DoubleDot },
    { "doubleSquare",   FermataSymbol::DoubleSquare },
    { "halfCurve",      FermataSymbol::HalfCurve },
    { "square",         FermataSymbol::Square }
});

MNX_ENUM_MAPPING(FontStyle, {
    { "plain",          FontStyle::Plain },
    { "italic",         FontStyle::Italic }
});

MNX_ENUM_MAPPING(FontWeight, {
    { "plain",          FontWeight::Plain },
    { "bold",           FontWeight::Bold }
});

MNX_ENUM_MAPPING(GraceType, {
    { "makeTime",       GraceType::MakeTime },
    { "stealFollowing", GraceType::StealFollowing },
    { "stealPrevious",  GraceType::StealPrevious }
});

MNX_ENUM_MAPPING(JumpType, {
    { "dsalfine",   JumpType::DsAlFine },
    { "segno",      JumpType::Segno }
});

MNX_ENUM_MAPPING(LabelRef, {
    { "name",       LabelRef::Name },
    { "shortName",  LabelRef::ShortName }
});

MNX_ENUM_MAPPING(LayoutSymbol, {
    { "noSymbol",   LayoutSymbol::NoSymbol },
    { "brace",      LayoutSymbol::Brace },
    { "bracket",    LayoutSymbol::Bracket }
});

MNX_ENUM_MAPPING(LineType, {
    { "solid",      LineType::Solid },
    { "dashed",     LineType::Dashed },
    { "dotted",     LineType::Dotted },
    { "wavy",       LineType::Wavy }
});

MNX_ENUM_MAPPING(LyricLineType, {
    { "end",        LyricLineType::End },
    { "middle",     LyricLineType::Middle },
    { "start",      LyricLineType::Start },
    { "whole",      LyricLineType::Whole }
});

MNX_ENUM_MAPPING(MarkingUpDown, {
    { "down",       MarkingUpDown::Down },
    { "up",         MarkingUpDown::Up }
});

MNX_ENUM_MAPPING(MarkingUpDownAuto, {
    { "auto",       MarkingUpDownAuto::Auto},
    { "down",       MarkingUpDownAuto::Down },
    { "up",         MarkingUpDownAuto::Up }
});

MNX_ENUM_MAPPING(MultiStaffPlacement, {
    { "auto",       MultiStaffPlacement::Auto },
    { "above",      MultiStaffPlacement::Above },
    { "below",      MultiStaffPlacement::Below },
    { "between",    MultiStaffPlacement::Between }
});

MNX_ENUM_MAPPING(NoteStep, {
    { "C", NoteStep::C },
    { "D", NoteStep::D },
    { "E", NoteStep::E },
    { "F", NoteStep::F },
    { "G", NoteStep::G },
    { "A", NoteStep::A },
    { "B", NoteStep::B }
});

MNX_ENUM_MAPPING(NoteValueBase, {
    { "4096th",       NoteValueBase::Note4096th },
    { "2048th",       NoteValueBase::Note2048th },
    { "1024th",       NoteValueBase::Note1024th },
    { "512th",        NoteValueBase::Note512th },
    { "256th",        NoteValueBase::Note256th },
    { "128th",        NoteValueBase::Note128th },
    { "64th",         NoteValueBase::Note64th },
    { "32nd",         NoteValueBase::Note32nd },
    { "16th",         NoteValueBase::Note16th },
    { "eighth",       NoteValueBase::Eighth },
    { "quarter",      NoteValueBase::Quarter },
    { "half",         NoteValueBase::Half },
    { "whole",        NoteValueBase::Whole },
    { "breve",        NoteValueBase::Breve },
    { "longa",        NoteValueBase::Longa },
    { "maxima",       NoteValueBase::Maxima },
    { "duplexMaxima", NoteValueBase::DuplexMaxima }
});

MNX_ENUM_MAPPING(Placement, {
    { "auto",       Placement::Auto },
    { "above",      Placement::Above },
    { "below",      Placement::Below }
});

MNX_ENUM_MAPPING(SlurTieSide, {
    { "auto",       SlurTieSide::Auto },
    { "down",       SlurTieSide::Down },
    { "up",         SlurTieSide::Up }
});

MNX_ENUM_MAPPING(StaffGroupBarlineStyle, {
    { "individual",     StaffGroupBarlineStyle::Individual },
    { "instrument",     StaffGroupBarlineStyle::Instrument },
    { "unified",        StaffGroupBarlineStyle::Unified },
    { "mensurstrich",   StaffGroupBarlineStyle::Mensurstrich }
});

MNX_ENUM_MAPPING(StemDirection, {
    { "down",       StemDirection::Down },
    { "up",         StemDirection::Up }
});

MNX_ENUM_MAPPING(TieTargetType, {
    { "nextNote",   TieTargetType::NextNote },
    { "crossVoice", TieTargetType::CrossVoice },
    { "arpeggio",   TieTargetType::Arpeggio },
    { "crossJump",  TieTargetType::CrossJump }
});

MNX_ENUM_MAPPING(TimeSignatureDisplay, {
    { "common",         TimeSignatureDisplay::Common },
    { "cut",            TimeSignatureDisplay::Cut }
});

MNX_ENUM_MAPPING(TupletDisplaySetting, {
    { "inner",      TupletDisplaySetting::Inner },
    { "noNumber",   TupletDisplaySetting::NoNumber },
    { "both",       TupletDisplaySetting::Both }
});

#endif // DOXYGEN_SHOULD_IGNORE_THIS

} // namespace mnx
