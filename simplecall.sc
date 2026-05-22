SIMPLECALL(1) "March 2026" "simplecall"

# NAME

simplecall - perform basic syscalls with given arguments

# SYNOPSIS

*simplecall* _syscall-id_ [_argument_]...

# DESCRIPTION

*simplecall* decodes each _argument_ and passes the decoded values to
the syscall identified by _syscall-id_.

Each argument has the form:

	_spec_:_data_

Supported specifiers are:

*i8, i16, i32, i64*
	Signed integer. _data_ is parsed as integer text (decimal by
	default, *0o* prefix for octal, *0x* prefix for hexadecimal),
	converted to the platform's widest integer, truncated to the
	requested width, then sign-extended to the platform *long* used
	for syscall arguments.

*u8, u16, u32, u64*
	Unsigned integer. _data_ is parsed as integer text (decimal by
	default, *0o* prefix for octal, *0x* prefix for hexadecimal),
	converted to the platform's widest integer, truncated to the
	requested width, then zero-extended to the platform *long* used
	for syscall arguments.

*s*
	String. _data_ is passed as a pointer to a C-style string
	buffer.

*b*
	Binary buffer. _data_ is interpreted as hexadecimal bytes (two
	hex digits per byte) and passed as a pointer to the allocated
	buffer.

*o*
	Output buffer. _data_ is parsed as a size in bytes (decimal,
	*0o*, or *0x*) and a buffer of that size is allocated; the
	pointer is passed to the syscall.

# EXIT STATUS

*0*
	Syscall succeeded.

*120*
	Too many syscall arguments.

*121*
	Bad argument format/specifier, invalid binary length, or
	allocation failure in output-buffer path.

*122*
	Allocation failure in binary-buffer path.

*2..255*
	A propagated _errno_ value when syscall fails.

# EXAMPLES

*simplecall 93*
	Invoke syscall 93 with no arguments.

*simplecall 1 i8:123 u64:0xdeadbeefcafebabe*
	Invoke syscall 1 with two arguments: 123 and
	0xdeadbeefcafebabe.

*simplecall 2 's: Hello World: from simplecall!' u8:1*
	Invoke syscall 2 with two arguments: pointer to string
	" Hello World: from simplecall!" and integer 1.

*simplecall 3 b:aabbccdd*
	Invoke syscall 3 with one argument: pointer to 4-byte buffer
	AA BB CC DD.

*simplecall 17 o:64 u8:64*
	Invoke syscall 17 with two argument: a pointer to a buffer of
	64-bytes, and an integer 64.

# NOTES

This program does not impose semantics on the syscall and does not
interpret successful syscall return values.

Current source is only tested for LoongArch64 compatibility.

# SEE ALSO

*syscall*(2), *errno*(3)

# BUGS

https://github.com/CSharperMantle/simplecall/issues

# AUTHORS

Written by Rong "Mantle" Bao <rong.bao@csmantle.top>.
