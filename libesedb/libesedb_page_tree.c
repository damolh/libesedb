/*
 * Page tree functions
 *
 * Copyright (c) 2009, Joachim Metz <forensics@hoffmannbv.nl>,
 * Hoffmann Investigations. All rights reserved.
 *
 * Refer to AUTHORS for acknowledgements.
 *
 * This software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this software.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <common.h>
#include <memory.h>
#include <types.h>

#include <liberror.h>
#include <libnotify.h>

#include "libesedb_data_definition.h"
#include "libesedb_debug.h"
#include "libesedb_definitions.h"
#include "libesedb_debug.h"
#include "libesedb_libfdatetime.h"
#include "libesedb_libuna.h"
#include "libesedb_list_type.h"
#include "libesedb_page.h"
#include "libesedb_page_tree.h"
#include "libesedb_string.h"

#ifdef OLD
#include "libesedb_column_definition.h"
#include "libesedb_index_definition.h"
#include "libesedb_long_value_definition.h"
#include "libesedb_table_definition.h"
#endif

#include "esedb_page_values.h"

/* Function to determine if there are zero bytes in a buffer
 * Returns 1 if the buffer contains zero bytes, 0 if not or -1 on error
 */
int libesedb_page_tree_buffer_contains_zero_bytes(
     uint8_t *buffer,
     size_t buffer_size,
     liberror_error_t **error )
{
	static char *function  = "libesedb_page_tree_buffer_contains_zero_bytes";
	size_t buffer_iterator = 0;

	if( buffer == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid buffer.",
		 function );

		return( -1 );
	}
	if( buffer_size > (size_t) SSIZE_MAX )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_VALUE_EXCEEDS_MAXIMUM,
		 "%s: invalid buffer size value exceeds maximum.",
		 function );

		return( -1 );
	}
	for( buffer_iterator = 0;
	     buffer_iterator < buffer_size;
	     buffer_iterator++ )
	{
		if( buffer[ buffer_iterator ] == 0 )
		{
			return( 1 );
		}
	}
	return( 0 );
}

/* Creates a page tree
 * Returns 1 if successful or -1 on error
 */
int libesedb_page_tree_initialize(
     libesedb_page_tree_t **page_tree,
     liberror_error_t **error )
{
	static char *function = "libesedb_page_tree_initialize";

	if( page_tree == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid page tree.",
		 function );

		return( -1 );
	}
	if( *page_tree == NULL )
	{
		*page_tree = (libesedb_page_tree_t *) memory_allocate(
		                             sizeof( libesedb_page_tree_t ) );

		if( *page_tree == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
			 "%s: unable to create page tree.",
			 function );

			return( -1 );
		}
		if( memory_set(
		     *page_tree,
		     0,
		     sizeof( libesedb_page_tree_t ) ) == NULL )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_MEMORY,
			 LIBERROR_MEMORY_ERROR_SET_FAILED,
			 "%s: unable to clear page tree.",
			 function );

			memory_free(
			 *page_tree );

			*page_tree = NULL;

			return( -1 );
		}
		if( libesedb_list_initialize(
		     &( ( *page_tree )->table_definition_list ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create table definition list.",
			 function );

			memory_free(
			 *page_tree );

			*page_tree = NULL;

			return( -1 );
		}
		if( libesedb_list_initialize(
		     &( ( *page_tree )->column_definition_list ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create column definition list.",
			 function );

			libesedb_list_free(
			 &( ( *page_tree )->table_definition_list ),
			 NULL,
			 NULL );
			memory_free(
			 *page_tree );

			*page_tree = NULL;

			return( -1 );
		}
		if( libesedb_list_initialize(
		     &( ( *page_tree )->index_definition_list ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create index definition list.",
			 function );

			libesedb_list_free(
			 &( ( *page_tree )->column_definition_list ),
			 NULL,
			 NULL );
			libesedb_list_free(
			 &( ( *page_tree )->table_definition_list ),
			 NULL,
			 NULL );
			memory_free(
			 *page_tree );

			*page_tree = NULL;

			return( -1 );
		}
		if( libesedb_list_initialize(
		     &( ( *page_tree )->long_value_definition_list ),
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create long value definition list.",
			 function );

			libesedb_list_free(
			 &( ( *page_tree )->index_definition_list ),
			 NULL,
			 NULL );
			libesedb_list_free(
			 &( ( *page_tree )->column_definition_list ),
			 NULL,
			 NULL );
			libesedb_list_free(
			 &( ( *page_tree )->table_definition_list ),
			 NULL,
			 NULL );
			memory_free(
			 *page_tree );

			*page_tree = NULL;

			return( -1 );
		}
	}
	return( 1 );
}

/* Frees page tree
 * Returns 1 if successful or -1 on error
 */
int libesedb_page_tree_free(
     libesedb_page_tree_t **page_tree,
     liberror_error_t **error )
{
	static char *function = "libesedb_page_tree_free";
	int result            = 1;

	if( page_tree == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid page tree.",
		 function );

		return( -1 );
	}
	if( *page_tree != NULL )
	{
		if( libesedb_list_free(
		     &( ( *page_tree )->table_definition_list ),
		     &libesedb_data_definition_free,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free table definition list.",
			 function );

			result = -1;
		}
		if( libesedb_list_free(
		     &( ( *page_tree )->column_definition_list ),
		     &libesedb_data_definition_free,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free column definition list.",
			 function );

			result = -1;
		}
		if( libesedb_list_free(
		     &( ( *page_tree )->index_definition_list ),
		     &libesedb_data_definition_free,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free index definition list.",
			 function );

			result = -1;
		}
		if( libesedb_list_free(
		     &( ( *page_tree )->long_value_definition_list ),
		     &libesedb_data_definition_free,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free long value definition list.",
			 function );

			result = -1;
		}
		memory_free(
		 *page_tree );

		*page_tree = NULL;
	}
	return( result );
}

/* Reads a page tree and its values
 * Returns 1 if successful or -1 on error
 */
int libesedb_page_tree_read(
     libesedb_page_tree_t *page_tree,
     libesedb_io_handle_t *io_handle,
     uint32_t father_data_page_number,
     liberror_error_t **error )
{
	libesedb_page_t *page = NULL;
	static char *function = "libesedb_page_tree_read";

	if( page_tree == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid page tree.",
		 function );

		return( -1 );
	}
	if( io_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid io handle.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	libnotify_verbose_printf(
	 "%s: reading page tree with FDP number\t\t: %" PRIu32 "\n",
	 function,
	 father_data_page_number );
#endif

	if( libesedb_page_initialize(
	     &page,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
		 "%s: unable to create page.",
		 function );

		return( -1 );
	}
	if( libesedb_page_read(
	     page,
	     io_handle,
	     father_data_page_number,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_IO,
		 LIBERROR_IO_ERROR_READ_FAILED,
		 "%s: unable to read page: %" PRIu32 ".",
		 function,
		 father_data_page_number );

		libesedb_page_free(
		 &page,
		 NULL );

		return( -1 );
	}
	if( ( page->flags & LIBESEDB_PAGE_FLAG_IS_LEAF ) == LIBESEDB_PAGE_FLAG_IS_LEAF )
	{
		if( libesedb_page_tree_read_leaf_page_values(
		     page_tree,
		     page,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read leaf page values.",
			 function );

			return( -1 );
		}
	}
	else if( ( page->flags & LIBESEDB_PAGE_FLAG_IS_ROOT ) == LIBESEDB_PAGE_FLAG_IS_ROOT )
	{
		if( libesedb_page_tree_read_father_data_page_values(
		     page_tree,
		     page,
		     io_handle,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read father data page values.",
			 function );

			return( -1 );
		}
	}
	if( libesedb_page_free(
	     &page,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
		 "%s: unable to free page.",
		 function );

		return( -1 );
	}
	return( 1 );
}

/* Reads the father data page values from the page
 * Returns 1 if successful or -1 on error
 */
