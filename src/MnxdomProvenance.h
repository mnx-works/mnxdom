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

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string_view>

#include "BaseTypes.h"

namespace mnx {

/// @brief Key used for the mnxdom provenance payload inside `_x`.
inline constexpr std::string_view MNXDOM_PROVENANCE_KEY = "mnxdom";
/// @brief Version of the provenance payload schema used under `_x.mnxdom`.
inline constexpr int MNXDOM_PROVENANCE_SCHEMA_VERSION = 1;

namespace detail {

/// @brief Formats the current time as an RFC 3339 UTC timestamp.
[[nodiscard]] inline std::string makeUtcTimestamp()
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t nowTime = std::chrono::system_clock::to_time_t(now);
    std::tm utcTime {};
#if defined(_WIN32)
    gmtime_s(&utcTime, &nowTime);
#else
    gmtime_r(&nowTime, &utcTime);
#endif

    std::ostringstream out;
    out << std::put_time(&utcTime, "%Y-%m-%dT%H:%M:%SZ");
    return out.str();
}

} // namespace detail

/**
 * @class MnxdomGeneratorInfo
 * @brief Identifies the software component that wrote the MNX JSON payload.
 */
class MnxdomGeneratorInfo : public Object
{
public:
    using Object::Object;

    MnxdomGeneratorInfo(Base& parent, std::string_view key);

    MNX_REQUIRED_PROPERTY(std::string, version);     ///< The version of the writing component.
    MNX_OPTIONAL_PROPERTY(std::string, build);       ///< Optional build number or build identifier.
    MNX_OPTIONAL_PROPERTY(std::string, commit);      ///< Optional source control revision for the writer.
};

/**
 * @class MnxdomClientInfo
 * @brief Identifies the client application that requested the export.
 */
class MnxdomClientInfo : public Object
{
public:
    using Object::Object;

    MNX_REQUIRED_PROPERTY(std::string, name);        ///< The application name.
    MNX_OPTIONAL_PROPERTY(std::string, version);     ///< The application version.
    MNX_OPTIONAL_PROPERTY(std::string, build);       ///< Optional build number or build identifier.
    MNX_OPTIONAL_PROPERTY(std::string, commit);      ///< Optional source control revision for the client.
};

/**
 * @class MnxdomSourceInfo
 * @brief Describes the source material that was converted or imported.
 */
class MnxdomSourceInfo : public Object
{
public:
    using Object::Object;

    MNX_REQUIRED_PROPERTY(std::string, format);     ///< The source format, such as `musicxml`, `midi`, or `manual`.
    MNX_OPTIONAL_PROPERTY(std::string, version);    ///< Optional source format version.
    MNX_OPTIONAL_PROPERTY(std::string, filename);   ///< Optional original source URI or path.
};

/**
 * @class MnxdomExtensionData
 * @brief Provenance data stored under `_x.mnxdom`.
 *
 * This is intentionally small and stable. It records who wrote the document,
 * which client requested the export, and what source material was used.
 */
class MnxdomExtensionData : public Object
{
public:
    using Object::Object;

    MnxdomExtensionData(Base& parent, std::string_view key)
        : Object(parent, key)
    {
        set_provenanceSchemaVersion(MNXDOM_PROVENANCE_SCHEMA_VERSION);
        set_mnxSchema(getMnxSchemaId());
        create_mnxdom();
        set_createdAt(detail::makeUtcTimestamp());
    }

    MNX_REQUIRED_PROPERTY(int, provenanceSchemaVersion);       ///< Schema version for the `_x.mnxdom` payload.
    MNX_REQUIRED_PROPERTY(std::string, mnxSchema);             ///< Full MNX schema identifier used by the document.
    MNX_REQUIRED_CHILD(MnxdomGeneratorInfo, mnxdom);            ///< The component that wrote the JSON.
    MNX_OPTIONAL_CHILD(MnxdomClientInfo, client);               ///< The client application that requested the export.
    MNX_OPTIONAL_CHILD(MnxdomSourceInfo, source);               ///< The imported or converted source.
    MNX_REQUIRED_PROPERTY(std::string, createdAt);              ///< UTC timestamp when the payload was created.
    MNX_OPTIONAL_PROPERTY(std::string, modifiedAt);             ///< Optional UTC timestamp of the most recent write.
    MNX_OPTIONAL_PROPERTY(std::string, documentId);             ///< Optional stable document identifier, such as a UUID URN.
};

} // namespace mnx
