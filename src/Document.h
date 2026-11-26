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

#include <filesystem>
#include <fstream>

#include "BaseTypes.h"
#include "DocumentIdIndex.h"
#include "MnxdomProvenance.h"
#include "Global.h"
#include "Layout.h"
#include "Part.h"
#include "Score.h"

namespace mnx {

namespace util {
class EntityMap;
}

/**
 * @struct EntityMapPolicies
 * @brief Controls optional behaviors when building an EntityMap.
 *
 * These policies allow consumers to relax certain MNX capabilities that they
 * might not support while still obtaining a useful ottava map.
 */
struct EntityMapPolicies
{
    /// @brief When false, ottava spans ignore grace-note targeting (graces follow the main event).
    bool ottavasRespectGraceTargets { true };
    /// @brief When false, ottava spans ignore voice-specific targeting (ottavas apply to the entire staff).
    bool ottavasRespectVoiceTargets { true };
};

/**
 * @class MnxMetaData
 * @brief Represents metadata for an MNX document.
 */
class MnxMetaData : public Object
{
public:
    using Object::Object;

    /// @brief Creates a new MnxMetaData class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    MnxMetaData(Base& parent, std::string_view key)
        : Object(parent, key)
    {
        // required children
        set_version(getMnxSchemaVersion());
        auto extensions = ensure__x();
        MnxdomExtensionData(extensions, MNXDOM_PROVENANCE_KEY);
    }

    /**
     * @class Support
     * @brief Represents optional support metadata within an MNX document.
     */
    class Support : public Object
    {
    public:
        using Object::Object;

        MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, useAccidentalDisplay, false);  ///< Optional property indicating whether accidental display is used.
        MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, useBeams, false);              ///< Optional property that indicates if beams are encoded.
        MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, useLyricExtenders, false);     ///< Whether lyric extenders are explicitly encoded

        inline static constexpr std::string_view JsonSchemaTypeName = "support";     ///< required for mapping
    };

    /**
     * @brief Required property indicating the version of the MNX document.
     */
    MNX_REQUIRED_PROPERTY(int, version);

    /**
     * @brief Optional child containing support metadata.
     */
    MNX_OPTIONAL_CHILD(Support, support);

    /// @brief Returns the mnxdom vendor extension stored in `_x.mnxdom`.
    [[nodiscard]] std::optional<MnxdomExtensionData> mnxdom() const
    {
        if (const auto value = getExtension(std::string(MNXDOM_PROVENANCE_KEY)); value.has_value() && value->is_object()) {
            return MnxdomExtensionData(root(), pointer() / "_x" / std::string(MNXDOM_PROVENANCE_KEY));
        }
        return std::nullopt;
    }

    /// @brief Ensures that `_x.mnxdom` exists and returns it.
    MnxdomExtensionData ensure_mnxdom()
    {
        if (auto existing = mnxdom()) {
            return *existing;
        }

        auto extensions = ensure__x();
        return MnxdomExtensionData(extensions, MNXDOM_PROVENANCE_KEY);
    }

    /// @brief Removes `_x.mnxdom`. If `_x` becomes empty, it is removed too.
    void clear_mnxdom()
    {
        clearExtension(std::string(MNXDOM_PROVENANCE_KEY));
        if (auto extensions = _x(); extensions && extensions->empty()) {
            clear__x();
        }
    }

    inline static constexpr std::string_view JsonSchemaTypeName = "mnx";     ///< required for mapping
};

/**
 * @class Document
 * @brief Represents an MNX document and provides methods for loading and saving.
 */
class Document : public Object
{
private:
    /// @brief Not really shared, but std::unique_ptr creates unacceptable dependencies in the headers
    std::shared_ptr<util::EntityMap> m_entityMapping;

    /// @brief Returns the entity mapping, creating an empty one if there is none.
    util::EntityMap& ensureEntityMap();

public:
    /**
     * @brief Constructs an empty MNX document. The resulting instance contains all
     * required fields and should validate against the schema.
     */
    Document() : Object()
    {
        // create required children
        create_mnx();
        create_global();
        create_parts();
    }

    /// @brief Wrap a document around a root element
    /// @param root 
    Document(const std::shared_ptr<json>& root) : Object(root, json_pointer{}) {}

    /// @brief Copy constructor that zaps the id mapping, if any
    Document(const Document& src) : Object(src), m_entityMapping(nullptr) {}

    using Base::root;

    /**
     * @brief Constructs a Document from an input stream.
     * @param inputStream The input stream containing the MNX JSON data.
     */
    Document(std::istream& inputStream) : Object(std::make_shared<json>(json::object()), json_pointer{})
    {
        inputStream >> *root();
    }

