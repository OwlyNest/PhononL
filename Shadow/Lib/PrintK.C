/*
 * Lib/PrintK.C - Kernel formatted output
 * Author:   amity
 * Date:     Thu Jun 11 10:01:53 2026
 * Copyright © 2026 OwlyNest
 *
 * Portable for both 32-bit and 64-bit. All va_arg consumers take va_list *
 * so the caller's argument pointer is advanced correctly on every ABI
 * (i386 char*, x86_64 struct, etc.). Passing va_list by value is undefineds
 * for this purpose and was the cause of the 32-bit PCI dump corruption.
 */

/* --- Includes ---*/
#include <GFX/GFX.H>
#include <Lib/Lib.H>

#include <stdarg.h>

/* --- Functions ---*/
static const char *fetch_string(va_list *args) {
  return va_arg(*args, const char *);
}

static void *fetch_pointer(va_list *args) { return va_arg(*args, void *); }

static int fetch_char(va_list *args) { return va_arg(*args, int); }

static void emit_char(char **out, size_t *remaining, int *count, char c) {
	if (*remaining > 1) {
		**out = c;
		(*out)++;
		(*remaining)--;
		(*count)++;
	}
	}

	static void emit_string(char **out, size_t *remaining, int *count,
							const char *str) {
	while (*str) {
		emit_char(out, remaining, count, *str++);
	}
	}

/* ==========================================================================
 *                                                                          *
 * Render an unsigned value into a NUL-terminated digit buffer (no sign,    *
 * no padding — those are the caller's job)                                 *
 *                                                                          *
 * ======================================================================= */
static void utoa_base(uint64_t value, unsigned base, int uppercase, char *buf) {
	char tmp[65];
	int i = 0;
	const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

	if (base < 2 || base > 16) {
		buf[0] = '\0';
		return;
	}

	if (value == 0) {
		buf[0] = '0';
		buf[1] = '\0';
		return;
	}

	while (value > 0) {
		tmp[i++] = digits[value % base];
		value /= base;
	}

	int j = 0;
	while (i > 0) {
		buf[j++] = tmp[--i];
	}
	buf[j] = '\0';
}

/* ==========================================================================
 *                                                                          *
 * Fetch an integer argument sized according to a length modifier.          *
 * Integer promotions mean hh/h always arrive as int / unsigned int.        *
 *                                                                          *
 * ======================================================================== */
static uint64_t fetch_unsigned(LengthModifier len, va_list *args) {
	switch (len) {
		case LEN_L:
			return (uint64_t)va_arg(*args, unsigned long);
		case LEN_LL:
			return (uint64_t)va_arg(*args, unsigned long long);
		case LEN_Z:
			return (uint64_t)va_arg(*args, size_t);
		case LEN_T:
			return (uint64_t)va_arg(*args, ptrdiff_t);
		case LEN_HH:
		case LEN_H:
		case LEN_NONE:
		default:
			/* short / unsigned short / unsigned int all promoted to unsigned int */
			return (uint64_t)va_arg(*args, unsigned int);
	}
}

static int64_t fetch_signed(LengthModifier len, va_list *args) {
	switch (len) {
		case LEN_L:
			return (int64_t)va_arg(*args, long);
		case LEN_LL:
			return (int64_t)va_arg(*args, long long);
		case LEN_Z:
			/* %zd is implementation-defined; treat as signed size (ssize_t) */
			return (int64_t)va_arg(*args, size_t);
		case LEN_T:
			return (int64_t)va_arg(*args, ptrdiff_t);
		case LEN_HH:
		case LEN_H:
		case LEN_NONE:
		default:
			return (int64_t)va_arg(*args, int);
	}
}

/* ==========================================================================
 *                                                                          *
 * Format one integer conversion — handles sign, flags, width AND           *
 * precision (minimum digit count)                                          *
 *                                                                          *
 * ======================================================================== */
