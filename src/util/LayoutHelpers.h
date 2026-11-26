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

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../BaseTypes.h"
#include "../Global.h"
#include "../Layout.h"

namespace mnx::util {

/// @brief Identifies a specific staff within a specific part.
///
/// A StaffKey uniquely identifies a part staff by pairing the part ID with
/// the 1-based staff index within that part. It is used to reason about
/// layout–part correspondence when validating or selecting conventional
/// score layouts.
struct StaffKey
{
    /// @brief The ID of the part.
    std::string partId;

    /// @brief The 1-based staff number within the part.
    int staffNo = 1;

    /// @brief Equality comparison.
    ///
    /// Two StaffKey values are equal if they refer to the same part ID
    /// and the same staff number within that part.
    bool operator==(const StaffKey& o) const noexcept
    {
        return staffNo == o.staffNo && partId == o.partId;
    }
};

/// @brief Hash functor for StaffKey.
///
/// Enables StaffKey to be used as a key in unordered associative containers.
struct StaffKeyHash
{
    /// @brief Computes a hash value for a StaffKey.
    ///
    /// The hash combines the part ID and staff number. It is intended for
    /// small collections and prioritizes simplicity over cryptographic strength.
    size_t operator()(const StaffKey& k) const noexcept
    {
        size_t h1 = std::hash<std::string>{}(k.partId);
        size_t h2 = std::hash<int>{}(k.staffNo);
        return h1 ^ (h2 + 0x9e3779b9u + (h1 << 6) + (h1 >> 2));
    }
};

/// @brief The set of staff keys contained in a single layout staff.
using LayoutStaffKeySet = std::unordered_set<StaffKey, StaffKeyHash>;

/// @brief Analyzes a single layout staff for semantic voice usage.
///
/// For each distinct StaffKey (partId + staffNo) referenced by this staff:
/// - The StaffKey may appear exactly once (voice optional).
/// - If it appears multiple times, then every occurrence must specify
///   a non-empty voice, and all voice values must be distinct.
///
/// Multiple parts and multiple part staves on a single layout staff are permitted.
///
/// @param staff The layout staff to analyze.
/// @return A set of StaffKey values if the staff is semantically valid;
///         std::nullopt otherwise.
[[nodiscard]] inline std::optional<LayoutStaffKeySet>
analyzeLayoutStaffVoices(const layout::Staff& staff)
{
    const auto sources = staff.sources();
    if (sources.empty()) {
        return std::nullopt;
    }

    struct KeyState
    {
        size_t count = 0;
        bool anyMissingVoice = false;
        std::unordered_set<std::string> voices;
    };

    std::unordered_map<StaffKey, KeyState, StaffKeyHash> stateByKey;
    stateByKey.reserve(sources.size());

    for (const auto src : sources) {
        const std::string partId = src.part();
        if (partId.empty()) {
            return std::nullopt;
        }

        const StaffKey key{partId, src.staff()};
        auto& st = stateByKey[key];
        ++st.count;

        const auto v = src.voice();
        if (!v) {
            st.anyMissingVoice = true;
            continue;
        }

        if (v->empty()) {
            return std::nullopt;
        }

        if (!st.voices.emplace(*v).second) {
            return std::nullopt; // duplicate voice for same StaffKey
        }
    }

    // Enforce per-StaffKey semantic rule.
    for (const auto& kv : stateByKey) {
        const KeyState& st = kv.second;
        if (st.count > 1) {
            if (st.anyMissingVoice || st.voices.size() != st.count) {
                return std::nullopt;
            }
        }
    }

    LayoutStaffKeySet result;
    result.reserve(stateByKey.size());
    for (const auto& kv : stateByKey) {
        result.insert(kv.first);
    }

    return result;
}

/// @brief Flattens a layout hierarchy into a linear sequence of staves.
///
/// Traverses the layout's content depth-first, recursively expanding groups and
/// appending staff elements in encounter order. The resulting sequence preserves
/// the visual staff order implied by the layout.
///
/// @param layout The layout whose content should be traversed.
/// @return A vector of layout::Staff elements in encounter order if traversal succeeds;
///         std::nullopt if an unsupported content element is encountered.
[[nodiscard]] inline std::optional<std::vector<layout::Staff>>
flattenLayoutStaves(const Layout& layout)
{
    auto content = layout.content();
    std::vector<layout::Staff> result;
    result.reserve(content.size()); // lower bound; groups may expand further

    const auto walk = [&](auto&& self, const layout::LayoutContent& content) -> std::optional<bool> {
        for (auto elem : content) {
            if (elem.type() == layout::Group::ContentTypeValue) {
                layout::Group g = elem.get<layout::Group>();
                auto ok = self(self, g.content());
                if (!ok) {
                    return std::nullopt;
                }
            } else if (elem.type() == layout::Staff::ContentTypeValue) {
                result.push_back(elem.get<layout::Staff>());
            } else {
                return std::nullopt;
            }
        }
        return true;
    };

    if (!walk(walk, content)) {
        return std::nullopt;
    }
    return result;
}

/**
 * @struct LayoutSpanLabelRef
 * @brief A value-type summary of a layout::LabelRef, independent of the source document.
 */
struct LayoutSpanLabelRef
{
    LabelRefType ref = LabelRefType::Auto;  ///< Which part name to display.
    std::optional<std::string> target;      ///< The ID of the part whose name is displayed.