int libesedb_page_tree_read_father_data_page_values(
     libesedb_page_tree_t *page_tree,
     libesedb_page_t *page,
     libesedb_io_handle_t *io_handle,
     liberror_error_t **error )
{
	libesedb_page_t *sub_page                = NULL;
	libesedb_page_value_t *page_value        = NULL;
	static char *function                    = "libesedb_page_tree_read_father_data_page_values";
	uint32_t child_page_number               = 0;
	uint32_t previous_child_page_number      = 0;
	uint32_t previous_next_child_page_number = 0;
	uint32_t required_flags                  = 0;
	uint32_t space_tree_page_number          = 0;
	uint32_t supported_flags                 = 0;
	uint16_t amount_of_page_values           = 0;
	uint16_t page_value_iterator             = 0;

#if defined( HAVE_DEBUG_OUTPUT )
	uint8_t *page_value_data                 = NULL;
	uint32_t extent_space                    = 0;
	uint32_t initial_amount_of_pages         = 0;
	uint16_t page_key_size                   = 0;
	uint16_t page_value_size                 = 0;
	uint32_t test                            = 0;
#endif

	if( page_tree == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid page tree.",
		 function );

		return( -1 );
	}
	if( page == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid page.",
		 function );

		return( -1 );
	}
	if( io_handle == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid io handle.",
		 function );

		return( -1 );
	}
	required_flags = LIBESEDB_PAGE_FLAG_IS_ROOT
	               | LIBESEDB_PAGE_FLAG_IS_PARENT;

	if( ( page->flags & required_flags ) != required_flags )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: missing required page flags: 0x%08" PRIx32 ".",
		 function,
		 page->flags );

		return( -1 );
	}
	supported_flags = required_flags
	                | LIBESEDB_PAGE_FLAG_IS_INDEX
	                | LIBESEDB_PAGE_FLAG_IS_LONG_VALUE
	                | LIBESEDB_PAGE_FLAG_IS_PRIMARY
	                | LIBESEDB_PAGE_FLAG_IS_NEW_RECORD_FORMAT;

	if( ( page->flags & ~supported_flags ) != 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported page flags: 0x%08" PRIx32 ".",
		 function,
		 page->flags );

		return( -1 );
	}
	if( page->previous_page_number != 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported previous page number: %" PRIu32 ".",
		 function,
		 page->previous_page_number );

		return( -1 );
	}
	if( page->next_page_number != 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported next page number: %" PRIu32 ".",
		 function,
		 page->next_page_number );

		return( -1 );
	}
	if( libesedb_page_get_amount_of_values(
	     page,
	     &amount_of_page_values,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve amount of page values.",
		 function );

		return( -1 );
	}
	if( libesedb_page_get_value(
	     page,
	     page_value_iterator,
	     &page_value,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve page value: %" PRIu16 ".",
		 function,
		 page_value_iterator );

		return( -1 );
	}
	if( page_value == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid page value.",
		 function );

		return( -1 );
	}
	endian_little_convert_32bit(
	 space_tree_page_number,
	 ( (esedb_father_data_page_header_t *) page_value->data )->space_tree_page_number );

	/* TODO handle the root page header */

#if defined( HAVE_DEBUG_OUTPUT )
	endian_little_convert_32bit(
	 initial_amount_of_pages,
	 ( (esedb_father_data_page_header_t *) page_value->data )->initial_amount_of_pages );
	libnotify_verbose_printf(
	 "%s: header initial amount of pages\t: %" PRIu32 "\n",
	 function,
	 initial_amount_of_pages );

	endian_little_convert_32bit(
	 test,
	 ( (esedb_father_data_page_header_t *) page_value->data )->parent_father_data_page_number );
	libnotify_verbose_printf(
	 "%s: header parent FDP number\t: %" PRIu32 "\n",
	 function,
	 test );

	endian_little_convert_32bit(
	 extent_space,
	 ( (esedb_father_data_page_header_t *) page_value->data )->extent_space );
	libnotify_verbose_printf(
	 "%s: header extent space\t\t: %" PRIu32 "\n",
	 function,
	 extent_space );

	libnotify_verbose_printf(
	 "%s: header space tree page number\t: %" PRIu32 " (0x%08" PRIx32 ")\n",
	 function,
	 space_tree_page_number,
	 space_tree_page_number );

	libnotify_verbose_printf(
	 "%s: header primary extent\t\t: %" PRIu32 "-%c\n",
	 function,
	 initial_amount_of_pages,
	 ( extent_space == 0 ? 's' : 'm' ) );

	libnotify_verbose_printf(
	 "\n" );
#endif

	/* Read the space tree pages
	 */
	if( ( space_tree_page_number > 0 )
	 && ( space_tree_page_number < 0xffffff00UL ) )
	{
		/* Read the owned pages space tree page
		 */
		if( libesedb_page_initialize(
		     &sub_page,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create space tree page.",
			 function );

			return( -1 );
		}
		if( libesedb_page_read(
		     sub_page,
		     io_handle,
		     space_tree_page_number,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read space tree page: %" PRIu32 ".",
			 function,
			 space_tree_page_number );

			libesedb_page_free(
			 &sub_page,
			 NULL );

			return( -1 );
		}
		if( page->father_data_page_object_identifier != sub_page->father_data_page_object_identifier )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
			 "%s: mismatch in father data page object identifier (%" PRIu32 " != %" PRIu32 ").",
			 function,
			 page->father_data_page_object_identifier,
			 sub_page->father_data_page_object_identifier );

			libesedb_page_free(
			 &sub_page,
			 NULL );

			return( -1 );
		}
		if( libesedb_page_tree_read_space_tree_page_values(
		     page_tree,
		     sub_page,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read space tree page values.",
			 function );

			libesedb_page_free(
			 &sub_page,
			 NULL );

			return( -1 );
		}
		if( libesedb_page_free(
		     &sub_page,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free space tree page.",
			 function );

			return( -1 );
		}

		/* Read the available pages space tree page
		 */
		space_tree_page_number += 1;

		if( libesedb_page_initialize(
		     &sub_page,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
			 "%s: unable to create space tree page.",
			 function );

			return( -1 );
		}
		if( libesedb_page_read(
		     sub_page,
		     io_handle,
		     space_tree_page_number,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read space tree page: %" PRIu32 ".",
			 function,
			 space_tree_page_number );

			libesedb_page_free(
			 &sub_page,
			 NULL );

			return( -1 );
		}
		if( page->father_data_page_object_identifier != sub_page->father_data_page_object_identifier )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
			 "%s: mismatch in father data page object identifier (%" PRIu32 " != %" PRIu32 ").",
			 function,
			 page->father_data_page_object_identifier,
			 sub_page->father_data_page_object_identifier );

			libesedb_page_free(
			 &sub_page,
			 NULL );

			return( -1 );
		}
		if( libesedb_page_tree_read_space_tree_page_values(
		     page_tree,
		     sub_page,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_IO,
			 LIBERROR_IO_ERROR_READ_FAILED,
			 "%s: unable to read space tree page values.",
			 function );

			libesedb_page_free(
			 &sub_page,
			 NULL );

			return( -1 );
		}
		if( libesedb_page_free(
		     &sub_page,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
			 "%s: unable to free space tree page.",
			 function );

			return( -1 );
		}
	}
	/* Read the page values
	 */
	for( page_value_iterator = 1;
	     page_value_iterator < amount_of_page_values;
	     page_value_iterator++ )
	{
		if( libesedb_page_get_value(
		     page,
		     page_value_iterator,
		     &page_value,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve page value: %" PRIu16 ".",
			 function,
			 page_value_iterator );

			return( -1 );
		}
		/* TODO handle leaf page values */

		page_value_data = page_value->data;
		page_value_size = page_value->size;

		if( ( page_value->flags & 0x7 ) != 0 )
		{
			libnotify_verbose_printf(
			 "MARKER: unsupported page flags: 0x%02" PRIx8 "\n",
			 page_value->flags );
		}
#if defined( HAVE_DEBUG_OUTPUT )
		endian_little_convert_16bit(
		 page_key_size,
		 page_value_data );

		page_value_data += 2;
		page_value_size -= 2;

		libnotify_verbose_printf(
		 "%s: value: %03d highest key size\t: %" PRIu16 "\n",
		 function,
		 page_value_iterator,
		 page_key_size );

		if( page_key_size > page_value_size )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_RANGE,
			 "%s: page key size exceeds page value size.",
			 function );

			return( -1 );
		}
		libnotify_verbose_printf(
		 "%s: value: %03d highest key value\t: ",
		 function,
		 page_value_iterator );

		while( page_key_size > 0 )
		{
			libnotify_verbose_printf(
			 "%02" PRIx8 " ",
			 *page_value_data );

			page_value_data++;
			page_value_size--;
			page_key_size--;
		}
		libnotify_verbose_printf(
		 "\n" );

		endian_little_convert_32bit(
		 child_page_number,
		 page_value_data );

		libnotify_verbose_printf(
		 "%s: value: %03d child page number\t: %" PRIu32 "\n",
		 function,
		 page_value_iterator,
		 child_page_number );
		libnotify_verbose_printf(
		 "\n" );
