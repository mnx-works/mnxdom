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

#include <utility>
#include <numeric>
#include <stdexcept>
#include <limits>
#include <type_traits>

#include "BaseTypes.h"
#include "BoilerplateMacros.h"
#include "Enumerations.h"

namespace mnx {

/**
 * @class Fermata
 * @brief Defines a fermata.
 */
class Fermata : public Object
{
public:
    using Object::Object;

    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(FermataDuration, duration, FermataDuration::Auto);       ///< subjective playback duration of the fermata
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(Orientation, orient, Orientation::Auto);                 ///< vertical placement of the fermata
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(MarkingUpDownAuto, pointing, MarkingUpDownAuto::Auto);   ///< direction of the fermata symbol
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(FermataSymbol, symbol, FermataSymbol::Normal);           ///< the style of symbol for the fermata

    inline static constexpr std::string_view JsonSchemaTypeName = "fermata";     ///< required for mapping
};

/**
 * @class FractionValue
 * @brief Represents a detached arithmetic fraction with normalization.
 *
 * This class provides arithmetic operations on rational numbers without
 * attaching them to the JSON DOM. It is intended for musical-duration and
 * proportional calculations where a simple fraction is needed independently
 * of the MNX DOM structure.
 *
 * A FractionValue constructed from a numerator and denominator preserves the
 * values exactly as provided. Arithmetic operations (+=, -=, *=, /=) reduce
 * the result to lowest terms using std::gcd. Call reduce() explicitly if
 * you need a reduced form immediately after construction.
 *
 * The class supports:
 *  - Exact rational arithmetic (addition, subtraction, multiplication, division)
 *  - Normalization to lowest terms after each operation
 *  - Comparison operators (in helper section)
 *
 * Compound assignment operators (+=, -=, *=, /=) are implemented as members.
 * Binary operators (+, -, *, /) are implemented as non-member functions to
 * allow symmetric conversions and correct C++ arithmetic semantics.
 */
struct [[nodiscard]] FractionValue
{
    /// @brief Unsigned integer type used for numerator and denominator.
    using NumType = unsigned;
    static_assert(std::is_integral<NumType>::value, "FractionValue::NumType must be an integral type");

private:
    NumType m_num = 0; ///< Numerator of the fraction (always non-negative).
    NumType m_den = 1; ///< Denominator of the fraction (must be > 0).

    template<typename T = NumType,
             typename std::enable_if_t<std::is_signed<T>::value, int> = 0>
    constexpr void normalizeSign()
    {
        if (m_den < 0) {
            m_den = -m_den;
            m_num = -m_num;
        }
    }

    // Unsigned case: no-op, never even sees m_den < 0
    template<typename T = NumType,
             typename std::enable_if_t<!std::is_signed<T>::value, int> = 0>
    constexpr void normalizeSign() {}

public:
    /**
     * @brief Default constructor initializes the value to 0/1.
     */
    constexpr FractionValue() = default;

    /**
     * @brief Constructs a fraction from a numerator and denominator.
     *
     * The fraction is not automatically reduced. Use reduce() if you need
     * the value in lowest terms. However, fractions *are* automatically reduced
     * after arithmetical operations.
     *
     * @param num The numerator.
     * @param den The denominator. Must not be zero.
     * @throws std::invalid_argument if @p den is zero.
     * @todo Make this constructor constexpr when we drop C++17 support.
     */
    constexpr FractionValue(NumType num, NumType den)
        : m_num(num), m_den(den)
    {
        if (m_den == 0) {
            throw std::invalid_argument("FractionValue: denominator must not be zero.");
        }
        normalizeSign();
    }

    /**
     * @brief Constructs a Fraction object from an integer.
     * @param value The integer value of the fraction.
     * @throws std::invalid_argument if the m_denominator is zero.
     */
    constexpr FractionValue(NumType value) : m_num(value), m_den(1) {}

    /// @brief Returns the numerator.
    [[nodiscard]] constexpr NumType numerator() const noexcept { return m_num; }

    /// @brief Returns the denominator.
    [[nodiscard]] constexpr NumType denominator() const noexcept { return m_den; }

    /**
     * @brief Returns the integer (whole number) part of the fraction.
     * @return The integer part of the fraction.
     */
    [[nodiscard]] constexpr NumType quotient() const
    {
        return m_num / m_den;
    }

