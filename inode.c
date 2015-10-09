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

static struct kmem_cache *varsymfs_inode_cachep;

static void init_once( void *foo )
{
	struct varsymfs_inode *i = (struct varsymfs_inode *) foo;

	inode_init_once( &i->vfs_inode );
}

int varsymfs_init_inode_cache( void )
{
	int err = 0;

	varsymfs_inode_cachep = kmem_cache_create( "varsymfs_inode_cache",
						   sizeof( struct varsymfs_inode ),
						   0,
						   SLAB_RECLAIM_ACCOUNT,
						   init_once );

	if( !varsymfs_inode_cachep )
		err = -ENOMEM;

	return err;
}

void varsymfs_destroy_inode_cache( void )
{
	if( varsymfs_inode_cachep )
		kmem_cache_destroy( varsymfs_inode_cachep );
}

struct inode *varsymfs_iget( struct super_block *sb, unsigned long ino )
{
	struct inode *inode;

	inode = iget_locked( sb, ino );
	if( !inode )
		return ERR_PTR( -ENOMEM );

	if( !(inode->i_state & I_NEW) )
		return inode;

	unlock_new_inode( inode );
	return inode;
}
