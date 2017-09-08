#ifndef _UTF8_FWRAP_HPP
#define _UTF8_FWRAP_HPP

#include <utf8.h>
#include <string>
#include <iterator>
#include <stdexcept>
#include <cstdio>
#include <cwchar>
#include <stdint.h>

#ifdef _WIN32
	#define utf32to8 utf16to8
#endif

inline wint_t fgetwc_u8(FILE *in) {
#ifdef _WIN32
	struct _cps {
		FILE *f = 0;
		wchar_t c = 0;
	};
	static _cps cps[4];

	for (auto& cp : cps) {
		if (cp.f == in) {
			cp.f = 0;
			return cp.c;
		}
	}
#endif

	int32_t rv = 0;
	int c = 0, i = 0;
	char buf[4];
	if ((c = fgetc_unlocked(in)) != EOF) {
		buf[i++] = static_cast<char>(c);
		if ((c & 0xF0) == 0xF0) {
			if (fread_unlocked(buf+i, 1, 3, in) != 3) {
				throw std::runtime_error("Could not read 3 expected bytes from stream");
			}
			i += 3;
		}
		else if ((c & 0xE0) == 0xE0) {
			if (fread_unlocked(buf+i, 1, 2, in) != 2) {
				throw std::runtime_error("Could not read 2 expected bytes from stream");
			}
			i += 2;
		}
		else if ((c & 0xC0) == 0xC0) {
			if (fread_unlocked(buf+i, 1, 1, in) != 1) {
				throw std::runtime_error("Could not read 1 expected byte from stream");
			}
			i += 1;
		}
	}
	if (i == 0 && c == EOF) {
		rv = WEOF;
	}
	else {
#ifdef _WIN32
		wchar_t u16[2] = {};
		utf8::unchecked::utf8to16(buf, buf+i, u16);

		if (u16[1]) {
			for (auto& cp : cps) {
				if (cp.f == 0) {
					cp.f = in;
					cp.c = u16[1];
					return u16[0];
				}
			}
			throw std::runtime_error("Not enough space to store UTF-16 high surrogate");
		}
		rv = u16[0];
#else
		utf8::unchecked::utf8to32(buf, buf+i, &rv);
#endif
	}
	return static_cast<wint_t>(rv);
}

inline wint_t fputwc_u8(wint_t wc, FILE *out) {
	char buf[4] = {};
	char *e = utf8::unchecked::utf32to8(&wc, &wc+1, buf);
	if (fwrite_unlocked(buf, 1, e-buf, out) != static_cast<size_t>(e-buf)) {
		return WEOF;
	}

	return wc;
}

inline int fputws_u8(const wchar_t* str, FILE *out) {
	static std::string buf;
	buf.clear();
	size_t len = wcslen(str);
	utf8::unchecked::utf32to8(str, str+len, std::back_inserter(buf));
	if (fwrite_unlocked(&buf[0], 1, buf.size(), out) != buf.size()) {
		return WEOF;
	}

	return 1;
}

inline wint_t ungetwc_u8(wint_t wc, FILE *out) {
	char buf[4] = {};
	char *e = utf8::unchecked::utf32to8(&wc, &wc+1, buf);
	for (char *b = buf ; b != e ; ++b) {
		if (ungetc(*b, out) == EOF) {
			return WEOF;
		}
	}

	return wc;
}

#ifdef fgetwc_unlocked
	#undef fgetwc_unlocked
#endif
#define fgetwc_unlocked fgetwc_u8

#ifdef fputwc_unlocked
	#undef fputwc_unlocked
#endif
#define fputwc_unlocked fputwc_u8

#ifdef fputws_unlocked
	#undef fputws_unlocked
#endif
#define fputws_unlocked fputws_u8

#ifdef ungetwc
	#undef ungetwc
#endif
#define ungetwc ungetwc_u8

#ifdef _WIN32
	#undef utf32to8
#endif

#endif