#endif

		/* TODO can an upper bound be determined ?
		 */
		if( child_page_number < 0x117f02 )
		{
			if( libesedb_page_initialize(
			     &sub_page,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
				 "%s: unable to create child page.",
				 function );

				return( -1 );
			}
			if( libesedb_page_read(
			     sub_page,
			     io_handle,
			     child_page_number,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_IO,
				 LIBERROR_IO_ERROR_READ_FAILED,
				 "%s: unable to read child page: %" PRIu32 ".",
				 function,
				 child_page_number );

				libesedb_page_free(
				 &sub_page,
				 NULL );

				return( -1 );
			}
			if( page->father_data_page_object_identifier != sub_page->father_data_page_object_identifier )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
				 "%s: mismatch in father data page object identifier (%" PRIu32 " != %" PRIu32 ").",
				 function,
				 page->father_data_page_object_identifier,
				 sub_page->father_data_page_object_identifier );

				libesedb_page_free(
				 &sub_page,
				 NULL );

				return( -1 );
			}
			if( page_value_iterator > 1 )
			{
				if( sub_page->page_number != previous_next_child_page_number )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_RUNTIME,
					 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
					 "%s: mismatch in child page number (%" PRIu32 " != %" PRIu32 ").",
					 function,
					 previous_next_child_page_number,
					 sub_page->page_number );

					libesedb_page_free(
					 &sub_page,
					 NULL );

					return( -1 );
				}
				if( sub_page->previous_page_number != previous_child_page_number )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_RUNTIME,
					 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
					 "%s: mismatch in previous child page number (%" PRIu32 " != %" PRIu32 ").",
					 function,
					 previous_child_page_number,
					 sub_page->previous_page_number );

					libesedb_page_free(
					 &sub_page,
					 NULL );

					return( -1 );
				}
			}
			if( page_value_iterator == 1 )
			{
				if( sub_page->previous_page_number != 0 )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_RUNTIME,
					 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
					 "%s: mismatch in previous child page number (%" PRIu32 " != %" PRIu32 ").",
					 function,
					 0,
					 sub_page->previous_page_number );

					libesedb_page_free(
					 &sub_page,
					 NULL );

					return( -1 );
				}
			}
			if( page_value_iterator == ( amount_of_page_values - 1 ) )
			{
				if( sub_page->next_page_number != 0 )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_RUNTIME,
					 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
					 "%s: mismatch in next child page number (%" PRIu32 " != %" PRIu32 ").",
					 function,
					 0,
					 sub_page->previous_page_number );

					libesedb_page_free(
					 &sub_page,
					 NULL );

					return( -1 );
				}
			}
			previous_child_page_number      = sub_page->page_number;
			previous_next_child_page_number = sub_page->next_page_number;

			if( libesedb_page_tree_read_leaf_page_values(
			     page_tree,
			     sub_page,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_IO,
				 LIBERROR_IO_ERROR_READ_FAILED,
				 "%s: unable to read leaf page values.",
				 function );

				libesedb_page_free(
				 &sub_page,
				 NULL );

				return( -1 );
			}
			if( libesedb_page_free(
			     &sub_page,
			     error ) != 1 )
			{
				liberror_error_set(
				 error,
				 LIBERROR_ERROR_DOMAIN_RUNTIME,
				 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
				 "%s: unable to free child page.",
				 function );

				return( -1 );
			}
		}
	}
#if defined( HAVE_DEBUG_OUTPUT )
	libnotify_verbose_printf(
	 "\n" );
#endif

	return( 1 );
}

/* Reads the space tree page values from the page
 * Returns 1 if successful or -1 on error
 */
int libesedb_page_tree_read_space_tree_page_values(
     libesedb_page_tree_t *page_tree,
     libesedb_page_t *page,
     liberror_error_t **error )
{
	libesedb_page_value_t *page_value = NULL;
	static char *function             = "libesedb_page_tree_read_space_tree_page_values";
	uint32_t required_flags           = 0;
	uint32_t supported_flags          = 0;
	uint16_t amount_of_page_values    = 0;
	uint16_t page_key_size            = 0;
	uint16_t page_value_iterator      = 0;

#if defined( HAVE_DEBUG_OUTPUT )
	uint32_t test                     = 0;
	uint32_t total_amount_of_pages    = 0;
#endif

	if( page_tree == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid page tree.",
		 function );

		return( -1 );
	}
	required_flags = LIBESEDB_PAGE_FLAG_IS_ROOT
	               | LIBESEDB_PAGE_FLAG_IS_LEAF
	               | LIBESEDB_PAGE_FLAG_IS_SPACE_TREE;

	if( ( page->flags & required_flags ) != required_flags )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: missing required page flags: 0x%08" PRIx32 ".",
		 function,
		 page->flags );

		return( -1 );
	}
	supported_flags = required_flags
	                | LIBESEDB_PAGE_FLAG_IS_INDEX
	                | LIBESEDB_PAGE_FLAG_IS_LONG_VALUE
	                | LIBESEDB_PAGE_FLAG_IS_PRIMARY
	                | LIBESEDB_PAGE_FLAG_IS_NEW_RECORD_FORMAT;

	if( ( page->flags & ~supported_flags ) != 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported page flags: 0x%08" PRIx32 ".",
		 function,
		 page->flags );

		return( -1 );
	}
	if( page->previous_page_number != 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported previous page number: %" PRIu32 ".",
		 function,
		 page->previous_page_number );

		return( -1 );
	}
	if( page->next_page_number != 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported next page number: %" PRIu32 ".",
		 function,
		 page->next_page_number );

		return( -1 );
	}
	if( libesedb_page_get_amount_of_values(
	     page,
	     &amount_of_page_values,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve amount of page values.",
		 function );

		return( -1 );
	}
	if( libesedb_page_get_value(
	     page,
	     page_value_iterator,
	     &page_value,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve page value: %" PRIu16 ".",
		 function,
		 page_value_iterator );

		return( -1 );
	}
	if( page_value == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid page value.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	libnotify_verbose_printf(
	 "%s: header:\n",
	 function,
	 page_value_iterator );
	libnotify_verbose_print_data(
	 page_value->data,
	 page_value->size );
#endif

	if( ( page_value->data[  0 ] != 0 )
	 || ( page_value->data[  1 ] != 0 )
	 || ( page_value->data[  2 ] != 0 )
	 || ( page_value->data[  3 ] != 0 )
	 || ( page_value->data[  4 ] != 0 )
	 || ( page_value->data[  5 ] != 0 )
	 || ( page_value->data[  6 ] != 0 )
	 || ( page_value->data[  7 ] != 0 )
	 || ( page_value->data[  8 ] != 0 )
	 || ( page_value->data[  9 ] != 0 )
	 || ( page_value->data[ 10 ] != 0 )
	 || ( page_value->data[ 11 ] != 0 )
	 || ( page_value->data[ 12 ] != 0 )
	 || ( page_value->data[ 13 ] != 0 )
	 || ( page_value->data[ 14 ] != 0 )
	 || ( page_value->data[ 15 ] != 0 ) )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported header.",
		 function );

		return( -1 );
	}
	/* TODO handle the space tree page header */

	for( page_value_iterator = 1;
	     page_value_iterator < amount_of_page_values;
	     page_value_iterator++ )
	{
		if( libesedb_page_get_value(
		     page,
		     page_value_iterator,
		     &page_value,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve page value: %" PRIu16 ".",
			 function,
			 page_value_iterator );

			return( -1 );
		}
		if( ( page_value->flags & 0x05 ) != 0 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
			 "%s: unsupported page value flags: 0x%02" PRIx8 ".",
			 function,
			 page_value->flags );

			return( -1 );
		}
		if( page_value->size != sizeof( esedb_space_tree_page_entry_t ) )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
			 "%s: unsupported page value size: %" PRIzd ".",
			 function,
			 page_value->size );

			return( -1 );
		}
		endian_little_convert_16bit(
		 page_key_size,
		 ( (esedb_space_tree_page_entry_t *) page_value->data )->key_size );

		if( page_key_size != 4 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
			 "%s: unsupported page key size: %" PRIu16 ".",
			 function,
			 page_key_size);

			return( -1 );
		}
		/* TODO handle the space tree page values */

#if defined( HAVE_DEBUG_OUTPUT )
		libnotify_verbose_printf(
		 "%s: value: %03d key size\t\t: %" PRIu16 "\n",
		 function,
		 page_value_iterator,
		 page_key_size );

		endian_little_convert_32bit(
		 test,
		 ( (esedb_space_tree_page_entry_t *) page_value->data )->last_page_number );
		libnotify_verbose_printf(
		 "%s: value: %03d key value\t\t: %" PRIu32 " (0x%08" PRIx32 ")\n",
		 function,
		 page_value_iterator,
		 test,
		 test );

		endian_little_convert_32bit(
		 test,
		 ( (esedb_space_tree_page_entry_t *) page_value->data )->amount_of_pages );
		libnotify_verbose_printf(
		 "%s: value: %03d amount of pages\t: %" PRIu32 "\n",
		 function,
		 page_value_iterator,
		 test );

		libnotify_verbose_printf(
		 "\n" );

		if( ( page_value->flags & 0x02 ) == 0 )
		{
			total_amount_of_pages += test;
		}
#endif
	}

#if defined( HAVE_DEBUG_OUTPUT )
	libnotify_verbose_printf(
	 "%s: total amount of pages\t\t: %" PRIu32 "\n",
	 function,
	 total_amount_of_pages );

	libnotify_verbose_printf(
	 "\n" );