    MNX_REQUIRED_CHILD(Global, global);         ///< Global data for the MNX document.
    MNX_REQUIRED_CHILD(MnxMetaData, mnx);       ///< Metadata for the MNX document.
    MNX_OPTIONAL_CHILD(Array<Layout>, layouts); ///< List of layouts for the MNX document.
    MNX_REQUIRED_CHILD(Array<Part>, parts);     ///< List of parts for the MNX document.
    MNX_OPTIONAL_CHILD(Array<Score>, scores);   ///< List of scores for the MNX document.

    /**
     * @brief Creates a Document from a JSON file.
     * @param inputPath The path to the JSON file.
     * @return A Document instance populated with the parsed data.
     * @throws std::runtime_error if the file cannot be opened.
     */
    [[nodiscard]] static Document create(const std::filesystem::path& inputPath)
    {
        std::ifstream jsonFile;
        jsonFile.exceptions(std::ios::failbit | std::ios::badbit);
        jsonFile.open(inputPath);
        if (!jsonFile.is_open()) {
            throw std::runtime_error("Unable to open JSON file: " + inputPath.string());
        }
        return Document(jsonFile);
    }

    /**
     * @brief Creates a Document from a data buffer containing JSON.
     * @tparam Byte An 8-bit integer type (char, signed/unsigned char, std::uint8_t, etc.)
     * @param data Pointer to the start of the JSON text buffer.
     * @param size Number of bytes in the buffer.
     * @return A Document instance populated with the parsed data.
     * @throws std::invalid_argument if data is null and size is non-zero.
     * @throws nlohmann::json::parse_error (or other nlohmann::json exceptions) on parse failure.
     */
    template <typename Byte,
              typename = std::enable_if_t<
                         std::is_integral_v<Byte> &&
                         (sizeof(Byte) == 1) &&
                         !std::is_same_v<std::remove_cv_t<Byte>, bool>>>
    [[nodiscard]] static Document create(const Byte* data, std::size_t size)
    {
        if (!data && size != 0) {
            throw std::invalid_argument("JSON buffer pointer is null.");
        }
        auto root = std::make_shared<json>(nlohmann::json::parse(data, data + size));
        return Document(root);
    }

    /**
     * @brief Saves the MNX document to a file.
     * @param outputPath The file path to save the document.
     * @param indentSpaces Optional number of spaces for indentation; if not provided, no indentation is applied.
     * @throws std::ios_base::failure if file operations fail.
     */
    void save(const std::filesystem::path& outputPath, std::optional<int> indentSpaces) const
    {
        std::ofstream file;
        file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
        file.open(outputPath, std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to write to JSON file: " + outputPath.string());
        }
        file << root()->dump(indentSpaces.value_or(-1));
        file.close();
    }

    /// @brief Builds or rebuilds the entity map using IDs already collected by validation::schemaValidate.
    /// @param ids Snapshot from this document, current for its IDs, locations and types.
    /// @param policies Controls construction of supplemental mappings such as ottavas.
    /// @param errorHandler Optional handler for duplicate keys.
    /// @throws util::mapping_error on duplicate keys when no handler is supplied.
    void buildEntityMapFromIndex(const DocumentIdIndex& ids, EntityMapPolicies policies = {},
                                 const std::optional<ErrorHandler>& errorHandler = std::nullopt);

    /// @brief Gets a reference to the entity mapping instance for the document.
    [[nodiscard]] const util::EntityMap& getEntityMap() const
    {
        MNX_ASSERT_IF(!m_entityMapping) {
            throw std::logic_error("Call buildEntityMapFromIndex or assignId before calling getEntityMap.");
        }
        return *m_entityMapping;
    }

    /// @brief Returns whether an entity mapping currently exists
    [[nodiscard]] bool hasEntityMap() const { return static_cast<bool>(m_entityMapping); }

    /// @brief Sets the id of @p object and adds it to the entity mapping, creating the mapping if there is none.
    ///
    /// This is the only way to set an id through the API, so every id set while building a document is mapped and
    /// its objects can be looked up with #getEntityMap before the document is validated. The object is mapped as type @p T, so pass it as its concrete type.
    /// The mapping records the object's location, which stays valid only while no array that contains the
    /// object has elements inserted or removed ahead of it. Appending to arrays is safe.
    /// @tparam T The object's type. It must carry the MNX global attributes and name its schema type.
    /// @param object The object whose id is set.
    /// @param id The id to set.
    /// @throws util::mapping_error if @p id is already mapped. The object is then left unchanged.
    /// @throws std::invalid_argument if @p object belongs to a different document.
    template <typename T>
    void assignId(T& object, const std::string& id);

    /// @brief Finds a layout that matches the canonical full score layout, where each part staff
    /// appears in order on a single corresponding layout staff.
    /// @return The layout if found or std::nullopt if not.
    [[nodiscard]] std::optional<Layout> findFullScoreLayout() const;

    inline static constexpr std::string_view JsonSchemaTypeName = "root";     ///< required for mapping
};

static_assert(std::is_move_constructible<mnx::Document>::value, "Document must be move constructible");
} // namespace mnx
