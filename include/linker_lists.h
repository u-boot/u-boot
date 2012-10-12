/*
 * include/linker_lists.h
 *
 * Implementation of linker-generated arrays
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */
#ifndef __LINKER_LISTS_H__
#define __LINKER_LISTS_H__

/**
 * ll_entry_declare() - Declare linker-generated array entry
 * @_type:	Data type of the entry
 * @_name:	Name of the entry
 * @_section_u:	Subsection of u_boot_list in which this entry is placed
 *		(with underscores instead of dots, for name concatenation)
 * @_section_d:	Subsection of u_boot_list in which this entry is placed
 *		(with dots, for section concatenation)
 *
 * This macro declares a variable that is placed into a linker-generated
 * array. This is a basic building block for more advanced use of linker-
 * generated arrays. The user is expected to build their own macro wrapper
 * around this one.
 *
 * A variable declared using this macro must be compile-time initialized
 * and is as such placed into subsection of special section, .u_boot_list.
 * The subsection is specified by the _section_[u,d] parameter, see below.
 * The base name of the variable is _name, yet the actual variable is
 * declared as concatenation of
 *
 *   %_u_boot_list_ + @_section_u + _ + @_name
 *
 * which ensures name uniqueness. This variable shall never be refered
 * directly though.
 *
 * Special precaution must be made when using this macro:
 * 1) The _type must not contain the "static" keyword, otherwise the entry
 *    is not generated.
 *
 * 2) The @_section_u and @_section_d variables must match, the only difference
 *    is that in @_section_u is every dot "." character present in @_section_d
 *    replaced by a single underscore "_" character in @_section_u. The actual
 *    purpose of these parameters is to select proper subsection in the global
 *    .u_boot_list section.
 *
 * 3) In case a section is declared that contains some array elements AND a
 *    subsection of this section is declared and contains some elements, it is
 *    imperative that the elements are of the same type.
 *
 * 4) In case an outer section is declared that contains some array elements
 *    AND am inner subsection of this section is declared and contains some
 *    elements, then when traversing the outer section, even the elements of
 *    the inner sections are present in the array.
 *
 * Example:
 * ll_entry_declare(struct my_sub_cmd, my_sub_cmd, cmd_sub, cmd.sub) = {
 *         .x = 3,
 *         .y = 4,
 * };
 */
#define ll_entry_declare(_type, _name, _section_u, _section_d)		\
	_type _u_boot_list_##_section_u##_##_name __attribute__((	\
			unused,	aligned(4),				\
			section(".u_boot_list."#_section_d"."#_name)))

/**
 * ll_entry_start() - Point to first entry of linker-generated array
 * @_type:	Data type of the entry
 * @_section_u:	Subsection of u_boot_list in which this entry is placed
 *		(with underscores instead of dots)
 *
 * This function returns (_type *) pointer to the very first entry of a
 * linker-generated array placed into subsection of .u_boot_list section
 * specified by _section_u argument.
 *
 * Example:
 * struct my_sub_cmd *msc = ll_entry_start(struct my_sub_cmd, cmd_sub);
 */
#define ll_entry_start(_type, _section_u)				\
	({								\
		extern _type _u_boot_list_##_section_u##__start;	\
		_type *_ll_result = &_u_boot_list_##_section_u##__start;\
		_ll_result;						\
	})

/**
 * ll_entry_count() - Return the number of elements in linker-generated array
 * @_type:	Data type of the entry
 * @_section_u:	Subsection of u_boot_list in which this entry is placed
 *		(with underscores instead of dots)
 *
 * This function returns the number of elements of a linker-generated array
 * placed into subsection of .u_boot_list section specified by _section_u
 * argument. The result is of an unsigned int type.
 *
 * Example:
 * int i;
 * const unsigned int count = ll_entry_count(struct my_sub_cmd, cmd_sub);
 * struct my_sub_cmd *msc = ll_entry_start(struct my_sub_cmd, cmd_sub);
 * for (i = 0; i < count; i++, msc++)
 *         printf("Entry %i, x=%i y=%i\n", i, msc->x, msc->y);
 */
#define ll_entry_count(_type, _section_u)				\
	({								\
		extern _type _u_boot_list_##_section_u##__start;	\
		extern _type _u_boot_list_##_section_u##__end;		\
		unsigned int _ll_result =				\
			&_u_boot_list_##_section_u##__end -		\
			&_u_boot_list_##_section_u##__start;		\
		_ll_result;						\
	})


/**
 * ll_entry_get() - Retrieve entry from linker-generated array by name
 * @_type:	Data type of the entry
 * @_name:	Name of the entry
 * @_section_u:	Subsection of u_boot_list in which this entry is placed
 *		(with underscores instead of dots)
 *
 * This function returns a pointer to a particular entry in LG-array
 * identified by the subsection of u_boot_list where the entry resides
 * and it's name.
 *
 * Example:
 * ll_entry_declare(struct my_sub_cmd, my_sub_cmd, cmd_sub, cmd.sub) = {
 *         .x = 3,
 *         .y = 4,
 * };
 * ...
 * struct my_sub_cmd *c = ll_entry_get(struct my_sub_cmd, my_sub_cmd, cmd_sub);
 */
#define ll_entry_get(_type, _name, _section_u)				\
	({								\
		extern _type _u_boot_list_##_section_u##_##_name;	\
		_type *_ll_result = &_u_boot_list_##_section_u##_##_name;\
		_ll_result;						\
	})

#endif	/* __LINKER_LISTS_H__ */
