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

#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>

#include "../BaseTypes.h"
#include "../Global.h"
#include "../Layout.h"
#include "../Part.h"
#include "../Score.h"
#include "../Sequence.h"

namespace mnx::util {

namespace detail {
#ifndef DOXYGEN_SHOULD_IGNORE_THIS

template <typename T>
struct JsonSchemaTypeNames
{
    static constexpr std::array<std::string_view, 1> value{ T::JsonSchemaTypeName };
};

template <>
struct JsonSchemaTypeNames<sequence::NoteBase>
{
    static constexpr std::array<std::string_view, 2> value{
        sequence::Note::JsonSchemaTypeName,
        sequence::KitNote::JsonSchemaTypeName
    };
};

template <>
struct JsonSchemaTypeNames<sequence::SequenceContentObject>
{
    static constexpr std::array<std::string_view, 5> value{
        sequence::Event::JsonSchemaTypeName,
        sequence::Grace::JsonSchemaTypeName,
        sequence::Tuplet::JsonSchemaTypeName,
        sequence::Space::JsonSchemaTypeName,
        sequence::MultiNoteTremolo::JsonSchemaTypeName
    };
};

template <>
struct JsonSchemaTypeNames<layout::LayoutContentObject>
{
    static constexpr std::array<std::string_view, 2> value{
        layout::Staff::JsonSchemaTypeName,
        layout::Group::JsonSchemaTypeName
    };
};

template <>
struct JsonSchemaTypeNames<part::ArpeggioBase>
{
    static constexpr std::array<std::string_view, 2> value{
        part::Arpeggio::JsonSchemaTypeName,
        part::NonArpeggio::JsonSchemaTypeName
    };
};

template <typename T>
bool matchesTypeName(std::string_view typeName)
{
    for (const auto name : JsonSchemaTypeNames<T>::value) {
        if (name == typeName) {
            return true;
        }
    }
    return false;
}

template <typename T>
std::string typeNamesExpectationString()
{
    std::ostringstream oss;
    const auto& names = JsonSchemaTypeNames<T>::value;
    const auto count = names.size();
    if (count > 1) {
        oss << "expected one of ";
    } else {
        oss << "expected ";
    }
    for (size_t i = 0; i < count; ++i) {
        if (i > 0) {
            oss << ", ";
        }
        oss << "\"" << names[i] << "\"";
    }
    return oss.str();
}

#endif // DOXYGEN_SHOULD_IGNORE_THIS
} // namespace detail

/// @brief base class for mapping error exceptions
class mapping_error : public std::runtime_error
{
    using runtime_error::runtime_error;
};

/**
 * @class EntityMap
 * @brief Provides type-safe ID-based lookup for elements in an MNX document.
 *
 * Constructed from an mnx::Document, the EntityMap indexes every object in the document
 * that carries an `id`, classifying each one by the name of the MNX schema definition that
 * describes it. Supports typed lookup (via each DOM class's `JsonSchemaTypeName`) as well as
 * untyped lookup for ids on objects the DOM does not model or the schema cannot classify.
 */
class EntityMap
{
public:
    /// @brief Canonical position value used by EntityMap for measure-relative comparisons and caches.
    struct MappedPosition
    {
        std::string measureId;                  ///< the measure id of the measure
        FractionValue fraction;                 ///< the rhythmic position within the measure
        std::optional<unsigned> graceIndex;     ///< the grace index of the event, if any

        /// @brief Constructor function
        MappedPosition(const std::string& measureId, const FractionValue& fraction,
                       std::optional<unsigned> graceIndex = std::nullopt)
            : measureId(measureId), fraction(fraction), graceIndex(graceIndex)
        {
        }

        /// @brief Constructor function from @ref MeasureRhythmicPosition.
        MappedPosition(const MeasureRhythmicPosition& position)
            : measureId(position.measure()), fraction(position.position().fraction()), graceIndex(position.position().graceIndex())
        {
        }
    };

    /// @brief Where an id lives in the document and what kind of object carries it.
    struct IdEntry
    {
        json_pointer location;      ///< location of the object carrying the id
        std::string typeName;       ///< MNX schema `$defs` name of the object, or empty if it could not be classified
    };