    /// @brief Default constructor.
    LayoutSpanLabelRef() = default;

    /// @brief Copies the values of a layout::LabelRef.
    /// @param src The label ref to copy.
    explicit LayoutSpanLabelRef(const layout::LabelRef& src)
        : ref(src.ref()), target(src.target())
    {
    }
};

/**
 * @struct LayoutSpan
 * @brief Describes a visual span in a flattened MNX layout.
 *
 * A LayoutSpan represents either a single staff or a group of staves
 * after the layout hierarchy has been flattened. Each span records
 * the range of staff indices it covers in the flattened staff list,
 * its nesting depth in the layout hierarchy, and any visual metadata
 * (label, label reference, symbol, and resolved barline override) associated with it.
 *
 * Spans are typically sorted by ascending startIndex and then by
 * ascending depth, so that outer groups precede inner groups and
 * staff spans appear at the deepest level.
 */
struct LayoutSpan
{
    /**
     * @enum Kind
     * @brief Identifies whether this span represents a staff or a group.
     */
    enum class Kind
    {
        Staff,  ///< Span represents a single staff.
        Group   ///< Span represents a group of staves.
    };

    Kind kind{}; ///< The kind of layout element represented by this span.

    /**
     * @brief Index of the first staff covered by this span.
     *
     * This index refers to the position of the staff in the flattened
     * staff sequence produced by depth-first traversal of the layout.
     */
    size_t startIndex{};

    /**
     * @brief Index of the last staff covered by this span.
     *
     * For staff spans, endIndex is equal to startIndex. For group spans,
     * this is the index of the final staff contained anywhere within
     * the group.
     */
    size_t endIndex{};

    /**
     * @brief Nesting depth of this span within the layout hierarchy.
     *
     * Depth reflects the layout content tree level: groups live at the
     * container depth, and staff spans are always one level deeper
     * (leaf depth = container depth + 1). This means ungrouped staves
     * in the root content still have depth 1, while root-level groups
     * have depth 0. Example: with root content [Flute, Clarinet, PianoGroup],
     * where PianoGroup contains two staves, the group span for PianoGroup has
     * depth 0, while all staff spans (Flute, Clarinet, and both piano staves)
     * have depth 1. For a given startIndex, group spans always have a smaller
     * depth than the staff span they contain, ensuring staff spans sort last.
     */
    size_t depth{};

    /**
     * @brief Optional label text associated with this span.
     *
     * For staff spans, this corresponds to Staff::label. For group
     * spans, it corresponds to Group::label.
     */
    std::optional<std::string> label;

    /**
     * @brief Optional label reference associated with this span.
     *
     * For staff spans, this corresponds to Staff::labelRef. For group
     * spans, it corresponds to Group::labelRef. When present, it refers
     * to a part's name or shortName. MNX recommends it over #label to avoid
     * duplicating part names; the specification does not define precedence
     * if both are present.
     *
     * For staff spans whose label ref omits a target, the target is set
     * to the part of the staff's first source, as specified by MNX. For
     * group spans, the target is as encoded; MNX leaves an omitted group
     * target to the consuming application.
     */
    std::optional<LayoutSpanLabelRef> labelRef;

    /**
     * @brief Layout symbol associated with this span.
     *
     * For staff spans, this corresponds to Staff::symbol. For group
     * spans, it corresponds to Group::symbol. The MNX default is
     * LayoutSymbol::NoSymbol.
     */
    LayoutSymbol symbol = LayoutSymbol::NoSymbol;

