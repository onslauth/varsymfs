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

/*
 * /opt/pq/0/resolve:
 */

static int varsymfs_resolve_readlink( struct dentry *dentry, char __user *buffer, int buflen )
{
	char *link;
	int err;

	link = varsymfs_environment_get_value( );
	if( !link ) {
		err = PTR_ERR( link );
		goto out;
	}

	if( dentry && dentry->d_inode )
		dentry->d_inode->i_size = strlen( link );

	err = readlink_copy( buffer, buflen, link );
	if( err )
		goto out;

out:
	return err;
}

static void *varsymfs_resolve_follow_link( struct dentry *dentry, struct nameidata *nd )
{
	char *link = ERR_PTR( -ENOENT );

	link = varsymfs_environment_get_value( );
	if( !link ) {
		link = ERR_PTR( -ENOMEM );
		goto out;
	}

	if( dentry && dentry->d_inode )
		dentry->d_inode->i_size = strlen( link );

out:
	nd_set_link( nd, link );
	return NULL;
}

static const struct inode_operations varsymfs_resolve_inode_operations = {
	.readlink	= varsymfs_resolve_readlink,
	.follow_link	= varsymfs_resolve_follow_link,
};

static unsigned resolve_inum = 1;

int varsymfs_setup_resolve( struct super_block *s )
{
	struct inode *root_inode = s->s_root->d_inode;
	struct dentry *resolve;

	mutex_lock( &root_inode->i_mutex );
	resolve = d_alloc_name( s->s_root, "resolve" );

	if( resolve ) {
		struct inode *inode = new_inode( s );
		if( inode ) {
			inode->i_ino = resolve_inum;
			inode->i_ctime = inode->i_mtime = inode->i_atime = CURRENT_TIME;
			inode->i_mode = S_IFLNK | S_IRWXUGO;
			inode->i_uid = GLOBAL_ROOT_UID;
			inode->i_gid = GLOBAL_ROOT_GID;
			inode->i_op = &varsymfs_resolve_inode_operations;
			d_add( resolve, inode );
		}
		else {
			dput( resolve );
			resolve = ERR_PTR( -ENOMEM );
		}
	}
	else {
		resolve = ERR_PTR( -ENOMEM );
	}

	dput( resolve );
	mutex_unlock( &root_inode->i_mutex );

	if( IS_ERR( resolve ) ) {
		pr_err( "varsymfs: can't allocate /resolve\n" );
		return PTR_ERR( resolve );
	}

	return 0;
}
