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

/** @mainpage
 * Welcome to the <b>MNX Document Model</b>.
 *
 * Use the <b>navigation buttons</b> at the top of this page to browse through
 * the framework documentation.
 *
 * The MNX Document Model is a <b>C++ class framework</b>
 * around the MNX music interchange format, which uses JSON files.
 * It requires C++20 or newer. The default CMake build uses C++23.
 * 
 * Features include:
 * - Implemented with an underlying nlohmann JSON document. No class-level serializing to/from JSON is required.
 * - Lightweight class definitions contain only the shared JSON document and a json_pointer to the object. (See @ref mnx::Base.)
 * - Validation routines for both the MNX JSON schema and semantic validation.
 *
 * Member functions for each MNX entity are defined using macros that create the boilerplate for the definitions.
 * For example, `MNX_REQUIRED_PROPERTY` creates a getter (equal to the property name) and a setter (`set_` followed
 * by the property name). The boilerplate macros are defined in `BaseTypes.h` and have extensive documentation in the comments.
 *
 * A key feature of this DOM is that MNX-required children and properties must be supplied when an object is constructed
 * from scratch. They can be subsequently changed if need be. Many classes have `Required` types (with `make(...)` helpers)
 * to facilitate passing all the required data for a child object.
 *
 * Example:
 * @code
 * mnx::sequence::Event evt(seq, "content", NoteValueBase::Quarter, 1);
 *
* auto note = evt.ensure_notes().append(mnx::sequence::Pitch::make(NoteStep::C, 4));
 * @endcode
 *
 * @warning Avoid using references for instances returned by the methods in this library. Because classes are so lightweight,
 * all methods return temporary copies. Using a reference on a temporary is undefined behavior.
 *
 * @author Robert Patterson
 */

#pragma once

#include "BaseTypes.h"
#include "Enumerations.h"
#include "Global.h"
#include "Layout.h"
#include "Part.h"
#include "Score.h"
#include "Document.h"
#include "validation/Validation.h"
#include "util/EntityMap.h"
#include "util/LayoutHelpers.h"
#include "util/WalkSequenceContent.h"