    /**
     * @brief Returns the fractional part of the fraction.
     * @return The remainder as a fraction, satisfying -1 < remainder < 1.
     */
    constexpr FractionValue remainder() const
    {
        FractionValue result;
        result.m_num = m_num % m_den;
        result.m_den = m_den;
        return result;
    }

    /// @brief Constructs the max fractional value.
    static constexpr FractionValue max() noexcept
    {
        return FractionValue((std::numeric_limits<NumType>::max)());
    }

    /// @brief Converts the fraction to floating point double.
    [[nodiscard]] constexpr double toDouble() const {
        return double(m_num) / double(m_den);
    }

    /**
     * @brief Adds another FractionValue to this one.
     *
     * @param rhs The fraction to add.
     * @return Reference to this object after modification.
     *
     * The result is normalized.
     */
    constexpr FractionValue& operator+=(const FractionValue& rhs)
    {
        // a/b + c/d = (ad + bc)/bd
        m_num = m_num * rhs.m_den + rhs.m_num * m_den;
        m_den = m_den * rhs.m_den;
        reduce();
        return *this;
    }

    /**
     * @brief Subtracts another FractionValue from this one.
     *
     * @param rhs The fraction to subtract.
     * @return Reference to this object after modification.
     *
     * The result is normalized.
     */
    constexpr FractionValue& operator-=(const FractionValue& rhs)
    {
        // a/b - c/d = (ad - bc)/bd
        m_num = m_num * rhs.m_den - rhs.m_num * m_den;
        m_den = m_den * rhs.m_den;
        reduce();
        return *this;
    }

    /**
     * @brief Multiplies this fraction by another.
     *
     * @param rhs The fraction to multiply with.
     * @return Reference to this object after modification.
     *
     * The result is normalized.
     */
    constexpr FractionValue& operator*=(const FractionValue& rhs)
    {
        m_num = m_num * rhs.m_num;
        m_den = m_den * rhs.m_den;
        reduce();
        return *this;
    }

    /**
     * @brief Divides this fraction by another.
     *
     * @param rhs The divisor.
     * @return Reference to this object after modification.
     *
     * @throws std::invalid_argument if @p rhs has a numerator of zero.
     *
     * The result is normalized.
     */
    constexpr FractionValue& operator/=(const FractionValue& rhs)
    {
        if (rhs.m_num == 0) {
            throw std::invalid_argument("Division by zero FractionValue.");
        }
        m_num = m_num * rhs.m_den;
        m_den = m_den * rhs.m_num;
        reduce();
        return *this;
    }

    /**
     * @brief Reduces the fraction to lowest terms using std::gcd.
     */
    constexpr void reduce()
    {
        const NumType g = std::gcd(m_num, m_den);
        if (g > 1) {
            m_num /= g;
            m_den /= g;
        }
        normalizeSign();
    }

    /**
     * @brief Returns a copy of the fraction reduced to lowest terms using std::gcd.
     */
    constexpr FractionValue reduced() const
    {
        auto result = *this;
        result.reduce();
        return result;
    }

