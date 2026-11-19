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
#include "mnxdom.h"
#include <charconv>
#include <stdexcept>

#ifdef MNXDOM_USE_LEGACY_SCHEMA_VALIDATOR
#include "util/IdIndexer.h"
#include <nlohmann/json-schema.hpp>
#else
#include <jsonschema/schema.hpp>
#include <algorithm>
#include <iterator>
#include <unordered_map>
#endif
#include "mnx-schema.xxd"

namespace mnx {

namespace {

std::string_view embeddedSchemaText()
{
    static const std::string_view MNX_SCHEMA(reinterpret_cast<const char*>(mnx_schema_json), mnx_schema_json_len);
    return MNX_SCHEMA;
}

const std::string& embeddedSchemaId()
{
    static const std::string schemaId = [] {
        const json& schemaJson = getMnxSchemaJson();
        const auto it = schemaJson.find("$id");
        if (it == schemaJson.end() || !it->is_string()) {
            throw std::runtime_error("MNX schema is missing a string \"$id\" field.");
        }
        return it->template get<std::string>();
    }();
    return schemaId;
}

int parseVersionFromSchemaId(const std::string& schemaId)
{
    const auto slashPos = schemaId.find_last_of('/');
    const size_t start = (slashPos == std::string::npos) ? 0 : slashPos + 1;
    if (start >= schemaId.size()) {
        throw std::runtime_error("MNX schema \"$id\" has no trailing version segment: " + schemaId);
    }

    int version = 0;
    const char* first = schemaId.data() + start;
    const char* last = schemaId.data() + schemaId.size();
    const auto parseResult = std::from_chars(first, last, version);
    if (parseResult.ec != std::errc{} || parseResult.ptr != last) {
        throw std::runtime_error("MNX schema \"$id\" trailing segment is not an integer version: " + schemaId);
    }
    return version;
}

} // namespace

const json& getMnxSchemaJson()
{
    static const json schemaJson = json::parse(embeddedSchemaText());
    return schemaJson;
}

const std::string& getMnxSchemaId()
{
    return embeddedSchemaId();
}

int getMnxSchemaVersion()
{
    static const int version = parseVersionFromSchemaId(getMnxSchemaId());
    return version;
}

namespace validation {

namespace {

void addSchemaError(ValidationResult& result, const json_pointer& location, const json& instance,
                    const std::string& message)
{
    // Only deliver the first error at or below an already reported location.
    for (json_pointer path = location;; path.pop_back()) {
        for (const auto& error : result.errors) {
            if (error.pointer == path) {
                return;
            }
        }
        if (path.empty()) {
            break;
        }
    }
    result.errors.emplace_back(location, instance, message);
}

#ifdef MNXDOM_USE_LEGACY_SCHEMA_VALIDATOR
class SchemaErrorHandler final : public nlohmann::json_schema::error_handler
{
public:
    explicit SchemaErrorHandler(ValidationResult& result) : m_result(result) {}

    void error(const json_pointer& location, const json& instance, const std::string& message) override
    {
        addSchemaError(m_result, location, instance, message);
    }

private:
    ValidationResult& m_result;
};

void validate(const json& instance, const json& schema, ValidationResult& result,
              std::vector<DocumentIdIndex::Item>* ids)
{
    nlohmann::json_schema::json_validator validator;
    validator.set_root_schema(schema);
    SchemaErrorHandler handler(result);
    validator.validate(instance, handler);
    if (ids) {
        util::detail::indexDocumentIds(instance, schema, [&](DocumentIdIndex::Item item) {
            ids->push_back(std::move(item));
        });
    }
}
#else
class SchemaErrorHandler final : public jsonschema::error_sink
{
public:
    explicit SchemaErrorHandler(ValidationResult& result) : m_result(result) {}

