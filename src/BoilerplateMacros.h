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

#ifndef DOXYGEN_SHOULD_IGNORE_THIS
// Preprocessor helpers to build child macro overloads and parameter lists.
#define MNX_PP_CAT(a, b) MNX_PP_CAT_I(a, b)
#define MNX_PP_CAT_I(a, b) a##b

#define MNX_PP_NARG(...) MNX_PP_NARG_I(__VA_ARGS__, MNX_PP_RSEQ_N(), 0)
#define MNX_PP_NARG_I(...) MNX_PP_ARG_N(__VA_ARGS__)
#define MNX_PP_ARG_N( \
    _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, N, ...) N
#define MNX_PP_RSEQ_N() 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1

#define MNX_FOR_EACH_COMMA(m, ...) MNX_PP_CAT(MNX_FOR_EACH_COMMA_, MNX_PP_NARG(__VA_ARGS__))(m, __VA_ARGS__)
#define MNX_FOR_EACH_COMMA_1(m, x) m(x)
#define MNX_FOR_EACH_COMMA_2(m, x, ...) m(x), MNX_FOR_EACH_COMMA_1(m, __VA_ARGS__)
#define MNX_FOR_EACH_COMMA_3(m, x, ...) m(x), MNX_FOR_EACH_COMMA_2(m, __VA_ARGS__)
#define MNX_FOR_EACH_COMMA_4(m, x, ...) m(x), MNX_FOR_EACH_COMMA_3(m, __VA_ARGS__)
#define MNX_FOR_EACH_COMMA_5(m, x, ...) m(x), MNX_FOR_EACH_COMMA_4(m, __VA_ARGS__)
#define MNX_FOR_EACH_COMMA_6(m, x, ...) m(x), MNX_FOR_EACH_COMMA_5(m, __VA_ARGS__)
#define MNX_FOR_EACH_COMMA_7(m, x, ...) m(x), MNX_FOR_EACH_COMMA_6(m, __VA_ARGS__)
#define MNX_FOR_EACH_COMMA_8(m, x, ...) m(x), MNX_FOR_EACH_COMMA_7(m, __VA_ARGS__)
#define MNX_FOR_EACH_COMMA_9(m, x, ...) m(x), MNX_FOR_EACH_COMMA_8(m, __VA_ARGS__)
#define MNX_FOR_EACH_COMMA_10(m, x, ...) m(x), MNX_FOR_EACH_COMMA_9(m, __VA_ARGS__)
#define MNX_FOR_EACH_COMMA_11(m, x, ...) m(x), MNX_FOR_EACH_COMMA_10(m, __VA_ARGS__)
#define MNX_FOR_EACH_COMMA_12(m, x, ...) m(x), MNX_FOR_EACH_COMMA_11(m, __VA_ARGS__)
#define MNX_FOR_EACH_COMMA_13(m, x, ...) m(x), MNX_FOR_EACH_COMMA_12(m, __VA_ARGS__)
#define MNX_FOR_EACH_COMMA_14(m, x, ...) m(x), MNX_FOR_EACH_COMMA_13(m, __VA_ARGS__)
#define MNX_FOR_EACH_COMMA_15(m, x, ...) m(x), MNX_FOR_EACH_COMMA_14(m, __VA_ARGS__)
#define MNX_FOR_EACH_COMMA_16(m, x, ...) m(x), MNX_FOR_EACH_COMMA_15(m, __VA_ARGS__)

#define MNX_FIELD_TYPE(pair) MNX_FIELD_TYPE_I pair
#define MNX_FIELD_TYPE_I(type, name) type

#define MNX_FIELD_NAME(pair) MNX_FIELD_NAME_I pair
#define MNX_FIELD_NAME_I(type, name) name

#define MNX_FIELD_PARAM(pair) MNX_FIELD_TYPE(pair) MNX_FIELD_NAME(pair)
#define MNX_FIELDS_PARAMS(...) MNX_FOR_EACH_COMMA(MNX_FIELD_PARAM, __VA_ARGS__)
#define MNX_FIELDS_NAMES(...) MNX_FOR_EACH_COMMA(MNX_FIELD_NAME, __VA_ARGS__)
#endif // DOXYGEN_SHOULD_IGNORE_THIS

/**
 * @brief creates a required property with a simple type
 *
 * It creates the following class methods.
 *
 * - `NAME()` returns the value of the property.
 * - `set_NAME(value) sets the value of the property.
 *
 * @param TYPE the type of the property
 * @param NAME the name of the property (no quotes)
 */
