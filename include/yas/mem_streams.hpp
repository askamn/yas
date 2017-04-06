
// Copyright (c) 2010-2017 niXman (i dot nixman dog gmail dot com). All
// rights reserved.
//
// This file is part of YAS(https://github.com/niXman/yas) project.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
//
// Boost Software License - Version 1.0 - August 17th, 2003
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef __yas__mem_streams_hpp
#define __yas__mem_streams_hpp

#include <yas/detail/config/config.hpp>
#include <yas/detail/tools/cast.hpp>
#include <yas/detail/tools/noncopyable.hpp>
#include <yas/buffers.hpp>

#include <cmath>

namespace yas {

/***************************************************************************/

struct mem_ostream {
    YAS_NONCOPYABLE(mem_ostream)

    mem_ostream(std::size_t reserved = 1024*20)
        :buf(reserved)
        ,beg(buf.data.get())
        ,cur(buf.data.get())
        ,end(buf.data.get()+buf.size)
    {}
    mem_ostream(void *ptr, std::size_t size)
        :buf()
        ,beg(YAS_SCAST(char*, ptr))
        ,cur(YAS_SCAST(char*, ptr))
        ,end(YAS_SCAST(char*, ptr)+size)
    {}

    template<typename T>
    std::size_t write(const T *tptr, const std::size_t size) {
        const std::uint8_t *ptr = YAS_RCAST(const std::uint8_t*, tptr);
        if ( cur+size > end ) {
            shared_buffer::shared_array_type prev = buf.data;
            const std::size_t olds = YAS_SCAST(std::size_t, cur-beg);
            const std::size_t news = YAS_SCAST(std::size_t,
                size + (olds * YAS_SCAST(std::size_t, ((1 + std::sqrt(5)) / 1.5)))
            );

            buf = shared_buffer(news);
            std::memcpy(buf.data.get(), prev.get(), olds);

            beg = buf.data.get();
            cur = beg+olds;
            end = beg+news;
        }

        switch (size) {
            case sizeof(std::uint8_t ):
                *YAS_RCAST(std::uint8_t*, cur) = *YAS_RCAST(const std::uint8_t*, ptr);
            break;
            case sizeof(std::uint16_t): std::memcpy(cur, ptr, sizeof(std::uint16_t)); break;
            case sizeof(std::uint32_t): std::memcpy(cur, ptr, sizeof(std::uint32_t)); break;
            case sizeof(std::uint64_t): std::memcpy(cur, ptr, sizeof(std::uint64_t)); break;
#if defined(__GNUC__) && defined(__SIZEOF_INT128__) // hack for detect int128 support
            case sizeof(unsigned __int128): std::memcpy(cur, ptr, sizeof(unsigned __int128)); break;
#endif
            default: std::memcpy(cur, ptr, size);
        }
        cur += size;

        return size;
    }

    shared_buffer get_shared_buffer() const { return shared_buffer(buf.data, YAS_SCAST(std::size_t, cur-beg)); }
    intrusive_buffer get_intrusive_buffer() const { return intrusive_buffer(beg, YAS_SCAST(std::size_t, cur-beg)); }

private:
    shared_buffer buf;
    char *beg, *cur, *end;
}; // struct mem_ostream

/***************************************************************************/

struct mem_istream {
    YAS_NONCOPYABLE(mem_istream)

    mem_istream(const void *ptr, std::size_t size)
        :beg(YAS_SCAST(const char*, ptr))
        ,cur(YAS_SCAST(const char*, ptr))
        ,end(YAS_SCAST(const char*, ptr)+size)
    {}
    mem_istream(const intrusive_buffer &buf)
        :beg(buf.data)
        ,cur(buf.data)
        ,end(buf.data+buf.size)
    {}
    mem_istream(const shared_buffer &buf)
        :beg(buf.data.get())
        ,cur(buf.data.get())
        ,end(buf.data.get()+buf.size)
    {}

    template<typename T>
    std::size_t read(T *ptr, const std::size_t size) {
        const std::size_t avail = YAS_SCAST(std::size_t, end-cur);
        const std::size_t to_copy = (avail < size) ? avail : size;

        switch ( to_copy ) {
            case sizeof(std::uint8_t ):
                *YAS_RCAST(std::uint8_t*, ptr) = *YAS_RCAST(const std::uint8_t*, cur);
            break;
            case sizeof(std::uint16_t): std::memcpy(ptr, cur, sizeof(std::uint16_t)); break;
            case sizeof(std::uint32_t): std::memcpy(ptr, cur, sizeof(std::uint32_t)); break;
            case sizeof(std::uint64_t): std::memcpy(ptr, cur, sizeof(std::uint64_t)); break;
#if defined(__GNUC__) && defined(__SIZEOF_INT128__) // hack for detect int128 support
            case sizeof(unsigned __int128): std::memcpy(ptr, cur, sizeof(unsigned __int128)); break;
#endif
            default: std::memcpy(ptr, cur, to_copy);
        }
        cur += to_copy;

        return to_copy;
    }

    shared_buffer get_shared_buffer() const { return shared_buffer(cur, YAS_SCAST(std::size_t, end-cur)); }
    intrusive_buffer get_intrusive_buffer() const { return intrusive_buffer(cur, YAS_SCAST(std::size_t, end-cur)); }

private:
    const char *beg, *cur, *end;
}; // struct mem_istream

/***************************************************************************/

} // ns yas

#endif // __yas__mem_streams_hpp