    /**
     * @brief Attempts to express this fraction with the given denominator.
     *
     * If the current value can be written exactly with @p targetDenominator
     * using an integer numerator, the numerator and denominator are updated
     * to use that denominator and the function returns true. Otherwise, the
     * fraction is left unchanged and the function returns false.
     *
     * This function does not require the fraction to be in reduced form. It
     * computes a reduced logical form internally when determining whether
     * the requested denominator is compatible.
     *
     * Examples:
     * - 1/1 expressed with denominator 4 becomes 4/4 and returns true.
     * - 3/2 expressed with denominator 4 becomes 6/4 and returns true.
     * - 2/6 (i.e. 1/3) expressed with denominator 3 becomes 1/3 and returns true.
     * - 1/3 expressed with denominator 4 cannot be represented exactly with
     *   an integer numerator, so the call returns false and the fraction
     *   remains 1/3.
     *
     * A zero fraction (0 / d) can always be expressed with any nonzero
     * @p targetDenominator; in that case the result becomes
     * 0 / targetDenominator.
     *
     * @param targetDenominator The desired denominator. If zero, the call
     *        fails and the fraction is left unchanged.
     * @return true if the fraction was successfully rewritten with the given
     *         denominator; false if no exact integer representation exists
     *         or if @p targetDenominator is zero.
     */
    constexpr bool expressWithDenominator(NumType targetDenominator)
    {
        if (targetDenominator == 0) {
            // Cannot express anything with denominator 0.
            return false;
        }

        // Zero fraction: 0/d is expressible as 0/targetDenominator.
        if (m_num == 0) {
            m_den = targetDenominator;
            return true;
        }

        // Reduce the fraction logically: m_num/m_den -> (numRed/denRed).
        // We don't actually have to store the reduced form; just use it to
        // check compatibility and compute the new numerator.
        const NumType g = std::gcd(m_num, m_den);
        const NumType denRed = m_den / g;
        const NumType numRed = m_num / g;   // sign preserved

        // For an integer representation with the requested denominator to exist
        // (while preserving the value), the reduced denominator must divide
        // targetDenominator exactly.
        if (targetDenominator % denRed != 0) {
            return false;  // No integer scaling factor exists.
        }

        const NumType factor = targetDenominator / denRed;

        // Apply the scaling to the reduced numerator and store the new form.
        m_num = numRed * factor;
        m_den = targetDenominator;

        normalizeSign();

        return true;
    }
};
#ifndef DOXYGEN_SHOULD_IGNORE_THIS

// ------------------------------------------------------------
// Non-member arithmetic operators
// ------------------------------------------------------------

[[nodiscard]] constexpr FractionValue operator+(FractionValue lhs, const FractionValue& rhs)
{
    lhs += rhs;
    return lhs;
}

[[nodiscard]] constexpr FractionValue operator-(FractionValue lhs, const FractionValue& rhs)
{
    lhs -= rhs;
    return lhs;
}

[[nodiscard]] constexpr FractionValue operator*(FractionValue lhs, const FractionValue& rhs)
{
    lhs *= rhs;
    return lhs;
}

[[nodiscard]] constexpr FractionValue operator/(FractionValue lhs, const FractionValue& rhs)
{
    lhs /= rhs;
    return lhs;
}

// ------------------------------------------------------------
// Comparison operators
// ------------------------------------------------------------

[[nodiscard]] constexpr bool operator==(const FractionValue& a, const FractionValue& b)
{
    return a.numerator() * b.denominator() == b.numerator() * a.denominator();
}

[[nodiscard]] constexpr bool operator!=(const FractionValue& a, const FractionValue& b)
{
    return !(a == b);
}

[[nodiscard]] constexpr bool operator<(const FractionValue& a, const FractionValue& b)
{
    return a.numerator() * b.denominator() < b.numerator() * a.denominator();
}

[[nodiscard]] constexpr bool operator<=(const FractionValue& a, const FractionValue& b)
{
    return !(b < a);
}

[[nodiscard]] constexpr bool operator>(const FractionValue& a, const FractionValue& b)
{
    return b < a;
}

[[nodiscard]] constexpr bool operator>=(const FractionValue& a, const FractionValue& b)
{
    return !(a < b);
}

#endif // DOXYGEN_SHOULD_IGNORE_THIS

/// @class Fraction
/// @brief Represents a fraction of a whole note, for measuring musical time.
class Fraction : private Array<unsigned>
{
private:
    using NumType = FractionValue::NumType;
    using ArrayType = Array<NumType>;

    static constexpr size_t NUMERATOR_INDEX = 0;
    static constexpr size_t DENOMINATOR_INDEX = 1;

public:    
    /// @brief Constructor to wrap a Fraction instance around existing JSON
    Fraction(const std::shared_ptr<json>& root, json_pointer pointer)
        : ArrayType(root, pointer)
    {
        if (size() != 2) {
            throw std::invalid_argument("mnx::Fraction must have exactly 2 elements.");
        }
    }

    /// @brief Creates a new Array class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param value The fraction value to use.
    Fraction(Base& parent, std::string_view key, const FractionValue& value)
        : ArrayType(parent, key)
    {
        push_back(value.numerator());
        push_back(value.denominator());
    }

    /// @brief Implicit conversion to @ref FractionValue for arithmetic and comparisons.
    [[nodiscard]] operator FractionValue() const
    {
        return FractionValue(numerator(), denominator());
    }

