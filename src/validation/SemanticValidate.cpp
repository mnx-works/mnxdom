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
#include <memory>
#include <unordered_map>
#include <set>
#include <map>
#include <string>
#include <vector>

#include "mnxdom.h"
#include "music_theory/music_theory.hpp"
#include "util/MusicTheoryConversions.h"

namespace mnx {
namespace validation {

#ifndef DOXYGEN_SHOULD_IGNORE_THIS

namespace {
struct ArpeggioSpanEndpoints
{
    sequence::Note startNote;
    sequence::Note endNote;
    sequence::Event startEvent;
    sequence::Event endEvent;
    Sequence startSequence;
    Sequence endSequence;
    part::Measure startMeasure;
    part::Measure endMeasure;
    Part startPart;
    Part endPart;
    util::EntityMap::MappedPosition startPosition;
    util::EntityMap::MappedPosition endPosition;
};

[[nodiscard]] bool sameRhythmicPosition(const util::EntityMap::MappedPosition& lhs,
                                        const util::EntityMap::MappedPosition& rhs,
                                        bool requireGraceIndexMatch)
{
    if (lhs.fraction != rhs.fraction) {
        return false;
    }
    if (!requireGraceIndexMatch) {
        return true;
    }
    return lhs.graceIndex == rhs.graceIndex;
}

[[nodiscard]] bool sameRhythmicPosition(const RhythmicPosition& lhs,
                                        const util::EntityMap::MappedPosition& rhs,
                                        bool requireGraceIndexMatch)
{
    if (lhs.fraction() != rhs.fraction) {
        return false;
    }
    if (!requireGraceIndexMatch || !lhs.graceIndex()) {
        return true;
    }
    return lhs.graceIndex() == rhs.graceIndex;
}

[[nodiscard]] int calcSoundedPitchSemitones(const sequence::Note& note)
{
    const auto pitch = note.pitch();
    return music_theory::calc12EdoHalfstepsInInterval(
        music_theory::calcDisplacement(util::toMusicTheoryPitch(pitch)),
        pitch.alter()
    );
}

} // namespace

class SemanticValidator
{
public:
    SemanticValidator(const Document& doc) : document(doc) {}

    SemanticValidationResult result;
    Document document;

    void validateGlobal();
    void validateParts();
    void validateLayouts();
    void validateScores();

    void addError(const std::string& message, const Base& location)
    {
        result.errors.emplace_back(location.pointer(), location.ref(), message);
    }

private:
    void validateSequenceContent(const mnx::sequence::SequenceContent& contentArray, const Base& location,
        FractionValue expectedDuration, bool allowEventsOnly = false, bool requireExactDuration = false, FractionValue* actualElapsedOut = nullptr);
    std::optional<ArpeggioSpanEndpoints> resolveArpeggioSpanEndpoints(const part::ArpeggioBase& arpeggioBase);
    std::optional<ArpeggioSpanEndpoints> validateArpeggioBase(const mnx::part::Measure& measure, const part::ArpeggioBase& arpeggioBase, std::string_view objectName);
    void validateArpeggios(const mnx::part::Measure& measure, const mnx::Array<mnx::part::Arpeggio>& arpeggios);
    void validateDynamics(const mnx::part::DynamicGroupArray& dynamics);
    void validateNonArpeggios(const mnx::part::Measure& measure, const mnx::Array<mnx::part::NonArpeggio>& nonArpeggios);
    void validateBeams(const mnx::Array<mnx::part::Beam>& beams, unsigned depth);
    void validateMeasureRepeats(const mnx::Part& part, const mnx::Array<mnx::part::Measure>& measures);
    void validateOttavas(const mnx::part::Measure& measure, const mnx::Array<mnx::part::Ottava>& ottavas);

    template <typename NoteType>
    void validateTies(const mnx::Array<mnx::sequence::Tie>& ties, const NoteType& note);

