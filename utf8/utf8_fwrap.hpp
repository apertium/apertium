#ifndef _UTF8_FWRAP_HPP
#define _UTF8_FWRAP_HPP

#include <utf8.h>
#include <string>
#include <iterator>
#include <stdexcept>
#include <cstdio>
#include <cwchar>
#include <stdint.h>

#define _SECURE_SCL 0
#define _ITERATOR_DEBUG_LEVEL 0
#define _CRT_SECURE_NO_DEPRECATE
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define NOMINMAX
#include <windows.h>

inline wint_t fgetwc_u8(FILE *in) {
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
		utf8::unchecked::utf8to32(buf, buf+i, &rv);
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

inline int mbtowc_u8(wchar_t* dst, const char* src, std::size_t n) {
	return MultiByteToWideChar(CP_UTF8, 0, s, -1, dst, 1);
}

inline std::size_t mbstowcs_u8(wchar_t* dst, const char* src, std::size_t len) {
	return MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, len);
}

inline int wctomb_u8(char *s, wchar_t wc) {
	return WideCharToMultiByte(CP_UTF8, 0, &wc, 1, s, 4, 0, 0);
}

inline std::size_t wcstombs_u8(char* dst, const wchar_t* src, std::size_t len) {
	return WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, len, 0, 0);
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

#ifdef mbtowc
	#undef mbtowc
#endif
#define mbtowc mbtowc_u8

#ifdef mbstowcs
	#undef mbstowcs
#endif
#define mbstowcs mbstowcs_u8

#ifdef wctomb
	#undef wctomb
#endif
#define wctomb wctomb_u8

#ifdef wcstombs
	#undef wcstombs
#endif
#define wcstombs wcstombs_u8

#endif