    MNX_ARRAY_ELEMENT_PROPERTY(NumType, numerator, NUMERATOR_INDEX);        ///< the numerator of the fraction
    MNX_ARRAY_ELEMENT_PROPERTY(NumType, denominator, DENOMINATOR_INDEX);    ///< the denominator of the fraction

    friend class Base;
};

/**
 * @class IdPair
 * @brief Represents a start and end id pair.
 */
class IdPair : public Object
{
public:
    /// @brief initializer class for #IdPair
    struct Required
    {
        std::string startId;    ///< the start id of the pair
        std::string endId;      ///< the end id of the pair
    };

    /// @brief Constructor for existing id pairs
    IdPair(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }
    
    /// @brief Creates a new IdPair class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param startId The start id.
    /// @param endId The end id.
    IdPair(Base& parent, std::string_view key, const std::string& startId, const std::string& endId)
        : Object(parent, key)
    {
        set_start(startId);
        set_end(endId);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { start(), end() }; }

    /// @brief Create a Required instance for #IdPair.
    static Required make(const std::string& startId, const std::string& endId) { return { startId, endId }; }

    MNX_REQUIRED_PROPERTY(std::string, start);          ///< the start id of the pair
    MNX_REQUIRED_PROPERTY(std::string, end);            ///< the end id of the pair
};

/**
 * @class RhythmicPosition
 * @brief Represents a system on a page in a score.
 */
class RhythmicPosition : public Object
{
public:
    /// @brief initializer class for #RhythmicPosition
    struct Required
    {
        FractionValue position{}; ///< position within a measure
    };

    /// @brief Constructor for existing rhythmic position instances
    RhythmicPosition(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }
    
    /// @brief Creates a new RhythmicPosition class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param position The rhythmic position value to use.
    RhythmicPosition(Base& parent, std::string_view key, const FractionValue& position)
        : Object(parent, key)
    {
        create_fraction(position);
    }
    
    /// @brief Creates a stand-alone RhythmicPosition instance unattached to any JSON document
    /// @param position The rhythmic position value to use.
    RhythmicPosition(const FractionValue& position)
        : Object()
    {
        create_fraction(position);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { fraction() }; }

    /// @brief Create a Required instance for #RhythmicPosition.
    static Required make(const FractionValue& position) { return { position }; }

    /// @brief Compare two rhythmic positions within the same measure.
    /// @param lhs The left-hand position to compare.
    /// @param rhs The right-hand position to compare.
    /// @param rhsIncludesTrailingGrace When true, a right-hand position with
    ///        `graceIndex == 0` is treated as also including trailing grace notes
    ///        at the same fractional position.
    /// @return -1 if @p lhs is earlier, 1 if later, or 0 if equivalent for the requested policy.
    [[nodiscard]] static int compare(const RhythmicPosition& lhs,
                                     const RhythmicPosition& rhs,
                                     bool rhsIncludesTrailingGrace = false);

    MNX_REQUIRED_CHILD(
        Fraction, fraction,
        (const FractionValue&, value)); ///< The metric position, where 1/4 is a quarter note.
    MNX_OPTIONAL_PROPERTY(unsigned, graceIndex);        ///< The grace note index of this position.
                                                        ///< (0 is the primary, and then count to the left.)

    inline static constexpr std::string_view JsonSchemaTypeName = "rhythmic-position";     ///< required for mapping
};

/**
 * @class MeasureRhythmicPosition
 * @brief Represents a system on a page in a score.
 */
class MeasureRhythmicPosition : public Object
{
public:
    /// @brief initializer class for #MeasureRhythmicPosition
    struct Required
    {
        std::string measureId;    ///< the global measure id of the position
        FractionValue position{}; ///< the position within the measure
    };

    /// @brief Constructor for existing rhythmic position instances
    MeasureRhythmicPosition(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }
    
    /// @brief Creates a new MeasureRhythmicPosition class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param measureId The measure id of the position
    /// @param position The position within the measure
    MeasureRhythmicPosition(Base& parent, std::string_view key, const std::string& measureId, const FractionValue& position)
        : Object(parent, key)
    {
        set_measure(measureId);
        create_position(position);
    }
    
