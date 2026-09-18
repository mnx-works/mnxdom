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
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "util/IdIndexer.h"

namespace mnx::util::detail {

namespace {

constexpr std::string_view DEFS_PREFIX = "#/$defs/";
constexpr std::string_view VENDOR_EXTENSIONS_KEY = "_x";
constexpr std::string_view COMMENT_KEY = "_c";
constexpr std::string_view ID_KEY = "id";
constexpr std::string_view TYPE_KEY = "type";

/// @brief A position in the schema that describes the instance node currently being walked.
/// A null @p node means the instance could not be matched to the schema.
struct Cursor
{
    const json* node{ nullptr };
    std::string typeName;       ///< the `$defs` name last followed to reach @p node (empty if inline or unknown)
};

/// @brief How a property key was matched in an object schema.
enum class PropertyMatch
{
    None,       ///< the schema does not describe this key
    Named,      ///< matched through `properties`
    Pattern     ///< matched through `patternProperties` or `additionalProperties` (a dictionary entry)
};

class SchemaWalker
{
public:
    SchemaWalker(const json& schema, const std::function<void(const IndexedId&)>& sink)
        : m_schema(schema), m_sink(sink)
    {
        if (const auto it = schema.find("$defs"); it != schema.end() && it->is_object()) {
            m_defs = &*it;
        }
    }

    void run(const json& root)
    {
        walk(root, json_pointer{}, resolve(&m_schema, "", root));
    }

private:
    /// @brief Follows `$ref` chains and `anyOf`/`oneOf` branches until a concrete schema node is reached.
    /// @param node The schema node to resolve (may be null).
    /// @param typeName The type name accumulated so far.
    /// @param instance The instance the node describes, used to discriminate `anyOf`/`oneOf` branches.
    Cursor resolve(const json* node, std::string typeName, const json& instance) const
    {
        for (int guard = 0; node && guard < 64; ++guard) {
            if (!node->is_object()) {
                return Cursor{};
            }
            if (const auto ref = node->find("$ref"); ref != node->end()) {
                if (!ref->is_string()) {
                    return Cursor{};
                }
                const auto refValue = ref->get<std::string>();
                if (!m_defs || refValue.compare(0, DEFS_PREFIX.size(), DEFS_PREFIX) != 0) {
                    return Cursor{};
                }
                typeName = refValue.substr(DEFS_PREFIX.size());
                const auto def = m_defs->find(typeName);
                if (def == m_defs->end()) {
                    return Cursor{};
                }
                node = &*def;
                continue;
            }
            const json* branches = nullptr;
            if (const auto anyOf = node->find("anyOf"); anyOf != node->end() && anyOf->is_array()) {
                branches = &*anyOf;
            } else if (const auto oneOf = node->find("oneOf"); oneOf != node->end() && oneOf->is_array()) {
                branches = &*oneOf;
            }
            if (branches) {
                node = selectBranch(*branches, instance);
                continue;
            }
            return Cursor{ node, std::move(typeName) };
        }
        return Cursor{};
    }

    /// @brief Chooses the `anyOf`/`oneOf` branch whose `type` const matches the instance's `type` property.
    /// When the instance has no `type`, the branch that does not require one (the default type, such as
    /// `event` in sequence content) is chosen if there is exactly one such branch.
    /// @return The branch or null if the instance cannot be discriminated.
    const json* selectBranch(const json& branches, const json& instance) const
    {
        if (branches.size() == 1) {
            return &branches[0];
        }
        if (!instance.is_object()) {
            return nullptr;
        }
        const auto typeValue = instance.find(TYPE_KEY);
        const bool hasType = typeValue != instance.end() && typeValue->is_string();
        const json* defaultBranch = nullptr;
        for (const auto& branch : branches) {
            const Cursor resolved = resolve(&branch, "", instance);
            if (!resolved.node) {
                continue;
            }
            if (!hasType) {
                if (!requiresProperty(*resolved.node, TYPE_KEY)) {
                    if (defaultBranch) {
                        return nullptr; // ambiguous
                    }
                    defaultBranch = &branch;
                }
                continue;
            }
            const auto properties = resolved.node->find("properties");
            if (properties == resolved.node->end() || !properties->is_object()) {
                continue;
            }
            const auto typeProperty = properties->find(TYPE_KEY);
            if (typeProperty == properties->end() || !typeProperty->is_object()) {
                continue;
            }
            const auto constValue = typeProperty->find("const");
            if (constValue != typeProperty->end() && *constValue == *typeValue) {
                return &branch;
            }
        }
        return defaultBranch;
    }

