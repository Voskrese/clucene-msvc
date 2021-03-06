/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* Updated by https://github.com/farfella/.
*
* Distributable under the terms of either the Apache License (Version 2.0) or
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#pragma once

#include <string>

std::wstring float_to_wstring(const float_t value, const size_t digits);
std::wstring boost_to_wstring(const float_t boost);
std::wstring bool_to_wstring(const bool value);


#if 0
namespace lucene {

    namespace util {

        class CLUCENE_SHARED_EXPORT StringBuffer
        {
        public:
            ///Constructor. Allocates a buffer with the default length.
            StringBuffer();
            ///Constructor. Allocates a buffer of length initSize + 1
            StringBuffer(const size_t initSize);
            ///Constructor. Creates an instance of Stringbuffer containing a copy of
            ///the string value
            StringBuffer(const wchar_t* value);
            ///Constructs a StringBuffer using another buffer. The StringBuffer can
            ///the be used to easily manipulate the buffer.
            StringBuffer(wchar_t* buf, size_t maxlen, const bool consumeBuffer);
            ///Destructor
            virtual ~StringBuffer();
            ///Clears the Stringbuffer and resets it to it default empty state
            void clear();

            ///Appends a single character
            void appendChar(const wchar_t chr);
            ///Appends a copy of the string value
            void append(const wchar_t* value);
            ///Appends a copy of the string value
            void append(const wchar_t* value, size_t appendedLength);
            ///Appends an integer (after conversion to a character string) with a default radix of 10. Radixes lower than 10 not supported.
            void appendInt(const int64_t value, const int32_t _Radix = 10);
            ///Appends a float_t (after conversion to a character string)
            void appendFloat(const float_t value, const size_t digits);
            ///Appends a Lucene boost, formatted in the format used in the toString() functions; replaces JL's ToStringUtils::boost
            void appendBoost(const float_t boost);
            ///Appends a bool in the same way as java StringBuffer does (i.e. "true", "false" )
            void appendBool(const bool value);
            ///Puts a copy of the string value in front of the current string in the StringBuffer
            void prepend(const wchar_t* value);
            ///Puts a copy of the string value in front of the current string in the StringBuffer
            void prepend(const wchar_t* value, size_t prependedLength);

            StringBuffer& operator<< (const wchar_t* value)
            {
                append(value);
                return *this;
            }
            StringBuffer& operator<< (const int64_t value)
            {
                appendInt(value);
                return *this;
            }

            void sewchar_tAt(size_t pos, const wchar_t chr);
            wchar_t charAt(size_t pos);

            void insert(const size_t pos, wchar_t chr);
            void insert(const size_t pos, const wchar_t* chrs, size_t length = -1);
            void deleteCharAt(size_t pos);
            void deleteChars(size_t start, size_t end);

            void toLower();
            bool substringEquals(size_t start, size_t end, const wchar_t* str, size_t length = -1) const;

            ///Contains the length of string in the StringBuffer
            ///Public so that analyzers can edit the length directly
            size_t len;
            ///Returns the length of the string in the StringBuffer
            size_t length() const;
            ///Returns a copy of the current string in the StringBuffer
            wchar_t* toString();
            ///Returns a null terminated reference to the StringBuffer's text
            wchar_t* getBuffer();
            /** Returns a null terminated reference to the StringBuffer's text
            * the StringBuffer's buffer is released so that the text doesn't need to be copied
            */
            wchar_t* giveBuffer();

            ///reserve a minimum amount of data for the buffer.
            ///no change made if the buffer is already longer than length
            void reserve(const size_t length);
        private:
            ///A buffer that contains strings
            wchar_t * buffer;
            ///The length of the buffer
            size_t bufferLength;
            bool bufferOwner;

            ///Has the buffer grown to a minimum length of minLength or bigger and shifts the
            ///current string in buffer by skippingNInitialChars forward
            void growBuffer(const size_t minLength, const size_t skippingNInitialChars = 0);

        };
    }
}
#endif