    /// @brief Creates a stand-alone MeasureRhythmicPosition class unattached to any json document
    /// @param measureId The measure id of the position
    /// @param position The position within the measure
    MeasureRhythmicPosition(const std::string& measureId, const FractionValue& position)
        : Object()
    {
        set_measure(measureId);
        create_position(position);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { measure(), position().fraction() }; }

    /// @brief Create a Required instance for #MeasureRhythmicPosition.
    static Required make(const std::string& measureId, const FractionValue& position) { return { measureId, position }; }

    MNX_REQUIRED_PROPERTY(std::string, measure);    ///< The id of the global measure of this MeasureRhythmicPosition.
    MNX_REQUIRED_CHILD(
        RhythmicPosition, position,
        (const FractionValue&, position)); ///< The metric position, where 1/4 is a quarter note.

    inline static constexpr std::string_view JsonSchemaTypeName = "measure-rhythmic-position";     ///< required for mapping
};

/**
 * @class Interval
 * @brief Represents a musical chromatic interval
 */
class Interval : public Object
{
public:
    /// @brief initializer class for #Interval
    struct Required
    {
        int staffDistance{};    ///< the number of diatonic steps in the interval (negative is down)
        int halfSteps{};        ///< the number of 12-EDO chromatic halfsteps in the interval (negative is down)
    };

    /// @brief Constructor for existing NoteValue instances
    Interval(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Barline class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param staffDistance The number of diatonic steps in the interval (negative is down)
    /// @param halfSteps The number of 12-EDO chromatic halfsteps in the interval (negative is down)
    Interval(Base& parent, std::string_view key, int staffDistance, int halfSteps)
        : Object(parent, key)
    {
        set_halfSteps(halfSteps);
        set_staffDistance(staffDistance);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { staffDistance(), halfSteps() }; }

    /// @brief Create a Required instance for #Interval.
    static Required make(int staffDistance, int halfSteps) { return { staffDistance, halfSteps }; }

    MNX_REQUIRED_PROPERTY(int, halfSteps);      ///< the number of 12-EDO chromatic halfsteps in the interval (negative is down)
    MNX_REQUIRED_PROPERTY(int, staffDistance);  ///< the number of diatonic steps in the interval (negative is down)

    inline static constexpr std::string_view JsonSchemaTypeName = "interval";     ///< required for mapping
};

/**
 * @class KeySignature
 * @brief Represents a key signature
 */
class KeySignature : public Object
{
public:
    /// @brief initializer class for #KeySignature
    struct Required
    {
        int fifths{};           ///< offset from signature with no accidentals
    };

    /// @brief Constructor for existing KeySignature objects
    KeySignature(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { fifths() }; }

    /// @brief Create a Required instance for #KeySignature.
    static Required make(int fifths) { return { fifths }; }

    /// @brief Creates a new KeySignature class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param fifths The offset from signature with no accidentals
    KeySignature(Base& parent, std::string_view key, int fifths)
        : Object(parent, key)
    {
        set_fifths(fifths);
    }

    MNX_OPTIONAL_PROPERTY(std::string, color);                  ///< color to use when rendering the key signature
    MNX_REQUIRED_PROPERTY(int, fifths);                         ///< offset from signature with no accidentals

    inline static constexpr std::string_view JsonSchemaTypeName = "key";     ///< required for mapping
};

/**
 * @class NoteValue
 * @brief Represents a symbolic note value (not necessarily a duration)
 */
class NoteValue : public Object
{
public:
    /// @brief initializer class for #NoteValue
    struct Required
    {
        NoteValueBase base{};   ///< the note value base to initialize
        unsigned dots{};        ///< the number of dots to initialize
    };

