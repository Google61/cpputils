#pragma once

#include <cpputils/arrayview.h>
#include <algorithm>
/*
 *
 * enumerate lines in a byte / char range like this:
 *
 * for (auto line : lineenumerator(range))
 * {
 *    ...
 * }
 *
 * note that this is kind of a duplicate of stringsplitter
 */

// todo: fix problem where last non-lf terminated section is not
// returned as a part.
template<typename PTR=std::string::const_iterator>
struct lineenumerator {
    struct iter {
        PTR p, q, last;

        iter(PTR first, PTR last)
            : p(first), q(first), last(last)
        {
            q = std::find(p, last, '\n');
        }
        auto operator*()
        {
            return std::string(p, q);
        }
        iter& operator++()
        {
            p = (q<last) ? q+1 : last;
            q = std::find(p, last, '\n');

            return *this;
        }
        friend bool operator!=(const iter& lhs, const iter& rhs)
        {
            return lhs.p != rhs.p;
        }
    };

    PTR first, last;
    lineenumerator(const std::string& v)
        : first(v.begin()), last(v.end())
    {
    }

    lineenumerator(PTR first, PTR last)
        : first(first), last(last)
    {
    }

    iter begin() { return iter(first, last); }
    iter end() { return iter(last, last); }

};
/*
struct lineenumerator {
    template<typename PTR>
    struct iter {
        PTR p, q, last;

        iter(PTR first, PTR last)
            : p(first), q(first), last(last)
        {
            q = std::find(p, last, '\n');
        }
        auto operator*()
        {
            return std::span(p, q);
        }
        iter& operator++()
        {
            p = (q<last) ? q+1 : last;
            q = std::find(p, last, '\n');

            return *this;
        }
        friend bool operator!=(const iter& lhs, const iter& rhs)
        {
            return lhs.p != rhs.p;
        }
    };


    std::span<const char> text;

    template<typename CHAR>
    lineenumerator(std::span<const CHAR> text)
        : text((const char*)text.data(), (const char*)text.data()+text.size())
    {
    }
    iter begin() { return iter(text.begin(), text.end()); }
    iter end() { return iter(text.end(), text.end()); }
};
*/