static void format_integer(char **out, size_t *remaining, int *count,
                           uint64_t uvalue, int negative, unsigned base,
                           int uppercase, const FormatSpec *spec) {
	char digits[65];
	char prefix[2] = {0, 0};
	int prefix_len = 0;
	int len;
	int digit_pad;
	int total_len;
	int field_pad;
	int use_zero;
	int i;

	utoa_base(uvalue, base, uppercase, digits);

	if (uvalue == 0 && spec->precision == 0) {
		digits[0] = '\0'; /* precision 0 + value 0 -> no digits */
	}

	len = (int)StrLen(digits);
	digit_pad = (spec->precision > len) ? (spec->precision - len) : 0;

	if (negative) {
		prefix[prefix_len++] = '-';
	} else if (spec->plus && base == 10) {
		prefix[prefix_len++] = '+';
	} else if (spec->space && base == 10) {
		prefix[prefix_len++] = ' ';
	}

	if (spec->alternate && base == 16 && uvalue != 0) {
		prefix[prefix_len++] = uppercase ? 'X' : 'x';
		/* '0' goes out separately below, ahead of this */
	}

	total_len = prefix_len + digit_pad + len +
				((spec->alternate && base == 16 && uvalue != 0) ? 1 : 0);
	field_pad = (spec->width > total_len) ? (spec->width - total_len) : 0;

	/* Zero-padding is suppressed once a precision is given, matching
		standard printf semantics (precision already pads the digits) */
	use_zero = spec->zero && !spec->left && spec->precision < 0;

	if (!spec->left && !use_zero) {
		while (field_pad--)
		emit_char(out, remaining, count, ' ');
	}

	if (spec->alternate && base == 16 && uvalue != 0) {
		emit_char(out, remaining, count, '0');
	}
	for (i = 0; i < prefix_len; i++) {
		emit_char(out, remaining, count, prefix[i]);
	}

	if (!spec->left && use_zero) {
		while (field_pad--)
		emit_char(out, remaining, count, '0');
	}

	while (digit_pad--)
		emit_char(out, remaining, count, '0');
	emit_string(out, remaining, count, digits);

	if (spec->left) {
		while (field_pad--) {
		emit_char(out, remaining, count, ' ');
		}
	}
}

/* ==========================================================================
 *                                                                          *
 * Format one %s conversion — precision bounds the read, so this is safe    *
 * on non-NUL-terminated fixed arrays (ACPI signatures, OEM IDs, etc.)      *
 *                                                                          *
 * ======================================================================== */
static void format_string(char **out, size_t *remaining, int *count, const char *str, const FormatSpec *spec) {
	int len;
	int pad;

	if (!str) {
		str = "(null)";
	}

	if (spec->precision >= 0) {
		len = 0;
		while (len < spec->precision && str[len]) {
		len++;
		}
	} else {
		len = (int)StrLen(str);
	}

	pad = (spec->width > len) ? (spec->width - len) : 0;

	if (!spec->left) {
		while (pad--) {
		emit_char(out, remaining, count, ' ');
		}
	}

	for (int i = 0; i < len; i++) {
		emit_char(out, remaining, count, str[i]);
	}

	if (spec->left) {
		while (pad--) {
		emit_char(out, remaining, count, ' ');
		}
	}
}

/* ==========================================================================
 *                                                                          *
 * Parse one %-conversion starting just past the '%'. Returns a pointer     *
 * just past the specifier character. Consumes * width/precision via *args. *
 *                                                                          *
 * ======================================================================== */
static const char *parse_format(const char *fmt, FormatSpec *spec, va_list *args) {
	spec->left = 0;
	spec->zero = 0;
	spec->alternate = 0;
	spec->plus = 0;
	spec->space = 0;
	spec->width = 0;
	spec->precision = -1;
	spec->length = LEN_NONE;
	spec->specifier = '\0';

	/* --- Flags --- */
	for (;;) {
		if (*fmt == '-') {
		spec->left = 1;
		fmt++;
		} else if (*fmt == '0') {
		spec->zero = 1;
		fmt++;
		} else if (*fmt == '+') {
		spec->plus = 1;
		fmt++;
		} else if (*fmt == ' ') {
		spec->space = 1;
		fmt++;
		} else if (*fmt == '#') {
		spec->alternate = 1;
		fmt++;
		} else
		break;
	}

	/* --- Width --- */
	if (*fmt == '*') {
		spec->width = va_arg(*args, int);
		if (spec->width < 0) {
		spec->left = 1;
		spec->width = -spec->width;
		}
		fmt++;
	} else {
		while (*fmt >= '0' && *fmt <= '9') {
		spec->width = (spec->width * 10) + (*fmt - '0');
		fmt++;
		}
	}

	/* --- Precision --- */
	if (*fmt == '.') {
		fmt++;
		spec->precision = 0;
		if (*fmt == '*') {
		spec->precision = va_arg(*args, int);
		if (spec->precision < 0)
			spec->precision = -1; /* negative precision = omitted */
		fmt++;
		} else {
		while (*fmt >= '0' && *fmt <= '9') {
			spec->precision = (spec->precision * 10) + (*fmt - '0');
			fmt++;
		}
		}
	}

	/* --- Length modifier --- */
	if (*fmt == 'h') {
		fmt++;
		if (*fmt == 'h') {
		spec->length = LEN_HH;
		fmt++;
		} else
		spec->length = LEN_H;
	} else if (*fmt == 'l') {
		fmt++;
		if (*fmt == 'l') {
		spec->length = LEN_LL;
		fmt++;
		} else
		spec->length = LEN_L;
	} else if (*fmt == 'z') {
		spec->length = LEN_Z;
		fmt++;
	} else if (*fmt == 't') {
		spec->length = LEN_T;
		fmt++;
	}

	spec->specifier = *fmt;
	if (*fmt) {
		fmt++;
	}

	return fmt;
}