#endif

	return( 1 );
}

/* Reads the leaf page values from the page
 * Returns 1 if successful or -1 on error
 */
int libesedb_page_tree_read_leaf_page_values(
     libesedb_page_tree_t *page_tree,
     libesedb_page_t *page,
     liberror_error_t **error )
{
#ifdef OLD
	libesedb_column_definition_t *column_definition         = NULL;
	libesedb_index_definition_t *index_definition           = NULL;
	libesedb_long_value_definition_t *long_value_definition = NULL;
	libesedb_table_definition_t *table_definition           = NULL;
	int result                                              = 0;
#endif

	libesedb_data_definition_t *data_definition             = NULL;
	libesedb_page_value_t *page_value                       = NULL;
	static char *function                                   = "libesedb_page_tree_read_leaf_page_values";
	uint32_t required_flags                                 = 0;
	uint32_t supported_flags                                = 0;
	uint16_t amount_of_page_values                          = 0;
	uint16_t page_value_iterator                            = 0;

#if defined( HAVE_DEBUG_OUTPUT )
	uint8_t *page_value_data                                = NULL;
	uint16_t page_key_size                                  = 0;
	uint16_t page_value_size                                = 0;
	uint16_t record_number                                  = 0;

#ifdef OLD
	libesedb_character_t date_time_string[ 22 ];

	libfdatetime_filetime_t *filetime                       = NULL;
	uint8_t *string                                         = NULL;
	size_t string_size                                      = 0;
	uint32_t value_32bit                                    = 0;
	uint16_t definition_flags                               = 0;
	uint16_t definition_size                                = 0;
	uint16_t definition_type                                = 0;
	uint16_t name_size                                      = 0;
	uint16_t value_16bit                                    = 0;
	uint8_t multi_value_iterator                            = 0;
#endif
#endif

	if( page_tree == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid page tree.",
		 function );

		return( -1 );
	}
	if( page == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_ARGUMENTS,
		 LIBERROR_ARGUMENT_ERROR_INVALID_VALUE,
		 "%s: invalid page.",
		 function );

		return( -1 );
	}
	required_flags = LIBESEDB_PAGE_FLAG_IS_LEAF;

	if( ( page->flags & required_flags ) != required_flags )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: missing required page flags: 0x%08" PRIx32 ".",
		 function,
		 page->flags );

		return( -1 );
	}
	supported_flags = required_flags
	                | LIBESEDB_PAGE_FLAG_IS_ROOT
	                | LIBESEDB_PAGE_FLAG_IS_INDEX
	                | LIBESEDB_PAGE_FLAG_IS_LONG_VALUE
	                | LIBESEDB_PAGE_FLAG_IS_PRIMARY
	                | LIBESEDB_PAGE_FLAG_IS_NEW_RECORD_FORMAT;

	if( ( page->flags & ~supported_flags ) != 0 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_UNSUPPORTED_VALUE,
		 "%s: unsupported page flags: 0x%08" PRIx32 ".",
		 function,
		 page->flags );

		return( -1 );
	}
	if( libesedb_page_get_amount_of_values(
	     page,
	     &amount_of_page_values,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve amount of page values.",
		 function );

		return( -1 );
	}
	if( libesedb_page_get_value(
	     page,
	     page_value_iterator,
	     &page_value,
	     error ) != 1 )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_GET_FAILED,
		 "%s: unable to retrieve page value: %" PRIu16 ".",
		 function,
		 page_value_iterator );

		return( -1 );
	}
	if( page_value == NULL )
	{
		liberror_error_set(
		 error,
		 LIBERROR_ERROR_DOMAIN_RUNTIME,
		 LIBERROR_RUNTIME_ERROR_VALUE_MISSING,
		 "%s: invalid page value.",
		 function );

		return( -1 );
	}
#if defined( HAVE_DEBUG_OUTPUT )
	page_value_data = page_value->data;
	page_value_size = page_value->size;

	libnotify_verbose_printf(
	 "%s: value: %03d value:\n",
	 function,
	 page_value_iterator );
	libnotify_verbose_print_data(
	 page_value_data,
	 page_value->size );

	libnotify_verbose_printf(
	 "%s: header (record key)\t\t\t\t: ",
	 function,
	 page_value_iterator );

	while( page_value_size > 0 )
	{
		libnotify_verbose_printf(
		 "%02" PRIx8 " ",
		 *page_value_data );

		page_value_data++;
		page_value_size--;
	}
	libnotify_verbose_printf(
	 "\n" );
	libnotify_verbose_printf(
	 "\n" );
#endif

	/* TODO handle the leaf page header */

	for( page_value_iterator = 1;
	     page_value_iterator < amount_of_page_values;
	     page_value_iterator++ )
	{
		if( libesedb_page_get_value(
		     page,
		     page_value_iterator,
		     &page_value,
		     error ) != 1 )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_GET_FAILED,
			 "%s: unable to retrieve page value: %" PRIu16 ".",
			 function,
			 page_value_iterator );

			return( -1 );
		}
		/* TODO handle the leaf page values */

		page_value_data = page_value->data;
		page_value_size = page_value->size;

#if defined( HAVE_DEBUG_OUTPUT )
		libnotify_verbose_printf(
		 "%s: value: %03d value:\n",
		 function,
		 page_value_iterator );
		libnotify_verbose_print_data(
		 page_value_data,
		 page_value->size );

		libnotify_verbose_printf(
		 "%s: value: %03d page tag flags\t\t\t: ",
		 function,
		 page_value_iterator );
		libesedb_debug_print_page_tag_flags(
		 page_value->flags );
		libnotify_verbose_printf(
		 "\n" );

		if( ( page_value->flags & 0x04 ) == 0x04 )
		{
			endian_little_convert_16bit(
			 record_number,
			 page_value_data );

			page_value_data += 2;
			page_value_size -= 2;

			libnotify_verbose_printf(
			 "%s: value: %03d key type\t\t\t\t: 0x%04" PRIx32 " (%" PRIu32 ")\n",
			 function,
			 page_value_iterator,
			 record_number,
			 record_number );
		}
		else
		{
			record_number = 0;
		}
		endian_little_convert_16bit(
		 page_key_size,
		 page_value_data );

		page_value_data += 2;
		page_value_size -= 2;

		libnotify_verbose_printf(
		 "%s: value: %03d key size\t\t\t\t: %" PRIu16 "\n",
		 function,
		 page_value_iterator,
		 page_key_size );

		if( page_key_size > page_value_size )
		{
			liberror_error_set(
			 error,
			 LIBERROR_ERROR_DOMAIN_RUNTIME,
			 LIBERROR_RUNTIME_ERROR_VALUE_OUT_OF_RANGE,
			 "%s: page key size exceeds page value size.",
			 function );

			return( -1 );
		}
		libnotify_verbose_printf(
		 "%s: value: %03d key value\t\t\t\t: ",
		 function,
		 page_value_iterator );

		while( page_key_size > 0 )
		{
			libnotify_verbose_printf(
			 "%02" PRIx8 " ",
			 *page_value_data );

			page_value_data++;
			page_value_size--;
			page_key_size--;
		}
		libnotify_verbose_printf(
		 "\n" );

		if( ( page->flags & LIBESEDB_PAGE_FLAG_IS_INDEX ) == LIBESEDB_PAGE_FLAG_IS_INDEX )
		{
			libnotify_verbose_printf(
			 "%s: value: %03d index value\t\t\t: ",
			 function,
			 page_value_iterator );

			while( page_value_size > 0 )
			{
				libnotify_verbose_printf(
				 "%02" PRIx8 " ",
				 *page_value_data );

				page_value_data++;
				page_value_size--;
			}
			libnotify_verbose_printf(
			 "\n" );
			libnotify_verbose_printf(
			 "\n" );
		}
		else if( ( page->flags & LIBESEDB_PAGE_FLAG_IS_LONG_VALUE ) == LIBESEDB_PAGE_FLAG_IS_LONG_VALUE )
		{
			libnotify_verbose_printf(
			 "%s: value: %03d long value:\n",
			 function,
			 page_value_iterator );
			libnotify_verbose_print_data(
			 page_value_data,
			 page_value_size );
		}
		else
		{
			if( ( page->father_data_page_object_identifier == LIBESEDB_FDP_OBJECT_IDENTIFIER_CATALOG )
			 || ( page->father_data_page_object_identifier == LIBESEDB_FDP_OBJECT_IDENTIFIER_CATALOG_BACKUP ) )
			{
				data_definition = NULL;

				if( libesedb_data_definition_initialize(
				     &data_definition,
				     error ) != 1 )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_RUNTIME,
					 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
					 "%s: unable to create data definition.",
					 function );

					return( -1 );
				}
				if( libesedb_data_definition_read(
				     data_definition,
				     page_value_data,
				     page_value_size,
				     error ) != 1 )
				{
					liberror_error_set(
					 error,
					 LIBERROR_ERROR_DOMAIN_IO,
					 LIBERROR_IO_ERROR_READ_FAILED,
					 "%s: unable to read page value: %d data definition.",
					 function,
					 page_value_iterator );

					libesedb_data_definition_free(
					 (intptr_t *) data_definition,
					 NULL );

					data_definition = NULL;

					return( -1 );
				}
			}