    void on_error(const jsonschema::error& error) override
    {
        addSchemaError(m_result, error.instance_location, *error.instance, error.message);
    }

private:
    ValidationResult& m_result;
};

class IdClassificationSink final : public jsonschema::classification_sink
{
public:
    explicit IdClassificationSink(std::vector<DocumentIdIndex::Item>& ids) : m_ids(ids) {}

    void on_classified(const jsonschema::classification& classification) override
    {
        const auto id = classification.instance.find("id");
        if (id == classification.instance.end() || !id->is_string()) {
            return;
        }
        for (const auto& match : classification.matches) {
            if (match.confidence != jsonschema::confidence::ambiguous && match.info.declares_property("id")) {
                auto& items = classification.branch ? m_pending[classification.branch] : m_ids;
                items.push_back({id->get<std::string>(), classification.instance_location, std::string(match.name)});
            }
        }
    }

    void on_branch(const jsonschema::classification_branch& branch) override
    {
        if (branch.action == jsonschema::branch_action::begin) {
            return;
        }
        auto pending = m_pending.extract(branch.id);
        if (pending.empty() || branch.action == jsonschema::branch_action::discard) {
            return;
        }
        auto& items = pending.mapped();
        if (branch.ambiguous_at) {
            std::erase_if(items, [&](const auto& item) { return item.location == *branch.ambiguous_at; });
        }
        if (!items.empty()) {
            auto& target = branch.parent ? m_pending[branch.parent] : m_ids;
            target.insert(target.end(), std::make_move_iterator(items.begin()), std::make_move_iterator(items.end()));
        }
    }

private:
    std::vector<DocumentIdIndex::Item>& m_ids;
    std::unordered_map<jsonschema::classification_branch_id, std::vector<DocumentIdIndex::Item>> m_pending;
};

jsonschema::schema compileMnxSchema(const json& schema, bool collectIds)
{
    jsonschema::compile_options options;
    if (collectIds) {
        options.classification.enabled = true;
        options.classification.name_rule = jsonschema::namer::custom([](const jsonschema::schema_node_view& node)
            -> std::optional<std::string> {
            // MNX entities are concrete definitions. The shared attributes also match
            // at the entity location, but the enclosing class owns the ID.
            if (node.parent_keyword == "$defs" && node.key != "global-attrs"
                && (node.schema.contains("properties") || node.schema.contains("allOf"))) {
                return std::string(node.key);
            }
            return std::nullopt;
        });
    }
    return jsonschema::schema::compile(schema, options);
}

const jsonschema::schema& compiledMnxSchema(bool collectIds)
{
    if (collectIds) {
        static const auto indexed = compileMnxSchema(getMnxSchemaJson(), true);
        return indexed;
    }
    static const auto plain = compileMnxSchema(getMnxSchemaJson(), false);
    return plain;
}

void validate(const json& instance, const json& schema, ValidationResult& result,
              std::vector<DocumentIdIndex::Item>* ids)
{
    SchemaErrorHandler handler(result);
    std::optional<IdClassificationSink> sink;
    if (ids) {
        sink.emplace(*ids);
    }
    const jsonschema::validation_options options{.errors = &handler, .classifications = sink ? &*sink : nullptr};
    if (&schema == &getMnxSchemaJson()) {
        (void)compiledMnxSchema(ids != nullptr).validate(instance, options);
    } else {
        (void)compileMnxSchema(schema, ids != nullptr).validate(instance, options);
    }
}
#endif

} // namespace

SchemaValidationResult schemaValidate(const Document& document, const SchemaValidationOptions& options)
{
    SchemaValidationResult result;
    const auto customSchema = options.jsonSchema ? std::optional<json>(json::parse(*options.jsonSchema)) : std::nullopt;
    const json& schema = customSchema ? *customSchema : getMnxSchemaJson();
    std::vector<DocumentIdIndex::Item> items;
    validate(*document.root(), schema, result, options.collectIds ? &items : nullptr);
    if (options.collectIds) {
        result.ids = DocumentIdIndex(std::move(items));
    }
    return result;
}

} // namespace validation
} // namespace mnx