/* ==========================================================================
 *                                                                          *
 * Core formatter                                                           *
 *                                                                          *
 * ======================================================================== */
int kvsnprintf(char *buf, size_t size, const char *fmt, va_list args) {
	char *out = buf;
	size_t remaining = size;
	int count = 0;
	/* Work on a local copy so we never mutate the caller's va_list. */
	va_list ap;
	va_copy(ap, args);

	while (*fmt) {
		if (*fmt != '%') {
		emit_char(&out, &remaining, &count, *fmt);
		fmt++;
		continue;
		}

		fmt++;

		if (*fmt == '%') {
		emit_char(&out, &remaining, &count, '%');
		fmt++;
		continue;
		}

		FormatSpec spec;
		fmt = parse_format(fmt, &spec, &ap);

		switch (spec.specifier) {
		case 's':
		format_string(&out, &remaining, &count, fetch_string(&ap), &spec);
		break;

		case 'c': {
		emit_char(&out, &remaining, &count, (char)fetch_char(&ap));
		break;
		}

		case 'd':
		case 'i': {
		int64_t value = fetch_signed(spec.length, &ap);
		uint64_t mag =
			(value < 0) ? (uint64_t)(-(uint64_t)value) : (uint64_t)value;
		format_integer(&out, &remaining, &count, mag, value < 0, 10, 0, &spec);
		break;
		}

		case 'u':
		format_integer(&out, &remaining, &count, fetch_unsigned(spec.length, &ap),
						0, 10, 0, &spec);
		break;

		case 'x':
		format_integer(&out, &remaining, &count, fetch_unsigned(spec.length, &ap),
						0, 16, 0, &spec);
		break;

		case 'X':
		format_integer(&out, &remaining, &count, fetch_unsigned(spec.length, &ap),
						0, 16, 1, &spec);
		break;

		case 'p': {
		FormatSpec pspec = spec;
		/* Always print a full-width zero-padded pointer (8 hex digits on
			32-bit, 16 on 64-bit).  Do not honour a user-supplied width or
			precision for %p — that would break the conventional look. */
		pspec.width = (int)(sizeof(uintptr_t) * 2);
		pspec.zero = 1;
		pspec.precision = -1;
		pspec.left = 0;
		pspec.plus = 0;
		pspec.space = 0;
		pspec.alternate = 0;

		emit_string(&out, &remaining, &count, "0x");
		format_integer(&out, &remaining, &count,
						(uint64_t)(uintptr_t)fetch_pointer(&ap), 0, 16, 0, &pspec);
		break;
		}

		case '\0':
		break;

		default:
		/* Unknown specifier: echo it back literally instead of
		* silently eating the argument. This is the exact bug
		* class that produced "2d"/"8X8X"/"6s" in the ACPICA log.
		* Make future mismatches visible near the source instead
		* of scattered as stray letters downstream.
		*/
		emit_char(&out, &remaining, &count, '%');
		emit_char(&out, &remaining, &count, spec.specifier);
		break;
		}
	}

	va_end(ap);

	if (size > 0) {
		*out = '\0';
	}
	return count;
}

int ksnprintf(char *buf, size_t size, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int ret = kvsnprintf(buf, size, fmt, args);
	va_end(args);
	return ret;
}

VOID printk(PCCHAR fmt, ...) {
  	char buf[4096];
  	va_list args;
  	va_start(args, fmt);
	kvsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	ConsoleWrite(buf);

  	/* Framebuffer presentation belongs to the graphics/UI caller. */
  	if (fb.Initialized) {
		if (ConsoleIsDirty()) {
			ConsoleRedraw();
		}
    	FbPresent();
  	}
}