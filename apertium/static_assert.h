#ifndef _STATIC_ASSERT_H
#define _STATIC_ASSERT_H

namespace Apertium {
template<bool>
struct static_assertion;

template<>
struct static_assertion<true> {};
}

#endif