#ifdef OLD
			/* TODO make sure the page value size >= 4 */

			endian_little_convert_16bit(
			 definition_flags,
			 ( (esedb_definition_header_t * ) page_value_data )->flags );

			libnotify_verbose_printf(
			 "%s: value: %03d definition flags\t\t\t: 0x%04" PRIx32 "\n",
			 function,
			 page_value_iterator,
			 definition_flags );

			endian_little_convert_16bit(
			 definition_size,
			 ( (esedb_definition_header_t * ) page_value_data )->size );

			libnotify_verbose_printf(
			 "%s: value: %03d definition size\t\t\t: %" PRIu32 "\n",
			 function,
			 page_value_iterator,
			 definition_size );

			page_value_data += sizeof( esedb_definition_header_t );
			page_value_size -= sizeof( esedb_definition_header_t );
			definition_size -= sizeof( esedb_definition_header_t );


			if( definition_flags == 0x7f01 )
			{
				endian_little_convert_32bit(
				 value_32bit,
				 page_value_data );

				page_value_data += 4;
				page_value_size -= 4;
				definition_size -= 4;

				libnotify_verbose_printf(
				 "%s: value: %03d parent identifier\t\t\t: 0x%08" PRIx32 " (%" PRIu32 ")\n",
				 function,
				 page_value_iterator,
				 value_32bit,
				 value_32bit );

				libnotify_verbose_printf(
				 "%s: value: %03d unknown1\t\t\t\t: 0x%02" PRIx8 "\n",
				 function,
				 page_value_iterator,
				 *page_value_data );

				page_value_data += 1;
				page_value_size -= 1;
				definition_size -= 1;
			}
			else if( ( definition_flags == 0x8009 )
			      && ( definition_size  == ( 27 - sizeof( esedb_definition_header_t ) ) ) )
			{
				libnotify_verbose_printf(
				 "MARKER: unsupported definition size: %" PRIu16 " for definition flags: 0x%04" PRIx16 "\n",
				 definition_size + sizeof( esedb_definition_header_t ),
				 definition_flags );
			}
			else if( ( definition_flags == 0x8007 )
			      || ( definition_flags == 0x8008 )
			      || ( definition_flags == 0x8009 )
			      || ( definition_flags == 0x8013 )
			      || ( definition_flags == 0x8209 )
			      || ( definition_flags == 0x8309 )
			      || ( definition_flags == 0x840a )
			      || ( definition_flags == 0x880a ) )
			{
/*
				libnotify_verbose_printf(
				 "%s: value: %03d definition data:\n",
				 function,
				 page_value_iterator );
				libnotify_verbose_print_data(
				 page_value_data,
				 definition_size );
*/

				endian_little_convert_32bit(
				 value_32bit,
				 page_value_data );

				page_value_data += 4;
				page_value_size -= 4;
				definition_size -= 4;

				libnotify_verbose_printf(
				 "%s: value: %03d parent identifier\t\t\t: 0x%08" PRIx32 " (%" PRIu32 ")\n",
				 function,
				 page_value_iterator,
				 value_32bit,
				 value_32bit );

				/* TODO some check to see if this value is present
				 */
				if( definition_flags != 0x8013 )
				{
					endian_little_convert_16bit(
					 definition_type,
					 page_value_data );

					page_value_data += 2;
					page_value_size -= 2;
					definition_size -= 2;

					libnotify_verbose_printf(
					 "%s: value: %03d definition type\t\t\t: 0x%04" PRIx16 " ",
					 function,
					 page_value_iterator,
					 definition_type );
					libesedb_debug_print_page_value_definition_type(
					 definition_type );
					libnotify_verbose_printf(
					 "\n" );
				}
				if( definition_type == 0x0000 )
				{
					libnotify_verbose_printf(
					 "%s: value: %03d definition data:\n",
					 function,
					 page_value_iterator );
					libnotify_verbose_print_data(
					 page_value_data,
					 definition_size );

					if( libfdatetime_filetime_initialize(
					     &filetime,
					     error ) != 1 )
					{
						liberror_error_set(
						 error,
						 LIBERROR_ERROR_DOMAIN_RUNTIME,
						 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
						 "%s: unable to create filetime.",
						 function );

						return( -1 );
					}
					if( ( definition_flags == 0x8009 )
					 || ( definition_flags == 0x8209 ) )
					{
						endian_little_convert_16bit(
						 value_16bit,
						 page_value_data );

						page_value_data += 2;
						page_value_size -= 2;
						definition_size -= 2;

						libnotify_verbose_printf(
						 "%s: value: %03d unknown2\t\t\t\t: 0x%04" PRIx16 "\n",
						 function,
						 page_value_iterator,
						 value_16bit );

						endian_little_convert_32bit(
						 value_32bit,
						 page_value_data );

						page_value_data += 4;
						page_value_size -= 4;
						definition_size -= 4;

						libnotify_verbose_printf(
						 "%s: value: %03d unknown3\t\t\t\t: 0x%08" PRIx32 "\n",
						 function,
						 page_value_iterator,
						 value_32bit );

						if( libfdatetime_filetime_copy_from_byte_stream(
						     filetime,
						     page_value_data,
						     8,
						     LIBFDATETIME_ENDIAN_BIG,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_CONVERSION,
							 LIBERROR_CONVERSION_ERROR_GENERIC,
							 "%s: unable to create creation time.",
							 function );

							libfdatetime_filetime_free(
							 &filetime,
							 NULL );

							return( -1 );
						}
						page_value_data += 8;
						page_value_size -= 8;
						definition_size -= 8;

						if( libfdatetime_filetime_copy_to_string(
						     filetime,
						     date_time_string,
						     22,
						     LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME,
						     LIBFDATETIME_DATE_TIME_FORMAT_CTIME,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_CONVERSION,
							 LIBERROR_CONVERSION_ERROR_GENERIC,
							 "%s: unable to create filetime string.",
							 function );

							libfdatetime_filetime_free(
							 &filetime,
							 NULL );

							return( -1 );
						}
						libnotify_verbose_printf(
						 "%s: value: %03d unknown4\t\t\t\t: %s\n",
						 function,
						 page_value_iterator,
						 (char *) date_time_string );

						endian_big_convert_32bit(
						 value_32bit,
						 page_value_data );

						page_value_data += 4;
						page_value_size -= 4;
						definition_size -= 4;

						libnotify_verbose_printf(
						 "%s: value: %03d file size (upper 32-bit)\t\t: %" PRIu32 "\n",
						 function,
						 page_value_iterator,
						 value_32bit );

						endian_big_convert_32bit(
						 value_32bit,
						 page_value_data );

						page_value_data += 4;
						page_value_size -= 4;
						definition_size -= 4;

						libnotify_verbose_printf(
						 "%s: value: %03d file size (lower 32-bit)\t\t: %" PRIu32 " bytes\n",
						 function,
						 page_value_iterator,
						 value_32bit );

						endian_big_convert_32bit(
						 value_32bit,
						 page_value_data );

						page_value_data += 4;
						page_value_size -= 4;
						definition_size -= 4;

						libnotify_verbose_printf(
						 "%s: value: %03d file attribute flags\t\t: ",
						 function,
						 page_value_iterator );
						libesedb_debug_print_file_attribute_flags(
						 value_32bit );
						libnotify_verbose_printf(
						 "\n" );

						if( libfdatetime_filetime_copy_from_byte_stream(
						     filetime,
						     page_value_data,
						     8,
						     LIBFDATETIME_ENDIAN_BIG,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_CONVERSION,
							 LIBERROR_CONVERSION_ERROR_GENERIC,
							 "%s: unable to create creation time.",
							 function );

							libfdatetime_filetime_free(
							 &filetime,
							 NULL );

							return( -1 );
						}
						page_value_data += 8;
						page_value_size -= 8;
						definition_size -= 8;

						if( libfdatetime_filetime_copy_to_string(
						     filetime,
						     date_time_string,
						     22,
						     LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME,
						     LIBFDATETIME_DATE_TIME_FORMAT_CTIME,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_CONVERSION,
							 LIBERROR_CONVERSION_ERROR_GENERIC,
							 "%s: unable to create filetime string.",
							 function );

							libfdatetime_filetime_free(
							 &filetime,
							 NULL );

							return( -1 );
						}
						libnotify_verbose_printf(
						 "%s: value: %03d file creation time\t\t\t: %s\n",
						 function,
						 page_value_iterator,
						 (char *) date_time_string );

						if( libfdatetime_filetime_copy_from_byte_stream(
						     filetime,
						     page_value_data,
						     8,
						     LIBFDATETIME_ENDIAN_BIG,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_CONVERSION,
							 LIBERROR_CONVERSION_ERROR_GENERIC,
							 "%s: unable to create creation time.",
							 function );

							libfdatetime_filetime_free(
							 &filetime,
							 NULL );

							return( -1 );
						}
						page_value_data += 8;
						page_value_size -= 8;
						definition_size -= 8;

						if( libfdatetime_filetime_copy_to_string(
						     filetime,
						     date_time_string,
						     22,
						     LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME,
						     LIBFDATETIME_DATE_TIME_FORMAT_CTIME,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_CONVERSION,
							 LIBERROR_CONVERSION_ERROR_GENERIC,
							 "%s: unable to create filetime string.",
							 function );

							libfdatetime_filetime_free(
							 &filetime,
							 NULL );

							return( -1 );
						}
						libnotify_verbose_printf(
						 "%s: value: %03d file modification time\t\t: %s\n",
						 function,
						 page_value_iterator,
						 (char *) date_time_string );

						if( libfdatetime_filetime_copy_from_byte_stream(
						     filetime,
						     page_value_data,
						     8,
						     LIBFDATETIME_ENDIAN_BIG,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_CONVERSION,
							 LIBERROR_CONVERSION_ERROR_GENERIC,
							 "%s: unable to create creation time.",
							 function );

							libfdatetime_filetime_free(
							 &filetime,
							 NULL );

							return( -1 );
						}
						page_value_data += 8;
						page_value_size -= 8;
						definition_size -= 8;

						if( libfdatetime_filetime_copy_to_string(
						     filetime,
						     date_time_string,
						     22,
						     LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME,
						     LIBFDATETIME_DATE_TIME_FORMAT_CTIME,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_CONVERSION,
							 LIBERROR_CONVERSION_ERROR_GENERIC,
							 "%s: unable to create filetime string.",
							 function );

							libfdatetime_filetime_free(
							 &filetime,
							 NULL );

							return( -1 );
						}
						libnotify_verbose_printf(
						 "%s: value: %03d file access time\t\t\t: %s\n",
						 function,
						 page_value_iterator,
						 (char *) date_time_string );

						libnotify_verbose_printf(
						 "%s: value: %03d unknown5\t\t\t\t: 0x%02" PRIx8 "\n",
						 function,
						 page_value_iterator,
						 *page_value_data );

						page_value_data += 1;
						page_value_size -= 1;
						definition_size -= 1;

						libnotify_verbose_printf(
						 "%s: value: %03d unknown6\t\t\t\t: 0x%02" PRIx8 "\n",
						 function,
						 page_value_iterator,
						 *page_value_data );

						page_value_data += 1;
						page_value_size -= 1;
						definition_size -= 1;
					}
			 		else if( definition_flags == 0x8013 )
					{
						endian_little_convert_32bit(
						 value_32bit,
						 page_value_data );

						page_value_data += 4;
						page_value_size -= 4;
						definition_size -= 4;

						libnotify_verbose_printf(
						 "%s: value: %03d unknown identifier\t\t\t: 0x%08" PRIx32 " (%" PRIu32 ")\n",
						 function,
						 page_value_iterator,
						 value_32bit,
						 value_32bit );

						endian_little_convert_32bit(
						 value_32bit,
						 page_value_data );

						page_value_data += 4;
						page_value_size -= 4;
						definition_size -= 4;

						libnotify_verbose_printf(
						 "%s: value: %03d unknown3\t\t\t\t: 0x%08" PRIx32 "\n",
						 function,
						 page_value_iterator,
						 value_32bit );

						endian_little_convert_32bit(
						 value_32bit,
						 page_value_data );

						page_value_data += 4;
						page_value_size -= 4;
						definition_size -= 4;

						libnotify_verbose_printf(
						 "%s: value: %03d unknown4\t\t\t\t: 0x%08" PRIx32 "\n",
						 function,
						 page_value_iterator,
						 value_32bit );

						if( libfdatetime_filetime_copy_from_byte_stream(
						     filetime,
						     page_value_data,
						     8,
						     LIBFDATETIME_ENDIAN_LITTLE,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_CONVERSION,
							 LIBERROR_CONVERSION_ERROR_GENERIC,
							 "%s: unable to create creation time.",
							 function );

							libfdatetime_filetime_free(
							 &filetime,
							 NULL );

							return( -1 );
						}
						page_value_data += 8;
						page_value_size -= 8;
						definition_size -= 8;

						if( libfdatetime_filetime_copy_to_string(
						     filetime,
						     date_time_string,
						     22,
						     LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME,
						     LIBFDATETIME_DATE_TIME_FORMAT_CTIME,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_CONVERSION,
							 LIBERROR_CONVERSION_ERROR_GENERIC,
							 "%s: unable to create filetime string.",
							 function );

							libfdatetime_filetime_free(
							 &filetime,
							 NULL );

							return( -1 );
						}
						libnotify_verbose_printf(
						 "%s: value: %03d unknown5\t\t\t\t: %s\n",
						 function,
						 page_value_iterator,
						 (char *) date_time_string );

						endian_little_convert_32bit(
						 value_32bit,
						 page_value_data );

						page_value_data += 4;
						page_value_size -= 4;
						definition_size -= 4;

						libnotify_verbose_printf(
						 "%s: value: %03d unknown6\t\t\t\t: 0x%08" PRIx32 "\n",
						 function,
						 page_value_iterator,
						 value_32bit );

						if( libfdatetime_filetime_copy_from_byte_stream(
						     filetime,
						     page_value_data,
						     8,
						     LIBFDATETIME_ENDIAN_LITTLE,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_CONVERSION,
							 LIBERROR_CONVERSION_ERROR_GENERIC,
							 "%s: unable to create creation time.",
							 function );

							libfdatetime_filetime_free(
							 &filetime,
							 NULL );

							return( -1 );
						}
						page_value_data += 8;
						page_value_size -= 8;
						definition_size -= 8;

						if( libfdatetime_filetime_copy_to_string(
						     filetime,
						     date_time_string,
						     22,
						     LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME,
						     LIBFDATETIME_DATE_TIME_FORMAT_CTIME,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_CONVERSION,
							 LIBERROR_CONVERSION_ERROR_GENERIC,
							 "%s: unable to create filetime string.",
							 function );

							libfdatetime_filetime_free(
							 &filetime,
							 NULL );

							return( -1 );
						}
						libnotify_verbose_printf(
						 "%s: value: %03d unknown7\t\t\t\t: %s\n",
						 function,
						 page_value_iterator,
						 (char *) date_time_string );

						endian_little_convert_32bit(
						 value_32bit,
						 page_value_data );

						page_value_data += 4;
						page_value_size -= 4;
						definition_size -= 4;

						libnotify_verbose_printf(
						 "%s: value: %03d unknown9\t\t\t\t: 0x%08" PRIx32 "\n",
						 function,
						 page_value_iterator,
						 value_32bit );

						endian_little_convert_32bit(
						 value_32bit,
						 page_value_data );

						page_value_data += 4;
						page_value_size -= 4;
						definition_size -= 4;

						libnotify_verbose_printf(
						 "%s: value: %03d unknown10\t\t\t\t: 0x%08" PRIx32 "\n",
						 function,
						 page_value_iterator,
						 value_32bit );

						endian_little_convert_32bit(
						 value_32bit,
						 page_value_data );

						page_value_data += 4;
						page_value_size -= 4;
						definition_size -= 4;

						libnotify_verbose_printf(
						 "%s: value: %03d unknown11\t\t\t\t: 0x%08" PRIx32 "\n",
						 function,
						 page_value_iterator,
						 value_32bit );

						endian_little_convert_32bit(
						 value_32bit,
						 page_value_data );

						page_value_data += 4;
						page_value_size -= 4;
						definition_size -= 4;

						libnotify_verbose_printf(
						 "%s: value: %03d unknown12\t\t\t\t: 0x%08" PRIx32 "\n",
						 function,
						 page_value_iterator,
						 value_32bit );

						endian_little_convert_32bit(
						 value_32bit,
						 page_value_data );

						page_value_data += 4;
						page_value_size -= 4;
						definition_size -= 4;

						libnotify_verbose_printf(
						 "%s: value: %03d unknown13\t\t\t\t: 0x%08" PRIx32 "\n",
						 function,
						 page_value_iterator,
						 value_32bit );

						if( libfdatetime_filetime_copy_from_byte_stream(
						     filetime,
						     page_value_data,
						     8,
						     LIBFDATETIME_ENDIAN_LITTLE,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_CONVERSION,
							 LIBERROR_CONVERSION_ERROR_GENERIC,
							 "%s: unable to create creation time.",
							 function );

							libfdatetime_filetime_free(
							 &filetime,
							 NULL );

							return( -1 );
						}
						page_value_data += 8;
						page_value_size -= 8;
						definition_size -= 8;

						if( libfdatetime_filetime_copy_to_string(
						     filetime,
						     date_time_string,
						     22,
						     LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME,
						     LIBFDATETIME_DATE_TIME_FORMAT_CTIME,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_CONVERSION,
							 LIBERROR_CONVERSION_ERROR_GENERIC,
							 "%s: unable to create filetime string.",
							 function );

							libfdatetime_filetime_free(
							 &filetime,
							 NULL );

							return( -1 );
						}
						libnotify_verbose_printf(
						 "%s: value: %03d unknown14\t\t\t\t: %s\n",
						 function,
						 page_value_iterator,
						 (char *) date_time_string );

						if( libfdatetime_filetime_copy_from_byte_stream(
						     filetime,
						     page_value_data,
						     8,
						     LIBFDATETIME_ENDIAN_LITTLE,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_CONVERSION,
							 LIBERROR_CONVERSION_ERROR_GENERIC,
							 "%s: unable to create creation time.",
							 function );

							libfdatetime_filetime_free(
							 &filetime,
							 NULL );

							return( -1 );
						}
						page_value_data += 8;
						page_value_size -= 8;
						definition_size -= 8;

						if( libfdatetime_filetime_copy_to_string(
						     filetime,
						     date_time_string,
						     22,
						     LIBFDATETIME_STRING_FORMAT_FLAG_DATE_TIME,
						     LIBFDATETIME_DATE_TIME_FORMAT_CTIME,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_CONVERSION,
							 LIBERROR_CONVERSION_ERROR_GENERIC,
							 "%s: unable to create filetime string.",
							 function );

							libfdatetime_filetime_free(
							 &filetime,
							 NULL );

							return( -1 );
						}
						libnotify_verbose_printf(
						 "%s: value: %03d unknown15\t\t\t\t: %s\n",
						 function,
						 page_value_iterator,
						 (char *) date_time_string );

						endian_little_convert_16bit(
						 value_16bit,
						 page_value_data );

						page_value_data += 2;
						page_value_size -= 2;
						definition_size -= 2;

						libnotify_verbose_printf(
						 "%s: value: %03d unknown16\t\t\t\t: 0x%04" PRIx16 "\n",
						 function,
						 page_value_iterator,
						 value_16bit );

						endian_little_convert_16bit(
						 value_16bit,
						 page_value_data );

						page_value_data += 2;
						page_value_size -= 2;
						definition_size -= 2;

						libnotify_verbose_printf(
						 "%s: value: %03d unknown17\t\t\t\t: 0x%04" PRIx16 "\n",
						 function,
						 page_value_iterator,
						 value_16bit );
					}
					if( libfdatetime_filetime_free(
					     &filetime,
					     error ) != 1 )
					{
						liberror_error_set(
						 error,
						 LIBERROR_ERROR_DOMAIN_RUNTIME,
						 LIBERROR_RUNTIME_ERROR_FINALIZE_FAILED,
						 "%s: unable to free filetime.",
						 function );

						return( -1 );
					}
				}
				/* definition type: 0x0001
				 */
				else if( definition_type == LIBESEDB_PAGE_VALUE_DATA_DEFINITION_TYPE_TABLE )
				{
					table_definition = NULL;

					if( libesedb_table_definition_initialize(
					     &table_definition,
					     error ) != 1 )
					{
						liberror_error_set(
						 error,
						 LIBERROR_ERROR_DOMAIN_RUNTIME,
						 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
						 "%s: unable to create table definition.",
						 function );

						return( -1 );
					}
					if( libesedb_table_definition_read(
					     table_definition,
					     page_value_data,
					     definition_size,
					     definition_flags,
					     error ) != 1 )
					{
						liberror_error_set(
						 error,
						 LIBERROR_ERROR_DOMAIN_IO,
						 LIBERROR_IO_ERROR_READ_FAILED,
						 "%s: unable to read table definition in page value: %d.",
						 function,
						 page_value_iterator );

						libesedb_table_definition_free(
						 (intptr_t *) table_definition,
						 NULL );

						table_definition = NULL;

						return( -1 );
					}
					if( libesedb_list_append_value(
					     page_tree->table_definition_list,
					     (intptr_t *) table_definition,
					     error ) != 1 )
					{
						liberror_error_set(
						 error,
						 LIBERROR_ERROR_DOMAIN_RUNTIME,
						 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
						 "%s: unable to append table definition to list.",
						 function );

						libesedb_table_definition_free(
						 (intptr_t *) table_definition,
						 NULL );

						table_definition = NULL;

						return( -1 );
					}
					page_value_data += definition_size;
					page_value_size -= definition_size;
					definition_size -= definition_size;
				}
				/* definition type: 0x0002
				 */
				else if( definition_type == LIBESEDB_PAGE_VALUE_DATA_DEFINITION_TYPE_COLUMN )
				{
					column_definition = NULL;

					if( libesedb_column_definition_initialize(
					     &column_definition,
					     error ) != 1 )
					{
						liberror_error_set(
						 error,
						 LIBERROR_ERROR_DOMAIN_RUNTIME,
						 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
						 "%s: unable to create column definition.",
						 function );

						return( -1 );
					}
					if( libesedb_column_definition_read(
					     column_definition,
					     page_value_data,
					     definition_size,
					     definition_flags,
					     error ) != 1 )
					{
						liberror_error_set(
						 error,
						 LIBERROR_ERROR_DOMAIN_IO,
						 LIBERROR_IO_ERROR_READ_FAILED,
						 "%s: unable to read column definition in page value: %d.",
						 function,
						 page_value_iterator );

						libesedb_column_definition_free(
						 (intptr_t *) column_definition,
						 NULL );

						column_definition = NULL;

						return( -1 );
					}
					if( libesedb_list_append_value(
					     page_tree->column_definition_list,
					     (intptr_t *) column_definition,
					     error ) != 1 )
					{
						liberror_error_set(
						 error,
						 LIBERROR_ERROR_DOMAIN_RUNTIME,
						 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
						 "%s: unable to append column definition to list.",
						 function );

						libesedb_column_definition_free(
						 (intptr_t *) column_definition,
						 NULL );

						column_definition = NULL;

						return( -1 );
					}
					page_value_data += definition_size;
					page_value_size -= definition_size;
					definition_size -= definition_size;
				}
				/* definition type: 0x0003
				 */
				else if( definition_type == LIBESEDB_PAGE_VALUE_DATA_DEFINITION_TYPE_INDEX )
				{
					index_definition = NULL;

					if( libesedb_index_definition_initialize(
					     &index_definition,
					     error ) != 1 )
					{
						liberror_error_set(
						 error,
						 LIBERROR_ERROR_DOMAIN_RUNTIME,
						 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
						 "%s: unable to create index definition.",
						 function );

						return( -1 );
					}
					if( libesedb_index_definition_read(
					     index_definition,
					     page_value_data,
					     definition_size,
					     definition_flags,
					     error ) != 1 )
					{
						liberror_error_set(
						 error,
						 LIBERROR_ERROR_DOMAIN_IO,
						 LIBERROR_IO_ERROR_READ_FAILED,
						 "%s: unable to read index definition in page value: %d.",
						 function,
						 page_value_iterator );

						libesedb_index_definition_free(
						 (intptr_t *) index_definition,
						 NULL );

						index_definition = NULL;

						return( -1 );
					}
					if( libesedb_list_append_value(
					     page_tree->index_definition_list,
					     (intptr_t *) index_definition,
					     error ) != 1 )
					{
						liberror_error_set(
						 error,
						 LIBERROR_ERROR_DOMAIN_RUNTIME,
						 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
						 "%s: unable to append index definition to list.",
						 function );

						libesedb_index_definition_free(
						 (intptr_t *) index_definition,
						 NULL );

						index_definition = NULL;

						return( -1 );
					}
					page_value_data += definition_size;
					page_value_size -= definition_size;
					definition_size -= definition_size;
				}
				/* definition type: 0x0004
				 */
				else if( definition_type == LIBESEDB_PAGE_VALUE_DATA_DEFINITION_TYPE_LONG_VALUE )
				{
					long_value_definition = NULL;

					if( libesedb_long_value_definition_initialize(
					     &long_value_definition,
					     error ) != 1 )
					{
						liberror_error_set(
						 error,
						 LIBERROR_ERROR_DOMAIN_RUNTIME,
						 LIBERROR_RUNTIME_ERROR_INITIALIZE_FAILED,
						 "%s: unable to create long value definition.",
						 function );

						return( -1 );
					}
					if( libesedb_long_value_definition_read(
					     long_value_definition,
					     page_value_data,
					     definition_size,
					     definition_flags,
					     error ) != 1 )
					{
						liberror_error_set(
						 error,
						 LIBERROR_ERROR_DOMAIN_IO,
						 LIBERROR_IO_ERROR_READ_FAILED,
						 "%s: unable to read long value definition in page value: %d.",
						 function,
						 page_value_iterator );

						libesedb_long_value_definition_free(
						 (intptr_t *) long_value_definition,
						 NULL );

						long_value_definition = NULL;

						return( -1 );
					}
					if( libesedb_list_append_value(
					     page_tree->long_value_definition_list,
					     (intptr_t *) long_value_definition,
					     error ) != 1 )
					{
						liberror_error_set(
						 error,
						 LIBERROR_ERROR_DOMAIN_RUNTIME,
						 LIBERROR_RUNTIME_ERROR_APPEND_FAILED,
						 "%s: unable to append long value definition to list.",
						 function );

						libesedb_long_value_definition_free(
						 (intptr_t *) long_value_definition,
						 NULL );

						long_value_definition = NULL;

						return( -1 );
					}
					page_value_data += definition_size;
					page_value_size -= definition_size;
					definition_size -= definition_size;
				}
				else
				{
					libnotify_verbose_printf(
					 "MARKER: unsupported definition type: 0x%04" PRIx16 "\n",
					 definition_type );

					libnotify_verbose_printf(
					 "%s: value: %03d definition data:\n",
					 function,
					 page_value_iterator );
					libnotify_verbose_print_data(
					 page_value_data,
					 definition_size );

					page_value_data += definition_size;
					page_value_size -= definition_size;
					definition_size -= definition_size;
				}
			}
			if( definition_size > 0 )
			{
				libnotify_verbose_printf(
				 "%s: value: %03d definition data:\n",
				 function,
				 page_value_iterator );
				libnotify_verbose_print_data(
				 page_value_data,
				 definition_size );

				page_value_data += definition_size;
				page_value_size -= definition_size;
			}
			if( ( definition_flags == 0x8007 )
			 || ( definition_flags == 0x8008 )
			 || ( definition_flags == 0x8009 )
			 || ( definition_flags == 0x8013 )
			 || ( definition_flags == 0x8309 )
			 || ( definition_flags == 0x840a )
			 || ( definition_flags == 0x880a ) )
			{
				if( page_value_size > 0 )
				{
					endian_little_convert_16bit(
					 name_size,
					 page_value_data );

					page_value_data += 2;
					page_value_size -= 2;

					libnotify_verbose_printf(
					 "%s: value: %03d name size\t\t\t\t: %" PRIu32 "\n",
					 function,
					 page_value_iterator,
					 name_size );

					if( ( definition_flags == 0x8309 )
					 || ( definition_flags == 0x840a )
					 || ( definition_flags == 0x880a ) )
					{
						libnotify_verbose_printf(
						 "%s: value: %03d multi value amount\t\t\t: %" PRIu32 "\n",
						 function,
						 page_value_iterator,
						 ( ( definition_flags & 0x7f00 ) >> 8 ) - 1 );

						for( multi_value_iterator = 0;
						     multi_value_iterator < ( ( ( definition_flags & 0x7f00 ) >> 8 ) - 1 );
						     multi_value_iterator++ )
						{
							endian_little_convert_16bit(
							 value_16bit,
							 page_value_data );

							page_value_data += 2;
							page_value_size -= 2;
							definition_size -= 2;

							libnotify_verbose_printf(
							 "%s: value: %03d unknownA_%02" PRIu8 "\t\t\t: 0x%04" PRIx16 "\n",
							 function,
							 page_value_iterator,
							 multi_value_iterator,
							 value_16bit );
						}
						endian_little_convert_16bit(
						 value_16bit,
						 page_value_data );

						page_value_data += 2;
						page_value_size -= 2;
						definition_size -= 2;

						libnotify_verbose_printf(
						 "%s: value: %03d unknown size\t\t\t: %" PRIu16 "\n",
						 function,
						 page_value_iterator,
						 value_16bit );
					}
					result = libesedb_page_tree_buffer_contains_zero_bytes(
					          page_value_data,
					          (size_t) name_size,
					          error );

					if( result == -1 )
					{
						liberror_error_set(
						 error,
						 LIBERROR_ERROR_DOMAIN_RUNTIME,
						 LIBERROR_RUNTIME_ERROR_GET_FAILED,
						 "%s: unable to determine name string contains zero bytes.",
						 function );

						return( -1 );
					}
					else if( result == 0 )
					{
						if( libuna_utf8_string_size_from_byte_stream(
						     page_value_data,
						     (size_t) name_size,
						     LIBUNA_CODEPAGE_ASCII,
						     &string_size,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_RUNTIME,
							 LIBERROR_RUNTIME_ERROR_GET_FAILED,
							 "%s: unable to determine string size.",
							 function );

							return( -1 );
						}
					}
					else
					{
						if( libuna_utf8_string_size_from_utf16_stream(
						     page_value_data,
						     (size_t) name_size,
						     LIBUNA_ENDIAN_LITTLE,
						     &string_size,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_RUNTIME,
							 LIBERROR_RUNTIME_ERROR_GET_FAILED,
							 "%s: unable to determine string size.",
							 function );

							return( -1 );
						}
					}
					string = (uint8_t *) memory_allocate(
					                      sizeof( uint8_t ) * string_size );

					if( string == NULL )
					{
						liberror_error_set(
						 error,
						 LIBERROR_ERROR_DOMAIN_MEMORY,
						 LIBERROR_MEMORY_ERROR_INSUFFICIENT,
						 "%s: unable to create string.",
						 function );

						return( -1 );
					}
					if( result == 0 )
					{
						if( libuna_utf8_string_copy_from_byte_stream(
						     string,
						     string_size,
						     page_value_data,
						     (size_t) name_size,
						     LIBUNA_CODEPAGE_ASCII,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_CONVERSION,
							 LIBERROR_CONVERSION_ERROR_GENERIC,
							 "%s: unable to set string.",
							 function );

							memory_free(
							 string );

							return( -1 );
						}
					}
					else
					{
						if( libuna_utf8_string_copy_from_utf16_stream(
						     string,
						     string_size,
						     page_value_data,
						     (size_t) name_size,
						     LIBUNA_ENDIAN_LITTLE,
						     error ) != 1 )
						{
							liberror_error_set(
							 error,
							 LIBERROR_ERROR_DOMAIN_CONVERSION,
							 LIBERROR_CONVERSION_ERROR_GENERIC,
							 "%s: unable to set string.",
							 function );

							memory_free(
							 string );

							return( -1 );
						}
					}
					page_value_data += name_size;
					page_value_size -= name_size;

					libnotify_verbose_printf(
					 "%s: value: %03d name string\t\t\t: %s\n",
					 function,
					 (char) page_value_iterator,
					 string );

					if( definition_type == LIBESEDB_PAGE_VALUE_DATA_DEFINITION_TYPE_TABLE )
					{
						table_definition->name      = string;
						table_definition->name_size = string_size;
					}
					else if( definition_type == LIBESEDB_PAGE_VALUE_DATA_DEFINITION_TYPE_COLUMN )
					{
						column_definition->name      = string;
						column_definition->name_size = string_size;
					}
					else if( definition_type == LIBESEDB_PAGE_VALUE_DATA_DEFINITION_TYPE_INDEX )
					{
						index_definition->name      = string;
						index_definition->name_size = string_size;
					}
					else if( definition_type == LIBESEDB_PAGE_VALUE_DATA_DEFINITION_TYPE_LONG_VALUE )
					{
						long_value_definition->name      = string;
						long_value_definition->name_size = string_size;
					}
					else
					{
						memory_free(
						 string );
					}
					if( ( definition_flags == 0x8309 )
					 || ( definition_flags == 0x840a )
					 || ( definition_flags == 0x880a ) )
					{
						libnotify_verbose_printf(
						 "%s: value: %03d column key size\t\t\t: %" PRIu16 "\n",
						 function,
						 page_value_iterator,
						 page_value_size );

						libnotify_verbose_printf(
						 "%s: value: %03d column key value\t\t\t: ",
						 function,
						 page_value_iterator );

						while( page_value_size > 0 )
						{
							libnotify_verbose_printf(
							 "%02" PRIx8 " ",
							 *page_value_data );

							page_value_data++;
							page_value_size--;
						}
						libnotify_verbose_printf(
						 "\n" );
					}
					libnotify_verbose_printf(
					 "\n" );
				}
			}
#endif
			if( page_value_size > 0 )
			{
				libnotify_verbose_printf(
				 "%s: value: %03d value data:\n",
				 function,
				 page_value_iterator );
				libnotify_verbose_print_data(
				 page_value_data,
				 page_value_size );
			}
		}
#endif
	}
#if defined( HAVE_DEBUG_OUTPUT )
	libnotify_verbose_printf(
	 "\n" );
#endif

	return( 1 );
}

