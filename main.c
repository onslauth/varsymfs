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
#include <linux/module.h>

static int varsymfs_fill_super( struct super_block *s, void *raw_data, int silent)
{
	struct inode *root_inode;
	int err = 0;
	unsigned int buflen = 0;
	char * env_var = (char *)raw_data;

	/* Set the super block details */
	s->s_flags = MS_NODIRATIME | MS_NOSUID | MS_NOEXEC;
	s->s_blocksize = 1024;
	s->s_blocksize_bits = 10;
	s->s_magic = VARSYMFS_MAGIC_NUMBER;
	s->s_op = &varsymfs_simple_super_operations;
	s->s_time_gran = 1;

	/* Copy in the environment variable passed in as an option */
	buflen = strlen( env_var ) + 1;
	s->s_fs_info = kmalloc( buflen, GFP_KERNEL );
	if( IS_ERR( s->s_fs_info ) ) {
		pr_err( "varsymfs_fill_super: failed to allocate memory for private filesystem data\n" );
		return -ENOMEM;
	}
	((char *)s->s_fs_info)[buflen] = '\0';
	strncpy( s->s_fs_info, env_var, buflen - 1 );

	/* Create a root inode with ino of 0 */
	root_inode = varsymfs_iget( s, 0 );
	if( IS_ERR( root_inode ) ) {
		pr_err( "varsymfs_fill_super: get root inode failed\n" );
		return -ENOMEM;
	}

	/* Setup inode */
	/* Set inode as a directory, with read and execute permissions, no write */
	root_inode->i_mode = S_IFDIR | S_IRUGO | S_IXUGO;

	/* Set the uid */
	root_inode->i_uid = GLOBAL_ROOT_UID;

	/* Set the gid */
	root_inode->i_gid = GLOBAL_ROOT_GID;

	/* Set creation time, access time and modified time to current time */
	root_inode->i_atime = root_inode->i_ctime = root_inode->i_mtime = CURRENT_TIME;

	/* Set the inode operations for the inode */
	root_inode->i_op = &simple_dir_inode_operations;

	/* Set the file operations for the inode */
	root_inode->i_fop = &simple_dir_operations;


	/* Create root dentry */
	s->s_root = d_make_root( root_inode );
	if( !s->s_root ) {
		pr_err( "varsymfs_fill_super: allocate dentry failed\n" );
		return -ENOMEM;
	}

	d_set_d_op( s->s_root, &simple_dentry_operations );

	err = varsymfs_setup_resolve( s );
	if( err ) {
		return err;
	}

	return err;
}

struct dentry *varsymfs_mount( struct file_system_type *fs_type, int flags,
			       const char *dev_name, void *raw_data )
{
	void *data = (void *)raw_data;

	if( !raw_data ) {
		printk( KERN_ERR "Missing mount point options\n" );
		return ERR_PTR( -EINVAL );
	}

	return mount_nodev( fs_type, flags, data, varsymfs_fill_super );
}

static struct file_system_type varsymfs_fs_type = {
	.owner		= THIS_MODULE,
	.name		= VARSYMFS_NAME,
	.mount		= varsymfs_mount,
	.kill_sb	= generic_shutdown_super,
	.fs_flags	= 0,
};
MODULE_ALIAS_FS(VARSYMFS_NAME);

static int __init init_varsymfs( void )
{
	int err;

	pr_info( "Registering varsymfs " VARSYMFS_VERSION "\n" );

	err = varsymfs_init_inode_cache( );
	if( err )
		goto out;

	err = register_filesystem( &varsymfs_fs_type );
	if( err )
		goto out;

out:
	if( err ) {
		varsymfs_destroy_inode_cache( );
	}
	return err;
}

static void __exit exit_varsymfs( void )
{
	/* Clean up the cache */
	varsymfs_destroy_inode_cache( );

	/* unregister the file system type */
	unregister_filesystem( &varsymfs_fs_type );

	pr_info( "Completed varsymfs module unload\n" );
}

MODULE_AUTHOR( "Cole Minnaar, Paroq Systems (http://www.paroq.com)" );
MODULE_DESCRIPTION( "Variant Symlink Filesystem " VARSYMFS_VERSION "(http://www.paroq.com)" );
MODULE_LICENSE( "BSD" );

module_init( init_varsymfs );
module_exit( exit_varsymfs );