    /// @brief Placeholder used in messages for an id whose object type could not be classified.
    static constexpr std::string_view UNKNOWN_TYPE_NAME = "<unknown>";

private:
    /// @brief Adds an id to the mapping. If there is no error handler, it throws @ref mapping_error if there is a duplicate id.
    /// @param id The id to add.
    /// @param location The location of the object that carries the id.
    /// @param typeName The schema `$defs` name of the object, or empty if unknown.
    /// @throws mapping_error if the id is a duplicate and there is no error handler.
    void add(const std::string& id, const json_pointer& location, std::string typeName)
    {
        auto result = m_objectMap.emplace(id, IdEntry{ location, std::move(typeName) });
        if (!result.second) {
            mapping_error err("ID " + formatKeyString(id) + " already exists for type \"" + displayTypeName(result.first->second.typeName)
                + "\" at " + result.first->second.location.to_string());
            if (m_errorHandler) {
                m_errorHandler.value()(err.what(), Object(root(), location));
            } else {
                throw err;
            }
        }
    }
    
    /// @brief Map an event's id to its beam
    /// @param eventId The id of the event to map.
    /// @param beam The beam that includes the event.
    void addEventToBeam(const std::string& eventId, const part::Beam& beam)
    {
        auto result = m_eventsInBeams.emplace(eventId, BeamMappingEntry{ MappedLocation{ beam.pointer(), part::Beam::JsonSchemaTypeName }, 0 });
        if (!result.second) {
            mapping_error err("ID " + formatKeyString(eventId) + " already exists in beam "
                + result.first->second.location.location.to_string());
            if (m_errorHandler) {
                m_errorHandler.value()(err.what(), beam);
            } else {
                throw err;
            }
        }
    }

    /// @brief Record (or lower) the beam depth that starts at an event.
    void setEventBeamStartLevel(const std::string& eventId, int level)
    {
        auto it = m_eventsInBeams.find(eventId);
        if (it == m_eventsInBeams.end()) {
            throw std::logic_error("Attempted to assign a beam start level to unmapped event " + eventId);
        }
        if (it->second.startLevel == 0) {
            it->second.startLevel = level;
        } else {
            it->second.startLevel = std::min(it->second.startLevel, level);
        }
    }

    /// @brief Cache the ottava shift for a specific event pointer.
    void setEventOttavaShift(const std::string& eventPointer, int shift)
    {
        m_eventOttavaShift[eventPointer] = shift;
    }

    /// @brief Cache the mapped position for a specific event pointer.
    void setEventPosition(const std::string& eventPointer, const MappedPosition& position)
    {
        m_eventPositions.insert_or_assign(eventPointer, position);
    }

    friend class mnx::Document;

public:

    /**
     * @brief Constructs the index for a given document.
     * @param documentRoot Shared pointer to the document's JSON root.
     * @param errorHandler A optional callback function for handling errors.
     */
    explicit EntityMap(std::weak_ptr<json> documentRoot, const std::optional<ErrorHandler>& errorHandler = std::nullopt)
        : m_root(documentRoot), m_errorHandler(errorHandler) {}
        
    /**
     * @brief Attempts to look up an object by string ID.
     * @tparam T The expected type (e.g., mnx::Part, mnx::Layout, mnx::sequence::Note).
     * @param id The ID to search for.
     * @param errorLocation The location in the document for error reporting purposes.
     * @return An instance of T if found; std::nullopt if the ID is not present.
     *
     * @throws mapping_error if the ID is found but has a different (or unknown) type than @p T.
     *
     * @note Because every id in the document is indexed, a type mismatch usually means the
     *       document references an id of the wrong kind of object (for example, a slur target
     *       that names a note instead of an event). This function only models the *absence*
     *       of an ID, not type ambiguity.
     */
    template <typename T>
    std::optional<T> tryGet(
        const std::string& id,
        const std::optional<Base>& errorLocation = std::nullopt) const
    {
        auto it = m_objectMap.find(id);
        if (it == m_objectMap.end()) {
            return std::nullopt;
        }
        if (!detail::matchesTypeName<T>(it->second.typeName)) {
            mapping_error err(
                "ID " + formatKeyString(id) + " has " + (it->second.typeName.empty() ? "unknown type" : "type \"" + it->second.typeName + "\"")
                + ", but " + detail::typeNamesExpectationString<T>() + "."
            );
            if (m_errorHandler) {
                m_errorHandler.value()(err.what(), errorLocation.value_or(Document(root())));
            }
            throw err;
        }
        return T(root(), it->second.location);
    }