#define MNX_REQUIRED_PROPERTY(TYPE, NAME) \
    [[nodiscard]] TYPE NAME() const { \
        if (!jsonRef().contains(#NAME)) { \
            throw std::runtime_error("Missing required property: " #NAME); \
        } \
        return jsonRef()[#NAME].get<TYPE>(); \
    } \
    void set_##NAME(const TYPE& value) { jsonRef()[#NAME] = value; } \
    static_assert(true, "") // require semicolon after macro

/**
 * @brief creates a property (with a simple type) that occupies a fixed position in an array
 *
 * It creates the following class methods.
 *
 * - `NAME()` returns the value of the property.
 * - `set_NAME(value) sets the value of the property.
 *
 * @param TYPE the type of the property
 * @param NAME the name of the property (no quotes)
 * @param INDEX the index in the array for the property
 */
#define MNX_ARRAY_ELEMENT_PROPERTY(TYPE, NAME, INDEX) \
    static_assert(std::is_integral_v<decltype(INDEX)>, "array index must be an integer type"); \
    [[nodiscard]] TYPE NAME() const { return (*this)[INDEX]; } \
    void set_##NAME(const TYPE& value) { (*this)[INDEX] = value; } \
    static_assert(true, "") // require semicolon after macro

/**
 * @brief creates an optional named property with a simple type. This is a property whose name is different than
 * its JSON key. This is used when the JSON property name is a C++ keyword. An example is `class` that becomes a
 * property called `styleClass`.
 *
 * It creates the following class methods.
 *
 * - `NAME()` returns a std::optional<TYPE> containing the value of the property.
 * - `NAME_or(value)` returns the property value if it exists or the input value if not.
 * - `set_NAME(value) sets the value of the property.
 * - `clear_NAME() clears the property from the JSON document.
 *
 * @param TYPE the type of the property
 * @param NAME the name of the property (no quotes)
 * @param KEY the JSON key of the property (with quotes)
 */
#define MNX_OPTIONAL_NAMED_PROPERTY(TYPE, NAME, KEY) \
    [[nodiscard]] std::optional<TYPE> NAME() const { \
        return jsonRef().contains(KEY) ? std::optional<TYPE>(jsonRef()[KEY].get<TYPE>()) : std::nullopt; \
    } \
    [[nodiscard]] TYPE NAME##_or(const TYPE& defaultVal) const { \
        return jsonRef().contains(KEY) ? jsonRef()[KEY].get<TYPE>() : defaultVal; \
    } \
    void set_##NAME(const TYPE& value) { jsonRef()[KEY] = value; } \
    void clear_##NAME() { jsonRef().erase(KEY); } \
    static_assert(true, "") // require semicolon after macro

/**
 * @brief creates an optional property with a simple type that can be read but not set through the property.
 *
 * Use it for a property whose value must stay consistent with state outside the JSON, so that only the code
 * that maintains that state writes it. For example, `id` is set by #mnx::Document::assignId, which also maps it.
 *
 * It creates the following class methods.
 *
 * - `NAME()` returns a std::optional<TYPE> containing the value of the property.
 * - `NAME_or(value)` returns the property value if it exists or the input value if not.
 *
 * @param TYPE the type of the property
 * @param NAME the name of the property (no quotes)
 */
#define MNX_READ_ONLY_OPTIONAL_PROPERTY(TYPE, NAME) \
    [[nodiscard]] std::optional<TYPE> NAME() const { \
        return jsonRef().contains(#NAME) ? std::optional<TYPE>(jsonRef()[#NAME].get<TYPE>()) : std::nullopt; \
    } \
    [[nodiscard]] TYPE NAME##_or(const TYPE& defaultVal) const { \
        return jsonRef().contains(#NAME) ? jsonRef()[#NAME].get<TYPE>() : defaultVal; \
    } \
    static_assert(true, "") // require semicolon after macro

/**
 * @brief creates an optional property with a simple type.
 *
 * It creates the following class methods.
 *
 * - `NAME()` returns a std::optional<TYPE> containing the value of the property.
 * - `NAME_or(value)` returns the property value if it exists or the input value if not.
 * - `set_NAME(value) sets the value of the property.
 * - `clear_NAME() clears the property from the JSON document.
 *
 * @param TYPE the type of the property
 * @param NAME the name of the property (no quotes)
 */
#define MNX_OPTIONAL_PROPERTY(TYPE, NAME) MNX_OPTIONAL_NAMED_PROPERTY(TYPE, NAME, #NAME)

/**
 * @brief creates an optional property with a default value.
 *
 * It has the following class methods.
 *
 * - `NAME()` returns the value of the property.
 * - `set_NAME(value) sets the value of the property.
 * - `clear_NAME() clears the property from the JSON document.
 * - `set_or_clear_NAME(value) sets the value of the property if the input value is not the default.
 * Otherwise it clears the property.
 *
 * @param TYPE the type of the property
 * @param NAME the name of the property (no quotes)
 * @param DEFAULT the default value of the property.
 */
#define MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(TYPE, NAME, DEFAULT) \
    [[nodiscard]] TYPE NAME() const { \
        return jsonRef().contains(#NAME) ? jsonRef()[#NAME].template get<TYPE>() : DEFAULT; \
    } \
    void set_##NAME(const TYPE& value) { jsonRef()[#NAME] = value; } \
    void clear_##NAME() { jsonRef().erase(#NAME); } \
    void set_or_clear_##NAME(const TYPE& value) { \
        if (value == DEFAULT) clear_##NAME(); \
        else set_##NAME(value); \
    } \
    static_assert(true, "") // require semicolon after macro

/**
 * @brief creates a required child object or array
 *
 * It creates the following class methods.
 *
 * - `NAME()` returns the child.
 * - `set_NAME(value)` sets the child from an existing object or array.
 * - `create_NAME(...)` creates the child from constructor parameters (variadic form only).
 * - `create_NAME()` creates the child with a default constructor (non-variadic form only).
 *
 * For classes that define `Required`, pass the constructor parameters directly.
 *
 * Example (IntelliSense):
 * `create_pitch(NoteStep::C, 4, 0);`
 *
 * @param TYPE the type of the child object or array
 * @param NAME the name of the child object or array (no quotes)
 */
#define MNX_REQUIRED_CHILD(...) \
    MNX_PP_CAT(MNX_REQUIRED_CHILD_, MNX_PP_NARG(__VA_ARGS__))(__VA_ARGS__)

#ifndef DOXYGEN_SHOULD_IGNORE_THIS

#define MNX_REQUIRED_CHILD_CORE(TYPE, NAME, CREATE_BODY) \
    [[nodiscard]] TYPE NAME() const { return getChild<TYPE>(#NAME); } \
    TYPE set_##NAME(const TYPE& value) { return setChild(#NAME, value); } \
    CREATE_BODY \
    static_assert(true, "") // require semicolon after macro

#define MNX_REQUIRED_CHILD_2(TYPE, NAME) \
    MNX_REQUIRED_CHILD_CORE(TYPE, NAME, \
        TYPE create_##NAME() { return setChild(#NAME, TYPE(*this, #NAME)); } \
    )

#define MNX_REQUIRED_CHILD_3(TYPE, NAME, ...) \
    MNX_REQUIRED_CHILD_CORE(TYPE, NAME, \
        TYPE create_##NAME(MNX_FIELDS_PARAMS(__VA_ARGS__)) { \
            return setChild(#NAME, TYPE(*this, #NAME, MNX_FIELDS_NAMES(__VA_ARGS__))); \
        } \
    )

#define MNX_REQUIRED_CHILD_4(TYPE, NAME, ...) MNX_REQUIRED_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_REQUIRED_CHILD_5(TYPE, NAME, ...) MNX_REQUIRED_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_REQUIRED_CHILD_6(TYPE, NAME, ...) MNX_REQUIRED_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_REQUIRED_CHILD_7(TYPE, NAME, ...) MNX_REQUIRED_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_REQUIRED_CHILD_8(TYPE, NAME, ...) MNX_REQUIRED_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_REQUIRED_CHILD_9(TYPE, NAME, ...) MNX_REQUIRED_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_REQUIRED_CHILD_10(TYPE, NAME, ...) MNX_REQUIRED_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_REQUIRED_CHILD_11(TYPE, NAME, ...) MNX_REQUIRED_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_REQUIRED_CHILD_12(TYPE, NAME, ...) MNX_REQUIRED_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_REQUIRED_CHILD_13(TYPE, NAME, ...) MNX_REQUIRED_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_REQUIRED_CHILD_14(TYPE, NAME, ...) MNX_REQUIRED_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_REQUIRED_CHILD_15(TYPE, NAME, ...) MNX_REQUIRED_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_REQUIRED_CHILD_16(TYPE, NAME, ...) MNX_REQUIRED_CHILD_3(TYPE, NAME, __VA_ARGS__)

#endif // DOXYGEN_SHOULD_IGNORE_THIS

/**
 * @brief creates an optional child object or array
 *
 * It creates the following class methods.
 *
 * - `NAME()` returns a std::optional<TYPE> containing the child or std::nullopt if none.
 * - `set_NAME(value)` sets the child from an existing object or array.
 * - `ensure_NAME(...)` if the child does not exist, creates it from constructor parameters (variadic form only).
 * - `ensure_NAME()` if the child does not exist, creates it with a default constructor (non-variadic form only).
 * - `clear_NAME()` clears the child from JSON document.
 *
 * For classes that define `Required`, pass the constructor parameters directly.
 *
 * Example (IntelliSense):
 * `ensure_duration(NoteValueBase::Quarter, 1);`
 *
 * @param TYPE the type of the child object or array
 * @param NAME the name of the child object or array (no quotes)
 */
#define MNX_OPTIONAL_CHILD(...) \
    MNX_PP_CAT(MNX_OPTIONAL_CHILD_, MNX_PP_NARG(__VA_ARGS__))(__VA_ARGS__)

#ifndef DOXYGEN_SHOULD_IGNORE_THIS

#define MNX_OPTIONAL_CHILD_CORE(TYPE, NAME, ENSURE_BODY) \
    [[nodiscard]] std::optional<TYPE> NAME() const { return getOptionalChild<TYPE>(#NAME); } \
    TYPE set_##NAME(const TYPE& value) { return setChild(#NAME, value); } \
    ENSURE_BODY \
    void clear_##NAME() { jsonRef().erase(#NAME); } \
    static_assert(true, "") // require semicolon after macro

#define MNX_OPTIONAL_CHILD_2(TYPE, NAME) \
    MNX_OPTIONAL_CHILD_CORE(TYPE, NAME, \
        TYPE ensure_##NAME() { \
            if (auto child = getOptionalChild<TYPE>(#NAME)) return child.value(); \
            return setChild(#NAME, TYPE(*this, #NAME)); \
        } \
    )

#define MNX_OPTIONAL_CHILD_3(TYPE, NAME, ...) \
    MNX_OPTIONAL_CHILD_CORE(TYPE, NAME, \
        TYPE ensure_##NAME(MNX_FIELDS_PARAMS(__VA_ARGS__)) { \
            if (auto child = getOptionalChild<TYPE>(#NAME)) return child.value(); \
            return setChild(#NAME, TYPE(*this, #NAME, MNX_FIELDS_NAMES(__VA_ARGS__))); \
        } \
    )

#define MNX_OPTIONAL_CHILD_4(TYPE, NAME, ...) MNX_OPTIONAL_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_OPTIONAL_CHILD_5(TYPE, NAME, ...) MNX_OPTIONAL_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_OPTIONAL_CHILD_6(TYPE, NAME, ...) MNX_OPTIONAL_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_OPTIONAL_CHILD_7(TYPE, NAME, ...) MNX_OPTIONAL_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_OPTIONAL_CHILD_8(TYPE, NAME, ...) MNX_OPTIONAL_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_OPTIONAL_CHILD_9(TYPE, NAME, ...) MNX_OPTIONAL_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_OPTIONAL_CHILD_10(TYPE, NAME, ...) MNX_OPTIONAL_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_OPTIONAL_CHILD_11(TYPE, NAME, ...) MNX_OPTIONAL_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_OPTIONAL_CHILD_12(TYPE, NAME, ...) MNX_OPTIONAL_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_OPTIONAL_CHILD_13(TYPE, NAME, ...) MNX_OPTIONAL_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_OPTIONAL_CHILD_14(TYPE, NAME, ...) MNX_OPTIONAL_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_OPTIONAL_CHILD_15(TYPE, NAME, ...) MNX_OPTIONAL_CHILD_3(TYPE, NAME, __VA_ARGS__)
#define MNX_OPTIONAL_CHILD_16(TYPE, NAME, ...) MNX_OPTIONAL_CHILD_3(TYPE, NAME, __VA_ARGS__)

#endif // DOXYGEN_SHOULD_IGNORE_THIS

#define MNX_ASSERT_IF(TEST) \
assert(!(TEST)); \
if (TEST)