    static bool requiresProperty(const json& node, std::string_view key)
    {
        const auto required = node.find("required");
        if (required == node.end() || !required->is_array()) {
            return false;
        }
        for (const auto& name : *required) {
            if (name.is_string() && name.get_ref<const std::string&>() == key) {
                return true;
            }
        }
        return false;
    }

    /// @brief Finds the schema for a property of an object schema, searching `properties`,
    /// `allOf` members, `patternProperties`, and `additionalProperties` in that order.
    std::pair<const json*, PropertyMatch> findProperty(const json* node, const std::string& key) const
    {
        if (!node || !node->is_object()) {
            return { nullptr, PropertyMatch::None };
        }
        if (const auto properties = node->find("properties"); properties != node->end() && properties->is_object()) {
            if (const auto it = properties->find(key); it != properties->end()) {
                return { &*it, PropertyMatch::Named };
            }
        }
        if (const auto allOf = node->find("allOf"); allOf != node->end() && allOf->is_array()) {
            for (const auto& member : *allOf) {
                // allOf members carry no discriminated branches in MNX, so the instance is irrelevant here.
                const Cursor resolved = resolve(&member, "", json{});
                if (auto found = findProperty(resolved.node, key); found.second != PropertyMatch::None) {
                    return found;
                }
            }
        }
        if (const auto patterns = node->find("patternProperties"); patterns != node->end() && patterns->is_object()) {
            for (const auto& [pattern, patternSchema] : patterns->items()) {
                if (std::regex_search(key, regexFor(pattern))) {
                    return { &patternSchema, PropertyMatch::Pattern };
                }
            }
        }
        if (const auto additional = node->find("additionalProperties"); additional != node->end() && additional->is_object()) {
            return { &*additional, PropertyMatch::Pattern };
        }
        return { nullptr, PropertyMatch::None };
    }

    const std::regex& regexFor(const std::string& pattern) const
    {
        auto it = m_regexCache.find(pattern);
        if (it == m_regexCache.end()) {
            it = m_regexCache.emplace(pattern, std::regex(pattern, std::regex::ECMAScript)).first;
        }
        return it->second;
    }

    void walk(const json& instance, const json_pointer& pointer, const Cursor& cursor)
    {
        if (instance.is_object()) {
            walkObject(instance, pointer, cursor);
        } else if (instance.is_array()) {
            const json* itemsNode = nullptr;
            if (cursor.node) {
                if (const auto items = cursor.node->find("items"); items != cursor.node->end()) {
                    itemsNode = &*items;
                }
            }
            for (size_t index = 0; index < instance.size(); ++index) {
                const auto& element = instance[index];
                walk(element, pointer / index, resolve(itemsNode, "", element));
            }
        }
    }

    void walkObject(const json& instance, const json_pointer& pointer, const Cursor& cursor)
    {
        for (const auto& [key, value] : instance.items()) {
            if (key == VENDOR_EXTENSIONS_KEY || key == COMMENT_KEY) {
                continue;
            }
            const auto [propertySchema, match] = findProperty(cursor.node, key);
            if (key == ID_KEY && value.is_string()) {
                if (match == PropertyMatch::Pattern) {
                    // a dictionary entry that happens to be keyed "id", not an id
                    continue;
                }
                // A named `id` is classified by the enclosing object's schema definition. An `id`
                // the schema does not describe here is still recorded, but with no type name.
                m_sink(IndexedId{ value.get<std::string>(), pointer, match == PropertyMatch::Named ? cursor.typeName : std::string() });
                continue;
            }
            if (!value.is_object() && !value.is_array()) {
                continue;
            }
            walk(value, pointer / key, resolve(propertySchema, "", value));
        }
    }

    const json& m_schema;
    const json* m_defs{ nullptr };
    const std::function<void(const IndexedId&)>& m_sink;
    mutable std::unordered_map<std::string, std::regex> m_regexCache;
};

} // namespace

void indexDocumentIds(const json& root, const json& schema, const std::function<void(const IndexedId&)>& sink)
{
    SchemaWalker walker(schema, sink);
    walker.run(root);
}

} // namespace mnx::util::detail