    /**
     * @brief Looks up an object by string ID.
     * @tparam T The expected type (e.g., mnx::Part, mnx::Layout, mnx::sequence::Note).
     * @param id The ID to search for.
     * @param errorLocation The location in the document for error reporting purposes.
     * @return An instance of T if found.
     * @throws mapping_error if the ID is not found.
     */
    template <typename T>
    T get(const std::string& id, const std::optional<Base>& errorLocation = std::nullopt) const
    {
        if (auto v = tryGet<T>(id, errorLocation)) {
            return *std::move(v);
        }
        mapping_error err("ID " + formatKeyString(id) + " not found in ID mapping");
        if (m_errorHandler) {
            m_errorHandler.value()(err.what(), errorLocation.value_or(Document(root())));
        }
        throw err;
    }

    /// @brief Returns the array index of an object identified by ID.
    ///
    /// Looks up an object of type @p T by its ID and returns the index of that
    /// object within its owning array. This is equivalent to calling #get<T>()
    /// followed by #ArrayElementObject::calcArrayIndex().
    ///
    /// @tparam T The expected object type. Must derive from ArrayElementObject.
    /// @param id The ID of the object to locate.
    /// @param errorLocation Optional document location used for error reporting.
    /// @return The zero-based array index of the object.
    /// @throws mapping_error if the ID is not found or does not refer to an object of type @p T.
    template <typename T>
    size_t getIndexOf(const std::string& id, const std::optional<Base>& errorLocation = std::nullopt) const
    {
        static_assert(std::is_base_of_v<ArrayElementObject, T>,
                    "getIndexOf<T> requires T to derive from ArrayElementObject");

        auto v = get<T>(id, errorLocation);
        return v.calcArrayIndex();
    }


    /// @brief Returns whether the specified ID exists in the mapping with type @p T.
    template <typename T>
    bool exists(const std::string& id) const
    {
        const auto it = m_objectMap.find(id);
        if (it == m_objectMap.end()) {
            return false;
        }
        return detail::matchesTypeName<T>(it->second.typeName);
    }

