/*
 * Copyright (c) 2015, Cole Minnaar
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may
 * be used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "varsymfs.h"

static int varsymfs_valid_environment( struct mm_struct *mm )
{
	return mm && mm->env_start && mm->env_start < mm->env_end;
}

static int varsymfs_is_variable_delimiter( char character )
{
	return character == '=';
}

char *varsymfs_environment_get_value( void )
{
	char *env = NULL;
	char *value = NULL;
	char env_var[] = "PQ_CHASSIS_OPT_DIR";
	size_t env_length;

	struct mm_struct *mm = current->mm;

	if( !varsymfs_valid_environment( mm ) ) {
		return NULL;
	}

	env_length = strlen( env_var );

	env = (char *)mm->env_start;

	while( env < (char *)mm->env_end )
	{
		if( !strncmp( env, env_var, env_length ) )
		{
			if( varsymfs_is_variable_delimiter( env[env_length] ) )
			{
				value = (char *)&env[ env_length + 1 ];
				break;
			}
		}

		env += strlen( env ) + 1;
	}


	return value;
}
