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
#include "CommonClasses.h"

namespace mnx {

/**
 * @namespace mnx::score
 * @brief classes related to the root scores array
 */
namespace score {

/**
 * @class MultimeasureRest
 * @brief Represents a multimeasure rest in a score.
 */
class MultimeasureRest : public ArrayElementObject
{
public:
    /// @brief initializer class for #MultimeasureRest
    struct Required
    {
        std::string startMeasure{}; ///< the measure index of the first measure in the multimeasure rest
        int numMeasures{};          ///< the number of measures in the multimeasure rest
    };

    /// @brief Constructor for existing mm rests
    MultimeasureRest(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }
    
    /// @brief Creates a new MultimeasureRest class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param startMeasure The global measure id of the first measure in the multimeasure rest
    /// @param numMeasures The number of measures in the multimeasure rest
    MultimeasureRest(Base& parent, std::string_view key, const std::string& startMeasure, int numMeasures)
        : ArrayElementObject(parent, key)
    {
        set_start(startMeasure);
        set_duration(numMeasures);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { start(), duration() }; }

    /// @brief Create a Required instance for #MultimeasureRest.
    static Required make(const std::string& startMeasure, int numMeasures) { return { startMeasure, numMeasures }; }

    MNX_REQUIRED_PROPERTY(int, duration);           ///< the number of measures in the multimeasure rest
    MNX_OPTIONAL_PROPERTY(std::string, label);      ///< the label to place on the multimeasure rest, if provided.
    MNX_REQUIRED_PROPERTY(std::string, start);      ///< the global measure id of the start measure of the multimeasure rest

    inline static constexpr std::string_view JsonSchemaTypeName = "multimeasure-rest";     ///< required for mapping
};

/**
 * @class LayoutChange
 * @brief Represents a system layout change in a score
 */
class LayoutChange : public ArrayElementObject
{
public:
    /// @brief initializer class for #LayoutChange
    struct Required
    {
        std::string layoutId;     ///< the id of the layout to use for the layout change
        std::string measureId;    ///< the global measure id of the measure of the position
        FractionValue position{}; ///< the position of the LayoutChange within the measure
    };

    /// @brief Constructor for existing system layouts
    LayoutChange(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }
    
    /// @brief Creates a new LayoutChange class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param layoutId The id of the layout to use for the layout change
    /// @param location The location where the layout change takes effect
    LayoutChange(Base& parent, std::string_view key, const std::string& layoutId, const MeasureRhythmicPosition::Required& location)
        : ArrayElementObject(parent, key)
    {
        set_layout(layoutId);
        create_location(location.measureId, location.position);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { layout(), location().measure(), location().position().fraction() }; }

    /// @brief Create a Required instance for #LayoutChange.
    static Required make(const std::string& layoutId, const MeasureRhythmicPosition::Required& location)
    { return { layoutId, location.measureId, location.position }; }

    MNX_REQUIRED_PROPERTY(std::string, layout);             ///< Layout id, referring to an element in the root-level layouts array.
    MNX_REQUIRED_CHILD(MeasureRhythmicPosition, location,
        (const std::string&, measureId), (const FractionValue&, position)); ///< location where the new layout takes effect.

    inline static constexpr std::string_view JsonSchemaTypeName = "layout-change";     ///< required for mapping
};

/**
 * @class System
 * @brief Represents a system on a page in a score.
 */
class System : public ArrayElementObject
{
public:
    /// @brief initializer class for #System
    struct Required
    {
        std::string startMeasure{}; ///< the measure index of the first measure in the system
    };

    /// @brief Constructor for existing system layouts
    System(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }
    
    /// @brief Creates a new System class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param startMeasure The measure index of the first measure in the system
    System(Base& parent, std::string_view key, const std::string& startMeasure)
        : ArrayElementObject(parent, key)
    {
        set_measure(startMeasure);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { measure() }; }

    /// @brief Create a Required instance for #System.
    static Required make(const std::string& startMeasure) { return { startMeasure }; }

    MNX_OPTIONAL_PROPERTY(std::string, layout);     ///< Layout id, referring to an element in the root-level layouts array.
    MNX_OPTIONAL_CHILD(Array<LayoutChange>, layoutChanges); ///< layout changes in the system (e.g., for changes in stem direction)
    MNX_REQUIRED_PROPERTY(std::string, measure);    ///< The global measure if of the first measure in the system

    inline static constexpr std::string_view JsonSchemaTypeName = "system";     ///< required for mapping
};

/**
 * @class Page
 * @brief Represents a page in a score.
 */
class Page : public ArrayElementObject
{
public:
    using ArrayElementObject::ArrayElementObject;

    /// @brief Creates a new Page class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    Page(Base& parent, std::string_view key)
        : ArrayElementObject(parent, key)
    {
        // required children
        create_systems();
    }

    MNX_OPTIONAL_PROPERTY(std::string, layout);     ///< Layout id, referring to an element in the root-level layouts array.
    MNX_REQUIRED_CHILD(Array<System>, systems);     ///< and array systems on the page.

    inline static constexpr std::string_view JsonSchemaTypeName = "page";     ///< required for mapping
};

} // namespace score

/**
 * @class Score
 * @brief Represents an element in the scores section of an MNX document.
 */
class Score : public ArrayElementObject
{
public:
    /// @brief initializer class for #Score
    struct Required
    {
        std::string scoreName; ///< the name of the score to be created
    };

    /// @brief Constructor for existing system layouts
    Score(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayElementObject(root, pointer)
    {
    }
    
    /// @brief Creates a new Score class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param scoreName The name of the score to be created
    Score(Base& parent, std::string_view key, const std::string& scoreName)
        : ArrayElementObject(parent, key)
    {
        set_name(scoreName);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { name() }; }

    /// @brief Create a Required instance for #Score.
    static Required make(const std::string& scoreName) { return { scoreName }; }

    MNX_OPTIONAL_PROPERTY(std::string, layout);                             ///< Layout id, referring to an element in the root-level layouts array.
    MNX_OPTIONAL_CHILD(Array<score::MultimeasureRest>, multimeasureRests);  ///< List of multimeasure rests in the score.
    MNX_REQUIRED_PROPERTY(std::string, name);                               ///< Required name, such as "Flute 1" or "Full Score".
    MNX_OPTIONAL_CHILD(Array<score::Page>, pages);                          ///< An optional list of pages.
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(bool, useWritten, false);            ///< If this value is true, the score should be displayed transposed.

    inline static constexpr std::string_view JsonSchemaTypeName = "score";     ///< required for mapping
};

} // namespace mnx