    /// @brief Looks up an id without regard to type.
    /// @param id The id to search for.
    /// @return The entry if the id exists in the document; std::nullopt otherwise.
    [[nodiscard]] std::optional<IdEntry> tryFind(const std::string& id) const
    {
        const auto it = m_objectMap.find(id);
        if (it == m_objectMap.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    /// @brief Returns the number of ids in the mapping.
    [[nodiscard]] size_t idCount() const { return m_objectMap.size(); }

    /// @brief Calls @p callback for every id in the mapping (in no particular order).
    /// @param callback A callable accepting `(const std::string& id, const IdEntry& entry)`.
    template <typename F>
    void forEachId(F&& callback) const
    {
        for (const auto& [id, entry] : m_objectMap) {
            callback(id, entry);
        }
    }

    /// @brief Get the beam for an event, if it is mapped.
    /// @param event The event to search for.
    /// @return The beam or std::nullopt if not found.
    std::optional<part::Beam> tryGetBeam(const sequence::Event& event) const
    {
        if (const auto& eventId = event.id()) {
            const auto it = m_eventsInBeams.find(eventId.value());
            if (it != m_eventsInBeams.end()) {
                MNX_ASSERT_IF(it->second.location.typeName != part::Beam::JsonSchemaTypeName) {
                    mapping_error err(
                        "The beam mapping for eventId " + formatKeyString(eventId.value())
                        + " was mapped to an object of type \"" + std::string(it->second.location.typeName) + "\"."
                    );
                    if (m_errorHandler) {
                        m_errorHandler.value()(err.what(), event);
                    }
                    throw err;
                }
                return part::Beam(root(), it->second.location.location);
            }
        }
        return std::nullopt;
    }

    /// @brief Return the secondary beam depth that starts at an event ID, if any.
    [[nodiscard]] std::optional<int> tryGetBeamStartLevel(const std::string& eventId) const
    {
        const auto it = m_eventsInBeams.find(eventId);
        if (it == m_eventsInBeams.end()) {
            return std::nullopt;
        }
        return it->second.startLevel;
    }

    /// @brief Return the beam start level for an event ID or 0 if none.
    [[nodiscard]] int getBeamStartLevel(const std::string& eventId) const
    {
        if (const auto level = tryGetBeamStartLevel(eventId)) {
            return *level;
        }
        return 0;
    }

    /// @brief Clears all mapped items.
    void clear()
    {
        m_objectMap.clear();
        m_eventsInBeams.clear();
        m_eventPositions.clear();
        m_eventOttavaShift.clear();
        m_lyricLineOrder.clear();
    }

    /// @brief Retrieve the ottava shift for an event (if known).
    [[nodiscard]] std::optional<int> tryGetOttavaShift(const sequence::Event& event) const
    {
        const auto it = m_eventOttavaShift.find(event.pointer().to_string());
        if (it == m_eventOttavaShift.end()) {
            return std::nullopt;
        }
        return -it->second;
    }

    /// @brief Retrieve the ottava shift for an event. Returns 0 if not cached.
    [[nodiscard]] int getOttavaShift(const sequence::Event& event) const
    {
        if (auto shift = tryGetOttavaShift(event)) {
            return shift.value();
        }
        return 0;
    }

    /// @brief Retrieve the cached mapped position for an event (if known).
    [[nodiscard]] std::optional<MappedPosition> tryGetEventPosition(const sequence::Event& event) const
    {
        const auto it = m_eventPositions.find(event.pointer().to_string());
        if (it == m_eventPositions.end()) {
            return std::nullopt;
        }
        return it->second;
    }

    /// @brief Retrieve the cached mapped position for an event.
    [[nodiscard]] MappedPosition getEventPosition(const sequence::Event& event) const
    {
        if (auto position = tryGetEventPosition(event)) {
            return *std::move(position);
        }
        throw std::logic_error("No mapped position cached for event at " + event.pointer().to_string());
    }

    /// @brief Compare two mapped positions using global measure order and grace-note targeting.
    /// @details The inputs may be native MappedPosition values or regular document-backed
    ///          MeasureRhythmicPosition objects converted implicitly to MappedPosition,
    ///          provided their `measureId` values refer to global measure IDs already present
    ///          in this EntityMap.
    /// @param lhs The left-hand position to compare.
    /// @param rhs The right-hand position to compare.
    /// @param rhsIncludesTrailingGrace When true, a right-hand position with
    ///        `graceIndex == 0` is treated as also including trailing grace notes
    ///        at the same fractional position, so those grace positions compare
    ///        equal to @p rhs rather than later than it.
    /// @return -1 if @p lhs is earlier, 1 if later, or 0 if equivalent for the requested policy.
    [[nodiscard]] int comparePositions(const MappedPosition& lhs,
                                       const MappedPosition& rhs,
                                       bool rhsIncludesTrailingGrace = false) const;

    /// @brief Retrieve the lyric line order.
    [[nodiscard]] const std::vector<std::string>& getLyricLineOrder() const
    { return m_lyricLineOrder; }

private:
    std::weak_ptr<json> m_root;
    std::optional<ErrorHandler> m_errorHandler;

    std::shared_ptr<json> root() const
    {
        std::shared_ptr<json> result = m_root.lock();
        if (!result) {
            throw std::runtime_error("ID mapping is invalid because the document was destroyed.");
        }
        return result;
    }

    using MappedLocation = struct
    {
        json_pointer location;          ///< location of instance in JSON
        std::string_view typeName;      ///< schema name of type for this instance
    };
    std::unordered_map<std::string, IdEntry> m_objectMap;
    struct BeamMappingEntry
    {
        MappedLocation location;
        int startLevel{0};
    };
    std::unordered_map<std::string, BeamMappingEntry> m_eventsInBeams;
    std::unordered_map<std::string, MappedPosition> m_eventPositions;
    std::unordered_map<std::string, int> m_eventOttavaShift;
    std::vector<std::string> m_lyricLineOrder;

    static std::string formatKeyString(const std::string& key) {
        return "\"" + key + "\"";
    }

    static std::string displayTypeName(const std::string& typeName) {
        return typeName.empty() ? std::string(UNKNOWN_TYPE_NAME) : typeName;
    }

};

} // namespace mnx::util