    template <typename T, typename KeyType>
    std::optional<T> tryGetValue(const KeyType& key, const Base& errorLocation);
};

template <typename T, typename KeyType>
std::optional<T> SemanticValidator::tryGetValue(const KeyType& key, const Base& errorLocation)
{
    try {
        return document.getEntityMap().get<T>(key, errorLocation);
    } catch (const util::mapping_error&) {
        // already reported error, so fall through
    }
    return std::nullopt;
}

std::optional<ArpeggioSpanEndpoints> SemanticValidator::resolveArpeggioSpanEndpoints(const part::ArpeggioBase& arpeggioBase)
{
    auto startNote = tryGetValue<sequence::Note>(arpeggioBase.span().start(), arpeggioBase);
    auto endNote = tryGetValue<sequence::Note>(arpeggioBase.span().end(), arpeggioBase);
    if (!startNote || !endNote) {
        return std::nullopt;
    }

    auto startEvent = startNote->parent<Array<sequence::Note>>().template parent<sequence::Event>();
    auto endEvent = endNote->parent<Array<sequence::Note>>().template parent<sequence::Event>();
    auto startSequence = startNote->getEnclosingElement<Sequence>();
    auto endSequence = endNote->getEnclosingElement<Sequence>();
    auto startMeasure = startNote->getEnclosingElement<part::Measure>();
    auto endMeasure = endNote->getEnclosingElement<part::Measure>();
    auto startPart = startNote->getEnclosingElement<Part>();
    auto endPart = endNote->getEnclosingElement<Part>();

    if (!startSequence || !endSequence || !startMeasure || !endMeasure || !startPart || !endPart) {
        addError("Unable to resolve arpeggio span endpoints.", arpeggioBase);
        return std::nullopt;
    }

    auto startPosition = document.getEntityMap().tryGetEventPosition(startEvent);
    auto endPosition = document.getEntityMap().tryGetEventPosition(endEvent);
    if (!startPosition || !endPosition) {
        addError("Unable to resolve arpeggio span endpoints.", arpeggioBase);
        return std::nullopt;
    }

    return ArpeggioSpanEndpoints{
        startNote.value(),
        endNote.value(),
        startEvent,
        endEvent,
        startSequence.value(),
        endSequence.value(),
        startMeasure.value(),
        endMeasure.value(),
        startPart.value(),
        endPart.value(),
        startPosition.value(),
        endPosition.value()
    };
}

std::optional<ArpeggioSpanEndpoints> SemanticValidator::validateArpeggioBase(const mnx::part::Measure& measure, const part::ArpeggioBase& arpeggioBase, std::string_view objectName)
{
    // start and end notes may be equal.
    
    auto endpoints = resolveArpeggioSpanEndpoints(arpeggioBase);
    if (!endpoints) {
        return std::nullopt;
    }

    if (endpoints->startPart.calcArrayIndex() != endpoints->endPart.calcArrayIndex()) {
        addError(std::string(objectName) + " span notes must be in the same part.", arpeggioBase);
    }
    if (endpoints->startMeasure.calcArrayIndex() != endpoints->endMeasure.calcArrayIndex()) {
        addError(std::string(objectName) + " span notes must be in the same bar.", arpeggioBase);
    }
    if (endpoints->startMeasure.calcArrayIndex() != measure.calcArrayIndex()
        || endpoints->endMeasure.calcArrayIndex() != measure.calcArrayIndex()) {
        addError(std::string(objectName) + " span notes must be in the same bar as the " + std::string(objectName) + ".", arpeggioBase);
    }
    const bool sameSequence = endpoints->startSequence.pointer() == endpoints->endSequence.pointer();
    if (!sameRhythmicPosition(endpoints->startPosition, endpoints->endPosition, sameSequence)) {
        addError(std::string(objectName) + " span notes must be in the same rhythmic position.", arpeggioBase);
        return std::nullopt;
    }
    if (!sameRhythmicPosition(arpeggioBase.position(), endpoints->startPosition, sameSequence)) {
        addError(std::string(objectName) + " position must match the span notes' rhythmic position.", arpeggioBase);
    }

    return endpoints;
}

void SemanticValidator::validateGlobal()
{
    auto global = document.global();

    result.lyricLines.clear();
    if (global.lyrics().has_value()) {
        const auto lyricsGlobal = global.lyrics().value();
        const auto lineOrder = lyricsGlobal.lineOrder();
        const auto lineMetadata = lyricsGlobal.lineMetadata();
        // If both are present, validate that they match
        if (lineOrder) {
            size_t x = 0;
            for (const auto& lineId : lineOrder.value()) {
                if (auto rslt = result.lyricLines.emplace(lineId, lineOrder.value()[x].pointer()); !rslt.second) {
                    addError("ID \"" + lineId + "\" already exists at " + rslt.first->second.to_string(), lineOrder.value()[x]);
                }
                x++;
            }
            if (lineMetadata) {
                if (result.lyricLines.size() != lineMetadata.value().size()) {
                    addError("Size of line metadata does not match size of line order.", lineMetadata.value());
                }
                for (const auto& [lineId, instance] : lineMetadata.value()) {
                    if (result.lyricLines.find(lineId) == result.lyricLines.end()) {
                        addError("ID \"" + lineId + "\" not found in ID mapping", instance);
                    }
                }
            }
        } else if (lineMetadata) {
            for (const auto& [lineId, instance] : lineMetadata.value()) {
                if (auto rslt = result.lyricLines.emplace(lineId, instance.pointer()); !rslt.second) {
                    addError("ID \"" + lineId + "\" already exists at " + rslt.first->second.to_string(), instance);
                }
            }
        }
    }

    if (const auto sounds = global.sounds()) {
        for (const auto& [soundId, sound] : sounds.value()) {
            if (auto midiNumber = sound.midiNumber()) {
                if (static_cast<int>(midiNumber.value()) < 0 || midiNumber.value() > 127) {
                    addError("Invalid midi number: " + std::to_string(midiNumber.value()), sound);
                }
            }
        }
    }
}

template <typename NoteType>
void SemanticValidator::validateTies(const mnx::Array<mnx::sequence::Tie>& ties, const NoteType& note)
{
    static_assert(std::is_base_of_v<mnx::sequence::NoteBase, NoteType>, "NoteType must be derived from NoteBase.");

    std::optional<mnx::Part> enclosingPart = note.template getEnclosingElement<Part>();
    if (!enclosingPart) {
        addError("The tied note has no part.", note);
        return;
    }

    for (const auto& tie : ties) {
        if (auto target = tie.target()) {
            if (tie.lv()) {
                addError("Tie has both a target and is an lv tie.", tie);
            }
            if (const auto targetNote = tryGetValue<NoteType>(target.value(), tie)) {
                if (targetNote->template getEnclosingElement<Part>()->calcArrayIndex() != enclosingPart->calcArrayIndex()) {
                    addError("Tie points to a note in a different part.", tie);
                }
                if constexpr (std::is_same_v<NoteType, mnx::sequence::Note>) {
                    if (!note.pitch().isSamePitch(targetNote->pitch())) {
                        addError("Tie points to a note with a different pitch.", tie);
                    }
                } else if constexpr (std::is_same_v<NoteType, mnx::sequence::KitNote>) {
                    if (targetNote->kitComponent() != note.kitComponent()) {
                        addError("Tie points to a different kit component.", tie);
                    }
                }
            } else {
                addError("Tie points to non-existent target " + target.value(), tie);
            }
        } else {
            if (!tie.lv()) {
                addError("Tie has neither a target nor is it an lv tie.", tie);
            } else if (tie.targetType()) {
                addError("Tie is an lv tie but also has targetType.", tie);
            }
        }
    }
}

void SemanticValidator::validateSequenceContent(const mnx::sequence::SequenceContent& contentArray, const Base& location,
    FractionValue expectedDuration, bool allowEventsOnly, bool requireExactDuration, FractionValue* actualElapsedOut)
{
    if (actualElapsedOut) {
        *actualElapsedOut = 0;
    }
    auto part = contentArray.getEnclosingElement<mnx::Part>();
    if (!part.has_value()) {
        addError("Sequence content array has no part.", contentArray);
        return;
    }
    auto elapsedTime = FractionValue(0);
    for (const auto content : contentArray) {
        if (content.type() == mnx::sequence::Event::ContentTypeValue) {
            auto event = content.get<mnx::sequence::Event>();
            elapsedTime += event.duration();
            if (event.rest().has_value()) {
                if (event.notes() && !event.notes().value().empty()) {
                    addError("Event \"" + event.id_or("<no-id>") + "\" is a rest but also has notes.", event);
                }
            } else {
                const bool notesExist = event.notes() && !event.notes().value().empty();
                const bool kitNotesExist = event.kitNotes() && !event.kitNotes().value().empty();
                if (!notesExist && !kitNotesExist) {
                    addError("Event \"" + event.id_or("<no-id>") + "\" is neither a rest nor has notes.", event);
                }
            }
            if (const auto notes = event.notes()) {
                for (const auto note : notes.value()) {
                    const int noteAlter = note.pitch().alter();
                    if (std::abs(noteAlter) > 3) {
                        addError("Note \"" + note.id_or("<no-id>") + "\" has alteration value " + std::to_string(noteAlter) + ". MNX files are limited to +/-3.", note);
                    }
                    if (const auto ties = note.ties()) {
                        validateTies(ties.value(), note);
                    }
                }
            }
            if (const auto kitNotes = event.kitNotes()) {
                for (const auto kitNote : kitNotes.value()) {
                    if (!part->kit() || !part->kit()->contains(kitNote.kitComponent())) {
                        addError("Kit note has kit element " + kitNote.kitComponent() + " that is not defined in the part's kit.", kitNote);
                    }
                    if (const auto ties = kitNote.ties()) {
                        validateTies(ties.value(), kitNote);
                    }
                }
            }
            if (!result.lyricLines.empty()) { // only check lyric lines if the line ids were provided in global.lyrics()
                if (auto lyrics = event.lyrics()) {
                    if (auto lines = lyrics.value().lines()) {
                        for (const auto& line : lines.value()) {
                            if (result.lyricLines.find(line.first) == result.lyricLines.end()) {
                                addError("ID \"" + line.first + "\" not found in ID mapping", line.second);
                            }
                        }
                    }
                }
            }
            if (const auto slurs = event.slurs()) {
                for (const auto& slur : slurs.value()) {
                    const auto targetEvent = tryGetValue<mnx::sequence::Event>(slur.target(), slur);
                    if (slur.endNote()) {
                        bool foundNote = false;
                        if (targetEvent) {
                            foundNote = targetEvent.value().findNote(slur.endNote().value()).has_value();
                        }
                        if (!foundNote) {
                            addError("Slur contains end note \"" + slur.endNote().value() + "\" that does not exist in target.", slur);
                        }
                    }
                    if (slur.startNote()) {
                        if (!event.findNote(slur.startNote().value())) {
                            addError("Slur contains start note \"" + slur.endNote().value()
                                + "\" that does not exist in the containing event.", slur);
                        }
                    }
                }
            }
        } else if (content.type() == mnx::sequence::Grace::ContentTypeValue) {
            if (allowEventsOnly) {
                addError("Content array contains grace note object, which is not permitted for type " + content.type(), content);
            }
            auto grace = content.get<mnx::sequence::Grace>();
            validateSequenceContent(grace.content(), grace, 0, /*allowEventsOnly*/true); // true => error on content other than events
        } else if (content.type() == mnx::sequence::Tuplet::ContentTypeValue) {
            if (allowEventsOnly) {
                addError("Content array contains tuplet object, which is not permitted for type " + content.type(), content);
            }
            auto tuplet = content.get<mnx::sequence::Tuplet>();
            FractionValue tupletElapsed;
            validateSequenceContent(tuplet.content(), tuplet, tuplet.inner(), /*allowEventsOnly*/false, /*requireExactDuration*/true, &tupletElapsed);
            if (tuplet.inner() != 0) {
                elapsedTime += tupletElapsed * tuplet.ratio();
            } else {
                addError("Encountered tuplet with zero length inner value.", tuplet);
            }
        } else if (content.type() == mnx::sequence::MultiNoteTremolo::ContentTypeValue) {
            if (allowEventsOnly) {
                addError("Content array contains multi-note tremolo object, which is not permitted for type " + content.type(), content);
            }
            auto tremolo = content.get<mnx::sequence::MultiNoteTremolo>();
            const auto multiple = tremolo.outer().multiple();
            if (multiple < 2) {
                addError("Multi-note tremolo has " + std::to_string(tremolo.outer().multiple()) + " events.", tremolo);
            }
            if (multiple > 0) {
                FractionValue tremoloElapsed;
                const FractionValue expectedInner = tremolo.outer() * multiple;
                validateSequenceContent(tremolo.content(), tremolo, expectedInner, /*allowEventsOnly*/true, /*requireExactDuration*/true, &tremoloElapsed);
            }
            elapsedTime += tremolo.outer();
        } else if (content.type() == mnx::sequence::Space::ContentTypeValue) {
            if (allowEventsOnly) {
                addError("Content array contains space object, which is not permitted for type " + content.type(), content);
            }
            auto space = content.get<mnx::sequence::Space>();
            elapsedTime += space.duration();
        } else {
            addError("Unknown content type \"" + content.type() + "\" encounterd in layout.", content);
        }
    }
    if (expectedDuration != 0) {
        if (elapsedTime > expectedDuration) {
            addError("Entries in content array add up to more than the expected value.", location);
        } else if (elapsedTime < expectedDuration && requireExactDuration) {
            addError("Entries in content array add up to less than the expected value.", location);
        }
    }
    if (actualElapsedOut) {
        *actualElapsedOut = elapsedTime;
    }
}

void SemanticValidator::validateBeams(const mnx::Array<mnx::part::Beam>& beams, unsigned depth)
{
    std::set<std::pair<unsigned, std::string>> ids;
    for (const auto beam : beams) {
        if (beam.events().empty()) {
            addError("Beam contains no events.", beam);
            continue;
        }
        const auto beamMeasure = beam.getEnclosingElement<mnx::part::Measure>();
        if (!beamMeasure) {
            addError("Unable to find enclosing measure for beam.", beam);
            continue;
        }
        size_t currentMeasureIndex = beamMeasure.value().calcArrayIndex();
        bool requireMeasuresEqualOnFirst = depth == 1;
        std::optional<bool> isGraceBeam;
        std::optional<std::string> voice;
        FractionValue currentSequenceTime = 0;
        for (const auto id : beam.events()) {
            if (ids.find(std::make_pair(depth, id)) != ids.end()) {
                addError("Event \"" + id + "\" is duplicated in beam at depth " + std::to_string(depth) + ".", beam);
                continue;
            }
            ids.emplace(std::make_pair(depth, id));
            if (const auto event = tryGetValue<mnx::sequence::Event>(id, beam)) {
                size_t nextMeasureIndex = currentMeasureIndex;
                if (const auto eventMeasure = event.value().getEnclosingElement<mnx::part::Measure>()) {
                    nextMeasureIndex = eventMeasure.value().calcArrayIndex();
                } else {
                    addError("Unable to find enclosing measure for event.", event.value());
                }
                const auto startTime = event.value().calcStartTime();
                if (requireMeasuresEqualOnFirst && nextMeasureIndex != currentMeasureIndex) {
                    addError("First event in beam is not in the same measure as the beam.", beam);
                } else if (nextMeasureIndex < currentMeasureIndex) {
                    addError("Beam measures are out of sequence", beam);
                } else if (nextMeasureIndex > currentMeasureIndex) {
                    currentSequenceTime = startTime;
                }
                requireMeasuresEqualOnFirst = false;
                currentMeasureIndex = nextMeasureIndex;
                const auto nextSequenceTime = startTime;
                if (nextSequenceTime < currentSequenceTime) {
                    addError("Beam events are out of sequence.", beam);
                }
                currentSequenceTime = nextSequenceTime;
                if (event.value().isTremolo()) {
                    addError("Beam containing event \"" + id + "\" is actually a multi-note tremolo and should not be a beam.", beam);
                    continue;
                }
                if (isGraceBeam.has_value()) {
                    if (isGraceBeam.value() != event.value().isGrace()) {
                        addError("Event \"" + id + "\" attempts to beam a grace note to a non grace note.", beam);
                    }
                } else {
                    isGraceBeam = event.value().isGrace();
                }
                if (voice.has_value()) {
                    if(voice.value() != event.value().getSequence().voice_or("")) {
                        addError("Event \"" + id + "\" attempts to beam events from different voices together.", beam);
                    }
                } else {
                    voice = event.value().getSequence().voice_or("");
                }
                const auto noteValue = event.value().duration();
                if (depth > noteValue.calcNumberOfFlags()) {
                    addError("Event \"" + id + "\" cannot have " + std::to_string(depth) + " beams", beam);
                }
            }
        }
        if (beam.direction() != BeamHookDirection::Auto && beam.events().size() != 1) {
            addError("Beam specifies a hook direction but contains more than one event.", beam);
        }
        if (auto singleBeam = beam.beams()) {
            validateBeams(singleBeam.value(), depth + 1);
        }
    }
}

void SemanticValidator::validateArpeggios(const mnx::part::Measure& measure, const mnx::Array<mnx::part::Arpeggio>& arpeggios)
{
    for (const auto arpeggio : arpeggios) {
        auto endpoints = validateArpeggioBase(measure, arpeggio, "Arpeggio");
        if (!endpoints) {
            continue;
        }

        const int startPitch = calcSoundedPitchSemitones(endpoints->startNote);
        const int endPitch = calcSoundedPitchSemitones(endpoints->endNote);
        if (arpeggio.direction() == MarkingUpDownAuto::Up && startPitch > endPitch) {
            addError("Arpeggio with direction \"up\" must start on a lower sounded pitch than it ends on.", arpeggio);
        } else if (arpeggio.direction() == MarkingUpDownAuto::Down && startPitch < endPitch) {
            addError("Arpeggio with direction \"down\" must start on a higher sounded pitch than it ends on.", arpeggio);
        }
    }
}

void SemanticValidator::validateDynamics(const mnx::part::DynamicGroupArray& dynamics)
{
    for (const auto dynamic : dynamics) {
        const auto dynamicType = dynamic.type();
        const auto dynamicJson = json::parse(dynamic.dump());
        const auto requireField = [&](bool present, std::string_view fieldName) {
            if (!present) {
                addError("Dynamic of type \"" + dynamicType + "\" requires field \"" + std::string(fieldName) + "\".", dynamic);
            }
        };

        if (dynamicType == mnx::part::DynamicImmediate::ContentTypeValue) {
            requireField(dynamicJson.contains("value"), "value");
        } else if (dynamicType == mnx::part::DynamicGradual::ContentTypeValue) {
            requireField(dynamicJson.contains("end"), "end");
            requireField(dynamicJson.contains("wedgeType"), "wedgeType");
        } else if (dynamicType == mnx::part::DynamicRelative::ContentTypeValue) {
            requireField(dynamicJson.contains("relativeValue"), "relativeValue");
        } else if (dynamicType == mnx::part::DynamicAccent::ContentTypeValue) {
            requireField(dynamicJson.contains("value"), "value");
        }

        if (!dynamic.calcHasImmediateText() && dynamicType != mnx::part::DynamicGradual::ContentTypeValue) {
            addError("Dynamic of type \"" + dynamicType + "\" has none of value, attackValue, prefix, or suffix.", dynamic);
        }
    }
}

void SemanticValidator::validateNonArpeggios(const mnx::part::Measure& measure, const mnx::Array<mnx::part::NonArpeggio>& nonArpeggios)
{
    for (const auto nonArpeggio : nonArpeggios) {
        auto endpoints = validateArpeggioBase(measure, nonArpeggio, "Non-arpeggio");
        if (!endpoints) {
            continue;
        }

        const int startPitch = calcSoundedPitchSemitones(endpoints->startNote);
        const int endPitch = calcSoundedPitchSemitones(endpoints->endNote);
        if (startPitch >= endPitch) {
            addError("Non-arpeggio span start note must be lower in sounded pitch than the end note.", nonArpeggio);
        }
    }
}

void SemanticValidator::validateMeasureRepeats(const mnx::Part& part, const mnx::Array<mnx::part::Measure>& measures)
{
    // A measure repeat of N measures occupies the N measures beginning with the one that carries it,
    // so only that first measure may declare it. The measures it covers are continuations.
    size_t blockStart = 0;      // index of the measure declaring the enclosing multi-measure block
    size_t blockEnd = 0;        // one past the last measure that block covers
    for (size_t index = 0; index < measures.size(); index++) {
        const auto measureRepeat = measures[index].measureRepeat();
        if (!measureRepeat) {
            continue;
        }
        if (index < blockEnd) {
            addError("Measure repeat in part " + part.id_or("<no-id>") + " occurs at measure index "
                + std::to_string(index) + ", which is already covered by the measure repeat at index "
                + std::to_string(blockStart) + ". Only the first measure of a repeated group may declare one.",
                measureRepeat.value());
            continue;
        }
        const int number = measureRepeat.value().number();
        if (number < 1) {
            // Enforcing positivity belongs in the schema, which does not yet bound any integer type.
            // Treat a non-positive count as covering nothing rather than duplicating that check here.
            continue;
        }
        blockStart = index;
        blockEnd = index + static_cast<size_t>(number);
        if (static_cast<size_t>(number) > index) {
            addError("Measure repeat in part " + part.id_or("<no-id>") + " at measure index "
                + std::to_string(index) + " has a repeat count of " + std::to_string(number)
                + ", which reaches back before the start of the part.", measureRepeat.value());
        }
        if (blockEnd > measures.size()) {
            addError("Measure repeat in part " + part.id_or("<no-id>") + " at measure index "
                + std::to_string(index) + " spans " + std::to_string(number)
                + " measures, which extends past the end of the part (" + std::to_string(measures.size())
                + " measures).", measureRepeat.value());
        }
    }
}

void SemanticValidator::validateOttavas(const mnx::part::Measure& measure, const mnx::Array<mnx::part::Ottava>& ottavas)
{    
    for (const auto ottava : ottavas) {
        if (auto endMeasure = tryGetValue<mnx::global::Measure>(ottava.end().measure(), ottava)) {
            size_t thisMeasureIndex = measure.calcArrayIndex();
            size_t endMeasureIndex = endMeasure.value().calcArrayIndex();
            if (thisMeasureIndex > endMeasureIndex) {
                addError("Ottava ends before it begins", ottava);
            } else if (thisMeasureIndex == endMeasureIndex && ottava.position().fraction() > ottava.end().position().fraction()) {
                addError("Ottava ends before it begins (in the same measure)", ottava);
            }
        }
    }
}

void SemanticValidator::validateParts()
{
    for (const auto part : document.parts()) {
        size_t x = part.calcArrayIndex();
        std::string partName = "[" + std::to_string(x) + "]";
        const auto measures = part.measures();
        size_t numPartMeasures = measures.size();
        size_t numGlobalMeasures = document.global().measures().size();
        if (numPartMeasures != numGlobalMeasures) {
            addError("Part" + partName + " contains a different number of measures (" + std::to_string(numPartMeasures)
                + ") than are defined globally (" + std::to_string(numGlobalMeasures) + ")", part);
            if (numGlobalMeasures < numPartMeasures) {
                return; // cannot continue because can't get current time for part measures greater than global measures
            }
        }
        if (part.staves() < 1) {
            addError("Part" + partName + " contains no staves (" + std::to_string(part.staves()) + ")", part);
        }
        if (auto kit = part.kit()) {
            auto sounds = document.global().sounds();
            for (const auto& [kitElementId, kitElement] : kit.value()) {
                if (kitElement.sound()) {
                    if (!sounds || !sounds->contains(kitElement.sound().value())) {
                        addError("Sound ID " + kitElement.sound().value() + " is not defined in global.sounds.", kitElement);
                    }
                }
            }
        }
        const int staffCount = part.staves();
        if (numPartMeasures > 0) {
            const auto firstMeasure = measures[0];
            std::vector<bool> staffHasInitialClef(static_cast<size_t>(staffCount) + 1, false);
            if (auto clefs = firstMeasure.clefs()) {
                for (const auto clef : clefs.value()) {
                    const int staffNumber = clef.staff();
                    if (staffNumber < 1 || staffNumber > staffCount) {
                        continue;
                    }
                    bool atMeasureStart = true;
                    if (const auto position = clef.position()) {
                        const FractionValue offset = position->fraction();
                        if (offset != FractionValue(0, 1)) {
                            atMeasureStart = false;
                        }
                    }
                    if (atMeasureStart) {
                        staffHasInitialClef[static_cast<size_t>(staffNumber)] = true;
                    }
                }
            }
            for (int staffNumber = 1; staffNumber <= staffCount; ++staffNumber) {
                /// @todo eventually we should not skip part.kit, but mnx currently has no percussion clef.
                if (!part.kit() && !staffHasInitialClef[static_cast<size_t>(staffNumber)]) {
                    addError("Missing clef at the beginning of staff " + std::to_string(staffNumber) +
                                 " in part " + part.id_or("<no-id>") + " (first measure).",
                             firstMeasure);
                }
            }
        }
        validateMeasureRepeats(part, measures);
        // first pass: validateSequenceContent creates the eventList and the noteList
        for (const auto measure : measures) {
            std::unordered_map<std::string, std::string> measureVoices;
            if (auto clefs = measure.clefs()) {
                for (const auto clef : clefs.value()) {
                    const int staffNumber = clef.staff();
                    if (staffNumber < 1 || staffNumber > staffCount) {
                        addError("Clef references non-existent staff " + std::to_string(staffNumber) +
                                     " in part " + part.id_or("<no-id>") + ".",
                                 clef);
                    }
                }
            }
            if (auto staffConfigs = measure.staffConfigs()) {
                for (const auto staffConfig : staffConfigs.value()) {
                    const int staffNumber = staffConfig.staff();
                    if (staffNumber < 1 || staffNumber > staffCount) {
                        addError("Staff config references non-existent staff " + std::to_string(staffNumber) +
                                     " in part " + part.id_or("<no-id>") + ".",
                                 staffConfig);
                    }
                }
            }
            auto measureTime = [&]() -> FractionValue {
                if (auto time = measure.calcCurrentTime()) {
                    return time.value();
                }
                return FractionValue(4, 4);
            }();
            for (const auto sequence : measure.sequences()) {
                if (sequence.staff() > part.staves()) {
                    addError("Sequence references non-existent part staff for part " + part.id_or("<no-id>") + ".", sequence);
                    continue;
                }
                if (sequence.fullMeasure() && !sequence.content().empty()) {
                    addError("Sequence \"" + sequence.id_or("<no-id>") + "\" has full measure rest but content is not empty.", sequence);
                }
                if (const auto voice = sequence.voice()) {
                    if (voice->empty()) {
                        addError("Sequence \"" + sequence.id_or("<no-id>") + "\" has empty voice identifier.", sequence);
                    } else if (auto insert = measureVoices.emplace(voice.value(), sequence.pointer().to_string()); !insert.second) {
                        addError("Voice \"" + voice.value() + "\" already exists at " + insert.first->second + ".", sequence);
                    }
                }
                validateSequenceContent(sequence.content(), sequence, measureTime);
            }
        }
        // second pass: validate other items that need a complete list of events and notes
        for (const auto measure : measures) {
            if (auto beams = measure.beams()) {
                validateBeams(beams.value(), 1);
            }
            if (auto dynamics = measure.dynamics()) {
                validateDynamics(dynamics.value());
            }
            if (auto arpeggios = measure.arpeggios()) {
                validateArpeggios(measure, arpeggios.value());
            }
            if (auto nonArpeggios = measure.nonArpeggios()) {
                validateNonArpeggios(measure, nonArpeggios.value());
            }
            if (auto ottavas = measure.ottavas()) {
                validateOttavas(measure, ottavas.value());
            }
        }
    }
}

void SemanticValidator::validateLayouts()
{
    if (auto layouts = document.layouts()) {  // layouts are *not* required in MNX
        for (const auto layout : layouts.value()) {
            auto validateContent = [&](auto&& self, const mnx::ContentArray<mnx::layout::LayoutContentObject>& content) -> void {
                for (const auto element : content) {
                    if (element.type() == mnx::layout::Group::ContentTypeValue) {
                        auto group = element.get<mnx::layout::Group>();
                        if (group.content().empty()) {
                            addError("Layout group contains no content.", group);
                        }
                        self(self, group.content());
                    } else if (element.type() == mnx::layout::Staff::ContentTypeValue) {
                        auto staff = element.get<mnx::layout::Staff>();
                        bool hasEmptyPartId = false;
                        /// @todo validate "labelref"?
                        for (const auto source : staff.sources()) {
                            if (source.part().empty()) {
                                addError("Layout staff \"" + staff.id_or("<no-id>") + "\" has empty part id in source.", source);
                                hasEmptyPartId = true;
                                continue;
                            }
                            if (const auto part = tryGetValue<mnx::Part>(source.part(), source)) {
                                int staffNum = source.staff();
                                int numStaves = part.value().staves();
                                if (staffNum > numStaves || staffNum < 1) {
                                    addError("Layout \"" + layout.id_or("<no-id>") + "\" has invalid staff number ("
                                        + std::to_string(staffNum) + ") for part " + source.part(), source);
                                }
                            }
                            /// @todo validate "labelref"?
                            /// @todo validate "voice"?
                        }
                        if (!hasEmptyPartId && !util::analyzeLayoutStaffVoices(staff)) {
                            addError("Layout staff \"" + staff.id_or("<no-id>") + "\" has one or more part voices specified multiple times.", staff);
                        }
                    } else {
                        addError("Unknown content type \"" + element.type() + "\" encounterd in layout.", element);
                    }
                }
            };
            validateContent(validateContent, layout.content());
        }
    }
}

void SemanticValidator::validateScores()
{
    if (const auto scores = document.scores()) {  // scores are *not* required in MNX
        for (const auto score : scores.value()) {
            if (const auto layout = score.layout()) {
                tryGetValue<mnx::Layout>(layout.value(), score); // adds error if index not found.
            }
            if (const auto multimeasureRests = score.multimeasureRests()) {
                for (const auto mmRest : multimeasureRests.value()) {
                    if (const auto measure = tryGetValue<mnx::global::Measure>(mmRest.start(), mmRest)) {
                        size_t index = measure.value().calcArrayIndex();
                        if (index + mmRest.duration() > document.global().measures().size()) {
                            addError("Multimeasure rest at measure \"" + mmRest.start() + "\" in score \""
                                + score.name() + "\" spans non-existent measures", mmRest);
                        }
                    }
                }
            }
            std::optional<size_t> lastSystemMeasure;
            bool isFirstSystem = true;
            bool hasSystem = false;
            bool skipScore = false;
            if (auto pages = score.pages()) {
                for (const auto page : pages.value()) {
                    if (const auto layout = page.layout()) {
                        tryGetValue<mnx::Layout>(layout.value(), page); // adds error if index not found.
                    }
                    for (const auto system : page.systems()) {
                        hasSystem = true;
                        if (const auto layout = system.layout()) {
                            tryGetValue<mnx::Layout>(layout.value(), system); // adds error if index not found.
                        }
                        std::optional<size_t> currentSystemMeasureIndex;
                        if (const auto currentSystemMeasure = tryGetValue<mnx::global::Measure>(system.measure(), system)) {
                            currentSystemMeasureIndex = currentSystemMeasure.value().calcArrayIndex();
                            if (isFirstSystem && currentSystemMeasureIndex.value() > 0) {
                                addError("The first system in score \"" + score.name()
                                    + "\" starts after the first measure", system);
                            }
                        } else {
                            addError("Score \"" + score.name()
                                + "\" references missing measure \"" + system.measure() + "\"", system);
                            skipScore = true;
                            break;
                        }
                        isFirstSystem = false;
                        if (lastSystemMeasure && currentSystemMeasureIndex && currentSystemMeasureIndex.value() <= lastSystemMeasure) {
                            std::string msg = currentSystemMeasureIndex.value() < lastSystemMeasure.value()
                                ? "starts before"
                                : "starts on the same measure as";
                            addError("Score \"" + score.name() + "\" contains system that " + msg + " previous system", system);
                        }
                        lastSystemMeasure = currentSystemMeasureIndex;
                        if (const auto layoutChanges = system.layoutChanges()) {
                            for (const auto layoutChange : layoutChanges.value()) {
                                tryGetValue<mnx::Layout>(layoutChange.layout(), layoutChange); // adds error if index not found.
                                if (const auto globalMeasure = tryGetValue<mnx::global::Measure>(layoutChange.location().measure(), layoutChange)) { // adds error if index not found.
                                    if (layoutChange.location().position().fraction() >= globalMeasure->calcCurrentTime()) {
                                        addError("Layout \"" + layoutChange.id_or("<no-id>") + "\" starts at or past the end of the measure.", layoutChange);
                                    }
                                }
                            }
                        }
                    }
                    if (skipScore) {
                        break;
                    }
                }
                if (!skipScore && !hasSystem) {
                    addError("Score \"" + score.name() + "\" has pages but no systems", score);
                }
            }
        }
    }
}

#endif // DOXYGEN_SHOULD_IGNORE_THIS

SemanticValidationResult semanticValidate(const Document& document)
{
    SemanticValidator validator(document);
    validator.document.buildEntityMap({}, [&](const std::string& message, const Base& location) {
        validator.addError(message, location);
    });

    // these calls are order-dependent
    validator.validateGlobal();
    validator.validateParts();
    validator.validateLayouts();
    validator.validateScores();

    return validator.result;
}

namespace {
// allowed root-level items defined here
constexpr std::array<std::string_view, 8> allowedItems = {
    "global",
    "layouts",
    "mnx",
    "parts",
    "scores",
    "_c",
    "_x",
    "id"
};

constexpr bool allowedItemsContains(std::string_view key)
{
    for (auto v : allowedItems) {
        if (v == key) {
            return true;
        }
    }
    return false;
}
} // namespace

bool hasValidDocumentRoot(const Document& document)
{
    // required items found here
    bool foundGlobal = false;
    bool foundParts = false;
    bool foundMnx = false;

    const auto root = document.root();
    for (const auto& [key, value] : root->items()) {
        if (!allowedItemsContains(key)) {
            return false;
        }
        if (key == "global") {
            foundGlobal = true;
        }
        if (key == "parts") {
            foundParts = true;
        }
        if (key == "mnx") {
            const auto mnx = root->at("mnx");
            if (!mnx["version"].empty()) {
                foundMnx = true;
            }
        }
    }

    return foundGlobal && foundParts && foundMnx;
}

} // namespace validation
} // namespace mnx
