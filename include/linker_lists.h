/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * include/linker_lists.h
 *
 * Implementation of linker-generated arrays
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 */

#ifndef __LINKER_LISTS_H__
#define __LINKER_LISTS_H__

#include <linux/compiler.h>

/*
 * There is no use in including this from ASM files.
 * So just don't define anything when included from ASM.
 */

#if !defined(__ASSEMBLY__)

/**
 * llsym() - Access a linker-generated array entry
 * @_type:	Data type of the entry
 * @_name:	Name of the entry
 * @_list:	name of the list. Should contain only characters allowed
 *		in a C variable name!
 */
#define llsym(_type, _name, _list) \
		((_type *)&_u_boot_list_2_##_list##_2_##_name)

/**
 * ll_entry_declare() - Declare linker-generated array entry
 * @_type:	Data type of the entry
 * @_name:	Name of the entry
 * @_list:	name of the list. Should contain only characters allowed
 *		in a C variable name!
 *
 * This macro declares a variable that is placed into a linker-generated
 * array. This is a basic building block for more advanced use of linker-
 * generated arrays. The user is expected to build their own macro wrapper
 * around this one.
 *
 * A variable declared using this macro must be compile-time initialized.
 *
 * Special precaution must be made when using this macro:
 *
 * 1) The _type must not contain the "static" keyword, otherwise the
 *    entry is generated and can be iterated but is listed in the map
 *    file and cannot be retrieved by name.
 *
 * 2) In case a section is declared that contains some array elements AND
 *    a subsection of this section is declared and contains some elements,
 *    it is imperative that the elements are of the same type.
 *
 * 3) In case an outer section is declared that contains some array elements
 *    AND an inner subsection of this section is declared and contains some
 *    elements, then when traversing the outer section, even the elements of
 *    the inner sections are present in the array.
 *
 * Example:
 *
 * ::
 *
 *   ll_entry_declare(struct my_sub_cmd, my_sub_cmd, cmd_sub) = {
 *           .x = 3,
 *           .y = 4,
 *   };
 */
#define ll_entry_declare(_type, _name, _list)				\
	_type _u_boot_list_2_##_list##_2_##_name __aligned(4)		\
			__attribute__((unused))				\
			__section("__u_boot_list_2_"#_list"_2_"#_name)

/**
 * ll_entry_declare_list() - Declare a list of link-generated array entries
 * @_type:	Data type of each entry
 * @_name:	Name of the entry
 * @_list:	name of the list. Should contain only characters allowed
 *		in a C variable name!
 *
 * This is like ll_entry_declare() but creates multiple entries. It should
 * be assigned to an array.
 *
 * ::
 *
 *   ll_entry_declare_list(struct my_sub_cmd, my_sub_cmd, cmd_sub) = {
 *        { .x = 3, .y = 4 },
 *        { .x = 8, .y = 2 },
 *        { .x = 1, .y = 7 }
 *   };
 */
#define ll_entry_declare_list(_type, _name, _list)			\
	_type _u_boot_list_2_##_list##_2_##_name[] __aligned(4)		\
			__attribute__((unused))				\
			__section("__u_boot_list_2_"#_list"_2_"#_name)

/*
 * We need a 0-byte-size type for iterator symbols, and the compiler
 * does not allow defining objects of C type 'void'. Using an empty
 * struct is allowed by the compiler, but causes gcc versions 4.4 and
 * below to complain about aliasing. Therefore we use the next best
 * thing: zero-sized arrays, which are both 0-byte-size and exempt from
 * aliasing warnings.
 */

/**
 * ll_entry_start() - Point to first entry of linker-generated array
 * @_type:	Data type of the entry
 * @_list:	Name of the list in which this entry is placed
 *
 * This function returns ``(_type *)`` pointer to the very first entry of a
 * linker-generated array placed into subsection of __u_boot_list section
 * specified by _list argument.
 *
 * Since this macro defines an array start symbol, its leftmost index
 * must be 2 and its rightmost index must be 1.
 *
 * Example:
 *
 * ::
 *
 *   struct my_sub_cmd *msc = ll_entry_start(struct my_sub_cmd, cmd_sub);
 */
#define ll_entry_start(_type, _list)					\
({									\
	static char start[0] __aligned(CONFIG_LINKER_LIST_ALIGN)	\
		__attribute__((unused))					\
		__section("__u_boot_list_2_"#_list"_1");			\
	_type * tmp = (_type *)&start;					\
	asm("":"+r"(tmp));						\
	tmp;								\
})

/**
 * ll_entry_end() - Point after last entry of linker-generated array
 * @_type:	Data type of the entry
 * @_list:	Name of the list in which this entry is placed
 *		(with underscores instead of dots)
 *
 * This function returns ``(_type *)`` pointer after the very last entry of
 * a linker-generated array placed into subsection of __u_boot_list
 * section specified by _list argument.
 *
 * Since this macro defines an array end symbol, its leftmost index
 * must be 2 and its rightmost index must be 3.
 *
 * Example:
 *
 * ::
 *
 *   struct my_sub_cmd *msc = ll_entry_end(struct my_sub_cmd, cmd_sub);
 */
#define ll_entry_end(_type, _list)					\
({									\
	static char end[0] __aligned(4) __attribute__((unused))		\
		__section("__u_boot_list_2_"#_list"_3");			\
	_type * tmp = (_type *)&end;					\
	asm("":"+r"(tmp));						\
	tmp;								\
})
/**
 * ll_entry_count() - Return the number of elements in linker-generated array
 * @_type:	Data type of the entry
 * @_list:	Name of the list of which the number of elements is computed
 *
 * This function returns the number of elements of a linker-generated array
 * placed into subsection of __u_boot_list section specified by _list
 * argument. The result is of an unsigned int type.
 *
 * Example:
 *
 * ::
 *
 *   int i;
 *   const unsigned int count = ll_entry_count(struct my_sub_cmd, cmd_sub);
 *   struct my_sub_cmd *msc = ll_entry_start(struct my_sub_cmd, cmd_sub);
 *   for (i = 0; i < count; i++, msc++)
 *           printf("Entry %i, x=%i y=%i\n", i, msc->x, msc->y);
 */
#define ll_entry_count(_type, _list)					\
	({								\
		_type *start = ll_entry_start(_type, _list);		\
		_type *end = ll_entry_end(_type, _list);		\
		unsigned int _ll_result = end - start;			\
		_ll_result;						\
	})

/**
 * Declares a symbol that points to the start/end of the list.
 *
 * @_sym:	Arbitrary name for the symbol (to use later in the file)
 * @_type:	Data type of the entry
 * @_list:	Name of the list in which this entry is placed
 *
 * The name of the (new) symbol is arbitrary and can be anything that is not
 * already declared in the file where it appears. It is provided in _sym and
 * can then be used (later in the same file) within a data structure.
 * The _type and _list arguments must match those passed to ll_entry_start/end()
 *
 * Example:
 *
 * Here we want to record the start of each sub-command in a list. We have two
 * sub-commands, 'bob' and 'mary'.
 *
 * In bob.c::
 *
 *   ll_entry_declare(struct my_sub_cmd, bob_cmd, cmd_sub) = {...};
 *
 * In mary.c::
 *
 *   ll_entry_declare(struct my_sub_cmd, mary_cmd, cmd_sub) = {...};
 *
 * In a different file where we want a list the start of all sub-commands.
 * It is not possible to use ll_entry_start() in a data structure, due to its
 * use of code inside expressions - ({ ... }) - so this fails to compile:
 *
 * In sub_cmds.c::
 *
 *    struct cmd_sub *my_list[] = {
 *      ll_entry_start(cmd_sub, bob),
 *      ll_entry_start(cmd_sub, bob),
 *    };
 *
 * Instead, we can use::
 *
 *    ll_start_decl(bob, struct my_sub_cmd, cmd_sub);
 *    ll_start_decl(mary, struct my_sub_cmd, cmd_sub);
 *
 *     struct cmd_sub *my_list[] = {
 *       bob,
 *       mary,
 *     };
 *
 * So 'bob' is declared as symbol, a struct my_list * which points to the
 * start of the bob sub-commands. It is then used in my_list[]
 */
#define ll_start_decl(_sym, _type, _list)					\
	static _type _sym[0] __aligned(CONFIG_LINKER_LIST_ALIGN)	\
		__maybe_unused __section("__u_boot_list_2_" #_list "_1")

#define ll_end_decl(_sym, _type, _list)					\
	static _type _sym[0] __aligned(CONFIG_LINKER_LIST_ALIGN)	\
		__maybe_unused __section("__u_boot_list_2_" #_list "_3")

/**
 * ll_entry_get() - Retrieve entry from linker-generated array by name
 * @_type:	Data type of the entry
 * @_name:	Name of the entry
 * @_list:	Name of the list in which this entry is placed
 *
 * This function returns a pointer to a particular entry in linker-generated
 * array identified by the subsection of u_boot_list where the entry resides
 * and it's name.
 *
 * Example:
 *
 * ::
 *
 *   ll_entry_declare(struct my_sub_cmd, my_sub_cmd, cmd_sub) = {
 *           .x = 3,
 *           .y = 4,
 *   };
 *   ...
 *   struct my_sub_cmd *c = ll_entry_get(struct my_sub_cmd, my_sub_cmd, cmd_sub);
 */
#define ll_entry_get(_type, _name, _list)				\
	({								\
		extern _type _u_boot_list_2_##_list##_2_##_name;	\
		_type *_ll_result =					\
			&_u_boot_list_2_##_list##_2_##_name;		\
		_ll_result;						\
	})

/**
 * ll_entry_ref() - Get a reference to a linker-generated array entry
 *
 * Once an extern ll_entry_declare() has been used to declare the reference,
 * this macro allows the entry to be accessed.
 *
 * This is like ll_entry_get(), but without the extra code, so it is suitable
 * for putting into data structures.
 *
 * @_type: C type of the list entry, e.g. 'struct foo'
 * @_name: name of the entry
 * @_list: name of the list
 */
#define ll_entry_ref(_type, _name, _list)				\
	((_type *)&_u_boot_list_2_##_list##_2_##_name)

/**
 * ll_start() - Point to first entry of first linker-generated array
 * @_type:	Data type of the entry
 *
 * This function returns ``(_type *)`` pointer to the very first entry of
 * the very first linker-generated array.
 *
 * Since this macro defines the start of the linker-generated arrays,
 * its leftmost index must be 1.
 *
 * Example:
 *
 * ::
 *
 *   struct my_sub_cmd *msc = ll_start(struct my_sub_cmd);
 */
#define ll_start(_type)							\
({									\
	static char start[0] __aligned(4) __attribute__((unused))	\
		__section("__u_boot_list_1");				\
	_type * tmp = (_type *)&start;					\
	asm("":"+r"(tmp));						\
	tmp;								\
})

/**
 * ll_end() - Point after last entry of last linker-generated array
 * @_type:	Data type of the entry
 *
 * This function returns ``(_type *)`` pointer after the very last entry of
 * the very last linker-generated array.
 *
 * Since this macro defines the end of the linker-generated arrays,
 * its leftmost index must be 3.
 *
 * Example:
 *
 * ::
 *
 *   struct my_sub_cmd *msc = ll_end(struct my_sub_cmd);
 */
#define ll_end(_type)							\
({									\
	static char end[0] __aligned(4) __attribute__((unused))		\
		__section("__u_boot_list_3");				\
	_type * tmp = (_type *)&end;					\
	asm("":"+r"(tmp));						\
	tmp;								\
})

#endif /* __ASSEMBLY__ */

#endif	/* __LINKER_LISTS_H__ */