    /// @brief Constructor for existing NoteValue instances
    NoteValue(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Barline class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param base The note value base to initialize
    /// @param dots The number of dots to initialize
    NoteValue(Base& parent, std::string_view key, NoteValueBase base, unsigned dots = 0)
        : Object(parent, key)
    {
        set_base(base);
        if (dots) {
            set_dots(dots);
        }
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { base(), dots() }; }

    /// @brief Create a Required instance for #NoteValue.
    static Required make(NoteValueBase base, unsigned dots = 0) { return { base, dots }; }

    /// @brief Convert the note value to a Fraction base where a quarter note is 1/4.
    [[nodiscard]] operator FractionValue() const;

    MNX_REQUIRED_PROPERTY(NoteValueBase, base);                 ///< the type ("base") of note
    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(unsigned, dots, 0);      ///< the number of dots

    /// @brief Calculates the number of flags or beams required by this note value
    [[nodiscard]] unsigned calcNumberOfFlags() const;

    inline static constexpr std::string_view JsonSchemaTypeName = "note-value";     ///< required for mapping
};

/**
 * @class NoteValueQuantity
 * @brief Represents a quantity of symbolic note values=
 */
class NoteValueQuantity : public Object
{
public:
    /// @brief initializer class for #NoteValueQuantity
    struct Required
    {
        unsigned count{};               ///< The quantity of note values
        NoteValue::Required noteValue{};  ///< the note value base to initialize
    };

    /// @brief Constructor for existing NoteValue instances
    NoteValueQuantity(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Barline class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param count The quantity of note values
    /// @param noteValue The note value base to initialize
    NoteValueQuantity(Base& parent, std::string_view key, unsigned count, const NoteValue::Required& noteValue)
        : Object(parent, key)
    {
        set_multiple(count);
        create_duration(noteValue.base, noteValue.dots);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { multiple(), duration() }; }

    /// @brief Create a Required instance for #NoteValueQuantity.
    static Required make(unsigned count, const NoteValue::Required& noteValue) { return { count, noteValue }; }

    /// @brief Convert the note value quantity to a Fraction base where a quarter note is 1/4.
    [[nodiscard]] operator FractionValue() const
    { return multiple() * duration(); }

    MNX_REQUIRED_CHILD(NoteValue, duration,
        (NoteValueBase, base), (unsigned, dots)); ///< duration unit
    MNX_REQUIRED_PROPERTY(unsigned, multiple);                  ///< quantity of duration units

    inline static constexpr std::string_view JsonSchemaTypeName = "note-value-quantity";     ///< required for mapping
};

/**
 * @class StaffConfig
 * @brief Represents a staff configuration
 */
class StaffConfig : public Object
{
public:
    using Object::Object;

    MNX_OPTIONAL_PROPERTY_WITH_DEFAULT(unsigned, lines, 5u);    ///< number of staff lines

    inline static constexpr std::string_view JsonSchemaTypeName = "staff-config";     ///< required for mapping
};

/**
 * @class TimeSignature
 * @brief Represents the tempo for a global measure.
 */
class TimeSignature : public Object
{
public:
    /// @brief initializer class for #TimeSignature
    struct Required
    {
        int count{};             ///< the number of beats (top number)
        TimeSignatureUnit unit{}; ///< the unit value (bottom number)
    };

    /// @brief Constructor for existing NoteValue instances
    TimeSignature(const std::shared_ptr<json>& root, json_pointer pointer)
        : Object(root, pointer)
    {
    }

    /// @brief Creates a new Barline class as a child of a JSON element
    /// @param parent The parent class instance
    /// @param key The JSON key to use for embedding in parent.
    /// @param count The number of beats (top number)
    /// @param unit The unit value (bottom number)
    TimeSignature(Base& parent, std::string_view key, int count, TimeSignatureUnit unit)
        : Object(parent, key)
    {
        set_count(count);
        set_unit(unit);
    }

    /// @brief Implicit conversion back to Required.
    operator Required() const { return { count(), unit() }; }

    /// @brief Create a Required instance for #TimeSignature.
    static Required make(int count, TimeSignatureUnit unit) { return { count, unit }; }

    /// @brief Implicit converter to FractionValue. This function preserves the time signature values
    /// rather than reducing to lowest common denominator.
    [[nodiscard]] operator FractionValue() const
    { return FractionValue(static_cast<FractionValue::NumType>(count()), static_cast<FractionValue::NumType>(unit())); }

    MNX_REQUIRED_PROPERTY(int, count);                      ///< the number of beats (top number)
    MNX_OPTIONAL_PROPERTY(TimeSignatureDisplay, display);   ///< A special time signature glyph to use, e.g. for common time or cut time.
    MNX_REQUIRED_PROPERTY(TimeSignatureUnit, unit);         ///< the unit value (bottom number)

    inline static constexpr std::string_view JsonSchemaTypeName = "time";     ///< required for mapping
};

} // namespace mnx