    /**
     * @brief Resolved barline override associated with this span.
     *
     * For group spans, this is Group::calcBarlineOverride(). For staff spans,
     * this is StaffGroupBarlineOverride::None.
     */
    StaffGroupBarlineOverride barlineOverride = StaffGroupBarlineOverride::None;

    /**
     * @brief Optional staff sources associated with this span.
     *
     * For staff spans, this corresponds to Staff::sources. For group
     * spans, it is std::nullopt.
     */
    std::optional<LayoutStaffKeySet> sources;
};

/// @brief Builds a sorted list of staff and group spans for a layout.
///
/// Traverses the layout’s content depth-first, computing flattened staff indices as staff elements
/// are encountered. For each staff, a span covering exactly one staff index is produced. For each
/// group, a span covering the inclusive range of staff indices contained anywhere within that group
/// is produced. Groups that contain no staves (directly or indirectly) are skipped.
///
/// The returned spans are sorted by ascending #LayoutSpan::startIndex and then by ascending
/// #LayoutSpan::depth. This ordering places outer groups before inner groups when they start at the
/// same staff, and places the staff span at the deepest depth for its start index.
///
/// Depth follows the content tree level: groups live at the container depth, and staff spans are
/// one level deeper (leaf depth = container depth + 1). This means ungrouped staves in the root
/// content have depth 1, while root-level groups have depth 0.
///
/// If an unsupported content element is encountered during traversal, the function fails and
/// returns std::nullopt.
///
/// @param layout The layout whose content hierarchy should be traversed.
/// @return A sorted vector of LayoutSpan entries on success; std::nullopt if traversal fails due to
///         an unsupported content element.
[[nodiscard]] inline std::optional<std::vector<LayoutSpan>>
buildLayoutSpans(const mnx::Layout& layout)
{
    const auto content = layout.content();
    std::vector<LayoutSpan> spans;
    spans.reserve(content.size()); // lower bound

    size_t staffIndex = 0;
    size_t encounter  = 0; // stable tiebreaker

    struct SortKey { size_t start, depth, encounter; };

    struct TaggedSpan {
        LayoutSpan span;
        SortKey key;
    };

    std::vector<TaggedSpan> tagged;
    tagged.reserve(content.size());

    // Return value:
    // - outer std::optional: hard failure (unsupported element) if empty
    // - inner std::optional: no staves in subtree if empty; otherwise first/last staff indices
    const auto walk =
        [&](auto&& self, const layout::LayoutContent& arr, size_t depth)
            -> std::optional<std::optional<std::pair<size_t,size_t>>>
    {
        std::optional<size_t> first;
        std::optional<size_t> last;

        for (auto elem : arr) {
            if (elem.type() == layout::Staff::ContentTypeValue) {
                layout::Staff s = elem.get<layout::Staff>();

                const size_t i = staffIndex++;

                LayoutSpan span;
                span.kind       = LayoutSpan::Kind::Staff;
                span.depth      = depth + 1; // staff spans must be at the deepest depth
                span.startIndex = i;
                span.endIndex   = i;
                span.symbol     = s.symbol();
                span.label      = s.label();
                if (const auto labelRef = s.labelRef()) {
                    LayoutSpanLabelRef value(*labelRef);
                    if (!value.target && !s.sources().empty()) {
                        value.target = s.sources()[0].part(); // MNX fallback for staves
                    }
                    span.labelRef = std::move(value);
                }
                span.barlineOverride = StaffGroupBarlineOverride::None;
                span.sources    = util::analyzeLayoutStaffVoices(s);

                tagged.push_back({ std::move(span), SortKey{i, depth + 1, encounter++} });

                first = first.value_or(i);
                last  = i;
            } else if (elem.type() == layout::Group::ContentTypeValue) {
                layout::Group g = elem.get<layout::Group>();

                auto childRange = self(self, g.content(), depth + 1);
                if (!childRange) {
                    return std::nullopt; // hard failure
                }
                if (!*childRange) {
                    continue; // skip groups with no staves anywhere in their subtree
                }
                const auto [cFirst, cLast] = **childRange;

                LayoutSpan span;
                span.kind       = LayoutSpan::Kind::Group;
                span.depth      = depth;
                span.startIndex = cFirst;
                span.endIndex   = cLast;
                span.symbol     = g.symbol();
                span.label      = g.label();
                if (const auto labelRef = g.labelRef()) {
                    span.labelRef = LayoutSpanLabelRef(*labelRef);
                }
                span.barlineOverride = g.calcBarlineOverride();

                tagged.push_back({ std::move(span), SortKey{cFirst, depth, encounter++} });

                first = first.value_or(cFirst);
                last  = cLast;
            } else {
                return std::nullopt; // unsupported content element
            }
        }

        if (!first || !last) {
            return std::optional<std::pair<size_t,size_t>>{}; // success, but no staves
        }
        return std::make_pair(*first, *last);
    };

    auto rootRange = walk(walk, content, /*depth*/0);
    if (!rootRange) {
        return std::nullopt;
    }
    // Empty root content is allowed; it simply produces an empty spans vector.

    std::stable_sort(tagged.begin(), tagged.end(),
        [](const TaggedSpan& a, const TaggedSpan& b)
        {
            if (a.key.start != b.key.start) return a.key.start < b.key.start;
            if (a.key.depth != b.key.depth) return a.key.depth < b.key.depth;
            return a.key.encounter < b.key.encounter;
        });

    spans.reserve(tagged.size());
    for (auto& t : tagged) spans.push_back(std::move(t.span));
    return spans;
}

/// @brief Builds a default set of layout spans when no explicit layout is available.
///
/// Constructs a fallback layout by grouping staves by part and assigning flattened staff indices
/// in part order. Each part contributes one or more staff spans, and parts with multiple staves
/// also contribute a group span covering all of their staves. Group spans use a brace symbol as a
/// reasonable default in the absence of an explicit layout definition.
///
/// Staff spans are assigned the deepest depth within their part so that they sort after their
/// enclosing group span when spans are ordered by start index and depth. This mirrors the explicit
/// layout semantics: group spans live at the container depth, and staff spans are one level deeper
/// (so single-staff parts still produce staff spans at depth 1). For single-staff parts, only a
/// staff span is produced, and the part's label is applied directly to that staff.
///
/// The part's label is a #LayoutSpan::labelRef with LabelRefType::Auto targeting the part's id,
/// so consuming software chooses between the part's name and shortName. If the part has no id,
/// the label ref cannot identify it, so #LayoutSpan::label is set to the part's name instead.
///
/// Parts that declare zero staves are ignored.
///
/// @param parts The array of parts from which to construct the default layout.
/// @return A vector of LayoutSpan entries representing the synthesized default layout.
[[nodiscard]] inline std::vector<LayoutSpan>
buildDefaultLayoutSpans(const Array<Part>& parts)
{
    std::vector<LayoutSpan> result;
    size_t staffIdx = 0;

    for (const auto& part : parts) {
        const size_t numStaves = static_cast<size_t>(part.staves());
        if (numStaves == 0) {
            continue;
        }
        const auto applyPartLabel = [&](LayoutSpan& span) {
            if (const auto partId = part.id()) {
                LayoutSpanLabelRef labelRef;
                labelRef.ref = LabelRefType::Auto;
                labelRef.target = partId;
                span.labelRef = std::move(labelRef);
            } else {
                span.label = part.name();
            }
        };
        size_t staffDepth = 1;
        bool staffNameNeeded = true;
        if (numStaves > 1) {
            LayoutSpan groupSpan;
            groupSpan.depth = 0;
            groupSpan.kind = LayoutSpan::Kind::Group;
            groupSpan.symbol = LayoutSymbol::Brace;
            groupSpan.startIndex = staffIdx;
            groupSpan.endIndex = staffIdx + numStaves - 1;
            applyPartLabel(groupSpan);
            groupSpan.barlineOverride = StaffGroupBarlineOverride::Unified;
            staffNameNeeded = false;
            result.emplace_back(std::move(groupSpan));
        }
        for (size_t x = 0; x < numStaves; x++) {
            LayoutSpan staffSpan;
            staffSpan.depth = staffDepth;
            staffSpan.kind = LayoutSpan::Kind::Staff;
            staffSpan.startIndex = staffIdx;
            staffSpan.endIndex = staffIdx;
            staffSpan.symbol = LayoutSymbol::NoSymbol;
            staffSpan.barlineOverride = StaffGroupBarlineOverride::None;
            if (staffNameNeeded) {
                applyPartLabel(staffSpan);
            }
            result.emplace_back(std::move(staffSpan));
            staffIdx++;
        }
    }

    return result;
}

} // namespace mnx::util
