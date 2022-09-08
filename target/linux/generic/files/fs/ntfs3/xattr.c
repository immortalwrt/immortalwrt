// SPDX-License-Identifier: GPL-2.0
/*
 *
 * Copyright (C) 2019-2021 Paragon Software GmbH, All rights reserved.
 *
 */

#include <linux/blkdev.h>
#include <linux/buffer_head.h>
#include <linux/fs.h>
#include <linux/nls.h>
#include <linux/posix_acl.h>
#include <linux/posix_acl_xattr.h>
#include <linux/xattr.h>

#include "debug.h"
#include "ntfs.h"
#include "ntfs_fs.h"

// clang-format off
#define SYSTEM_DOS_ATTRIB    "system.dos_attrib"
#define SYSTEM_NTFS_ATTRIB   "system.ntfs_attrib"
#define SYSTEM_NTFS_SECURITY "system.ntfs_security"
// clang-format on

static inline size_t unpacked_ea_size(const struct EA_FULL *ea)
{
	return ea->size ? le32_to_cpu(ea->size)
			: DwordAlign(struct_size(
				  ea, name,
				  1 + ea->name_len + le16_to_cpu(ea->elength)));
}

static inline size_t packed_ea_size(const struct EA_FULL *ea)
{
	return struct_size(ea, name,
			   1 + ea->name_len + le16_to_cpu(ea->elength)) -
	       offsetof(struct EA_FULL, flags);
}

/*
 * find_ea
 *
 * assume there is at least one xattr in the list
 */
static inline bool find_ea(const struct EA_FULL *ea_all, u32 bytes,
			   const char *name, u8 name_len, u32 *off)
{
	*off = 0;

	if (!ea_all || !bytes)
		return false;

	for (;;) {
		const struct EA_FULL *ea = Add2Ptr(ea_all, *off);
		u32 next_off = *off + unpacked_ea_size(ea);

		if (next_off > bytes)
			return false;

		if (ea->name_len == name_len &&
		    !memcmp(ea->name, name, name_len))
			return true;

		*off = next_off;
		if (next_off >= bytes)
			return false;
	}
}

/*
 * ntfs_read_ea
 *
 * reads all extended attributes
 * ea - new allocated memory
 * info - pointer into resident data
 */
static int ntfs_read_ea(struct ntfs_inode *ni, struct EA_FULL **ea,
			size_t add_bytes, const struct EA_INFO **info)
{
	int err;
	struct ATTR_LIST_ENTRY *le = NULL;
	struct ATTRIB *attr_info, *attr_ea;
	void *ea_p;
	u32 size;

	static_assert(le32_to_cpu(ATTR_EA_INFO) < le32_to_cpu(ATTR_EA));

	*ea = NULL;
	*info = NULL;

	attr_info =
		ni_find_attr(ni, NULL, &le, ATTR_EA_INFO, NULL, 0, NULL, NULL);
	attr_ea =
		ni_find_attr(ni, attr_info, &le, ATTR_EA, NULL, 0, NULL, NULL);

	if (!attr_ea || !attr_info)
		return 0;

	*info = resident_data_ex(attr_info, sizeof(struct EA_INFO));
	if (!*info)
		return -EINVAL;

	/* Check Ea limit */
	size = le32_to_cpu((*info)->size);
	if (size > ni->mi.sbi->ea_max_size)
		return -EFBIG;

	if (attr_size(attr_ea) > ni->mi.sbi->ea_max_size)
		return -EFBIG;

	/* Allocate memory for packed Ea */
	ea_p = ntfs_malloc(size + add_bytes);
	if (!ea_p)
		return -ENOMEM;

	if (attr_ea->non_res) {
		struct runs_tree run;

		run_init(&run);

		err = attr_load_runs(attr_ea, ni, &run, NULL);
		if (!err)
			err = ntfs_read_run_nb(ni->mi.sbi, &run, 0, ea_p, size,
					       NULL);
		run_close(&run);

		if (err)
			goto out;
	} else {
		void *p = resident_data_ex(attr_ea, size);

		if (!p) {
			err = -EINVAL;
			goto out;
		}
		memcpy(ea_p, p, size);
	}

	memset(Add2Ptr(ea_p, size), 0, add_bytes);
	*ea = ea_p;
	return 0;

out:
	ntfs_free(ea_p);
	*ea = NULL;
	return err;
}

/*
 * ntfs_list_ea
 *
 * copy a list of xattrs names into the buffer
 * provided, or compute the buffer size required
 *
 * Returns a negative error number on failure, or the number of bytes
 * used / required on success.
 */
static ssize_t ntfs_list_ea(struct ntfs_inode *ni, char *buffer,
			    size_t bytes_per_buffer)
{
	const struct EA_INFO *info;
	struct EA_FULL *ea_all = NULL;
	const struct EA_FULL *ea;
	u32 off, size;
	int err;
	size_t ret;

	err = ntfs_read_ea(ni, &ea_all, 0, &info);
	if (err)
		return err;

	if (!info || !ea_all)
		return 0;

	size = le32_to_cpu(info->size);

	/* Enumerate all xattrs */
	for (ret = 0, off = 0; off < size; off += unpacked_ea_size(ea)) {
		ea = Add2Ptr(ea_all, off);

		if (buffer) {
			if (ret + ea->name_len + 1 > bytes_per_buffer) {
				err = -ERANGE;
				goto out;
			}

			memcpy(buffer + ret, ea->name, ea->name_len);
			buffer[ret + ea->name_len] = 0;
		}

		ret += ea->name_len + 1;
	}

out:
	ntfs_free(ea_all);
	return err ? err : ret;
}

static int ntfs_get_ea(struct inode *inode, const char *name, size_t name_len,
		       void *buffer, size_t size, size_t *required)
{
	struct ntfs_inode *ni = ntfs_i(inode);
	const struct EA_INFO *info;
	struct EA_FULL *ea_all = NULL;
	const struct EA_FULL *ea;
	u32 off, len;
	int err;

	if (!(ni->ni_flags & NI_FLAG_EA))
		return -ENODATA;

	if (!required)
		ni_lock(ni);

	len = 0;

	if (name_len > 255) {
		err = -ENAMETOOLONG;
		goto out;
	}

	err = ntfs_read_ea(ni, &ea_all, 0, &info);
	if (err)
		goto out;

	if (!info)
		goto out;

	/* Enumerate all xattrs */
	if (!find_ea(ea_all, le32_to_cpu(info->size), name, name_len, &off)) {
		err = -ENODATA;
		goto out;
	}
	ea = Add2Ptr(ea_all, off);

	len = le16_to_cpu(ea->elength);
	if (!buffer) {
		err = 0;
		goto out;
	}

	if (len > size) {
		err = -ERANGE;
		if (required)
			*required = len;
		goto out;
	}

	memcpy(buffer, ea->name + ea->name_len + 1, len);
	err = 0;

out:
	ntfs_free(ea_all);
	if (!required)
		ni_unlock(ni);

	return err ? err : len;
}

static noinline int ntfs_set_ea(struct inode *inode, const char *name,
				size_t name_len, const void *value,
				size_t val_size, int flags, int locked)
{
	struct ntfs_inode *ni = ntfs_i(inode);
	struct ntfs_sb_info *sbi = ni->mi.sbi;
	int err;
	struct EA_INFO ea_info;
	const struct EA_INFO *info;
	struct EA_FULL *new_ea;
	struct EA_FULL *ea_all = NULL;
	size_t add, new_pack;
	u32 off, size;
	__le16 size_pack;
	struct ATTRIB *attr;
	struct ATTR_LIST_ENTRY *le;
	struct mft_inode *mi;
	struct runs_tree ea_run;
	u64 new_sz;
	void *p;

	if (!locked)
		ni_lock(ni);

	run_init(&ea_run);

	if (name_len > 255) {
		err = -ENAMETOOLONG;
		goto out;
	}

	add = DwordAlign(struct_size(ea_all, name, 1 + name_len + val_size));

	err = ntfs_read_ea(ni, &ea_all, add, &info);
	if (err)
		goto out;

	if (!info) {
		memset(&ea_info, 0, sizeof(ea_info));
		size = 0;
		size_pack = 0;
	} else {
		memcpy(&ea_info, info, sizeof(ea_info));
		size = le32_to_cpu(ea_info.size);
		size_pack = ea_info.size_pack;
	}

	if (info && find_ea(ea_all, size, name, name_len, &off)) {
		struct EA_FULL *ea;
		size_t ea_sz;

		if (flags & XATTR_CREATE) {
			err = -EEXIST;
			goto out;
		}

		/* Remove current xattr */
		ea = Add2Ptr(ea_all, off);
		if (ea->flags & FILE_NEED_EA)
			le16_add_cpu(&ea_info.count, -1);

		ea_sz = unpacked_ea_size(ea);

		le16_add_cpu(&ea_info.size_pack, 0 - packed_ea_size(ea));

		memmove(ea, Add2Ptr(ea, ea_sz), size - off - ea_sz);

		size -= ea_sz;
		memset(Add2Ptr(ea_all, size), 0, ea_sz);

		ea_info.size = cpu_to_le32(size);

		if ((flags & XATTR_REPLACE) && !val_size)
			goto update_ea;
	} else {
		if (flags & XATTR_REPLACE) {
			err = -ENODATA;
			goto out;
		}

		if (!ea_all) {
			ea_all = ntfs_zalloc(add);
			if (!ea_all) {
				err = -ENOMEM;
				goto out;
			}
		}
	}

	/* append new xattr */
	new_ea = Add2Ptr(ea_all, size);
	new_ea->size = cpu_to_le32(add);
	new_ea->flags = 0;
	new_ea->name_len = name_len;
	new_ea->elength = cpu_to_le16(val_size);
	memcpy(new_ea->name, name, name_len);
	new_ea->name[name_len] = 0;
	memcpy(new_ea->name + name_len + 1, value, val_size);
	new_pack = le16_to_cpu(ea_info.size_pack) + packed_ea_size(new_ea);

	/* should fit into 16 bits */
	if (new_pack > 0xffff) {
		err = -EFBIG; // -EINVAL?
		goto out;
	}
	ea_info.size_pack = cpu_to_le16(new_pack);

	/* new size of ATTR_EA */
	size += add;
	if (size > sbi->ea_max_size) {
		err = -EFBIG; // -EINVAL?
		goto out;
	}
	ea_info.size = cpu_to_le32(size);

update_ea:

	if (!info) {
		/* Create xattr */
		if (!size) {
			err = 0;
			goto out;
		}

		err = ni_insert_resident(ni, sizeof(struct EA_INFO),
					 ATTR_EA_INFO, NULL, 0, NULL, NULL);
		if (err)
			goto out;

		err = ni_insert_resident(ni, 0, ATTR_EA, NULL, 0, NULL, NULL);
		if (err)
			goto out;
	}

	new_sz = size;
	err = attr_set_size(ni, ATTR_EA, NULL, 0, &ea_run, new_sz, &new_sz,
			    false, NULL);
	if (err)
		goto out;

	le = NULL;
	attr = ni_find_attr(ni, NULL, &le, ATTR_EA_INFO, NULL, 0, NULL, &mi);
	if (!attr) {
		err = -EINVAL;
		goto out;
	}

	if (!size) {
		/* delete xattr, ATTR_EA_INFO */
		err = ni_remove_attr_le(ni, attr, le);
		if (err)
			goto out;
	} else {
		p = resident_data_ex(attr, sizeof(struct EA_INFO));
		if (!p) {
			err = -EINVAL;
			goto out;
		}
		memcpy(p, &ea_info, sizeof(struct EA_INFO));
		mi->dirty = true;
	}

	le = NULL;
	attr = ni_find_attr(ni, NULL, &le, ATTR_EA, NULL, 0, NULL, &mi);
	if (!attr) {
		err = -EINVAL;
		goto out;
	}

	if (!size) {
		/* delete xattr, ATTR_EA */
		err = ni_remove_attr_le(ni, attr, le);
		if (err)
			goto out;
	} else if (attr->non_res) {
		err = ntfs_sb_write_run(sbi, &ea_run, 0, ea_all, size);
		if (err)
			goto out;
	} else {
		p = resident_data_ex(attr, size);
		if (!p) {
			err = -EINVAL;
			goto out;
		}
		memcpy(p, ea_all, size);
		mi->dirty = true;
	}

	/* Check if we delete the last xattr */
	if (size)
		ni->ni_flags |= NI_FLAG_EA;
	else
		ni->ni_flags &= ~NI_FLAG_EA;

	if (ea_info.size_pack != size_pack)
		ni->ni_flags |= NI_FLAG_UPDATE_PARENT;
	mark_inode_dirty(&ni->vfs_inode);

out:
	if (!locked)
		ni_unlock(ni);

	run_close(&ea_run);
	ntfs_free(ea_all);

	return err;
}

#ifdef CONFIG_NTFS3_FS_POSIX_ACL
static inline void ntfs_posix_acl_release(struct posix_acl *acl)
{
	if (acl && refcount_dec_and_test(&acl->a_refcount))
		kfree(acl);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
static struct posix_acl *ntfs_get_acl_ex(struct user_namespace *mnt_userns,
#else
static struct posix_acl *ntfs_get_acl_ex(
#endif
					 struct inode *inode, int type,
					 int locked)
{
	struct ntfs_inode *ni = ntfs_i(inode);
	const char *name;
	size_t name_len;
	struct posix_acl *acl;
	size_t req;
	int err;
	void *buf;

	/* allocate PATH_MAX bytes */
	buf = __getname();
	if (!buf)
		return ERR_PTR(-ENOMEM);

	/* Possible values of 'type' was already checked above */
	if (type == ACL_TYPE_ACCESS) {
		name = XATTR_NAME_POSIX_ACL_ACCESS;
		name_len = sizeof(XATTR_NAME_POSIX_ACL_ACCESS) - 1;
	} else {
		name = XATTR_NAME_POSIX_ACL_DEFAULT;
		name_len = sizeof(XATTR_NAME_POSIX_ACL_DEFAULT) - 1;
	}

	if (!locked)
		ni_lock(ni);

	err = ntfs_get_ea(inode, name, name_len, buf, PATH_MAX, &req);

	if (!locked)
		ni_unlock(ni);

	/* Translate extended attribute to acl */
	if (err > 0) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
		acl = posix_acl_from_xattr(mnt_userns, buf, err);
#else
		acl = posix_acl_from_xattr(&init_user_ns, buf, err);
#endif
		if (!IS_ERR(acl))
			set_cached_acl(inode, type, acl);
	} else {
		acl = err == -ENODATA ? NULL : ERR_PTR(err);
	}

	__putname(buf);

	return acl;
}

/*
 * ntfs_get_acl
 *
 * inode_operations::get_acl
 */
struct posix_acl *ntfs_get_acl(struct inode *inode, int type)
{
	/* TODO: init_user_ns? */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
	return ntfs_get_acl_ex(&init_user_ns, inode, type, 0);
#else
	return ntfs_get_acl_ex(inode, type, 0);
#endif
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
static noinline int ntfs_set_acl_ex(struct user_namespace *mnt_userns,
#else
static noinline int ntfs_set_acl_ex(
#endif
				    struct inode *inode, struct posix_acl *acl,
				    int type, int locked)
{
	const char *name;
	size_t size, name_len;
	void *value = NULL;
	int err = 0;

	if (S_ISLNK(inode->i_mode))
		return -EOPNOTSUPP;

	switch (type) {
	case ACL_TYPE_ACCESS:
		if (acl) {
			umode_t mode = inode->i_mode;

			err = posix_acl_equiv_mode(acl, &mode);
			if (err < 0)
				return err;

			if (inode->i_mode != mode) {
				inode->i_mode = mode;
				mark_inode_dirty(inode);
			}

			if (!err) {
				/*
				 * acl can be exactly represented in the
				 * traditional file mode permission bits
				 */
				acl = NULL;
				goto out;
			}
		}
		name = XATTR_NAME_POSIX_ACL_ACCESS;
		name_len = sizeof(XATTR_NAME_POSIX_ACL_ACCESS) - 1;
		break;

	case ACL_TYPE_DEFAULT:
		if (!S_ISDIR(inode->i_mode))
			return acl ? -EACCES : 0;
		name = XATTR_NAME_POSIX_ACL_DEFAULT;
		name_len = sizeof(XATTR_NAME_POSIX_ACL_DEFAULT) - 1;
		break;

	default:
		return -EINVAL;
	}

	if (!acl)
		goto out;

	size = posix_acl_xattr_size(acl->a_count);
	value = ntfs_malloc(size);
	if (!value)
		return -ENOMEM;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
	err = posix_acl_to_xattr(mnt_userns, acl, value, size);
#else
	err = posix_acl_to_xattr(&init_user_ns, acl, value, size);
#endif
	if (err)
		goto out;

	err = ntfs_set_ea(inode, name, name_len, value, size, 0, locked);
	if (err)
		goto out;

	inode->i_flags &= ~S_NOSEC;

out:
	if (!err)
		set_cached_acl(inode, type, acl);

	kfree(value);

	return err;
}

/*
 * ntfs_set_acl
 *
 * inode_operations::set_acl
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
int ntfs_set_acl(struct user_namespace *mnt_userns, struct inode *inode,
#else
int ntfs_set_acl(struct inode *inode,
#endif
		 struct posix_acl *acl, int type)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
	return ntfs_set_acl_ex(mnt_userns, inode, acl, type, 0);
#else
	return ntfs_set_acl_ex(inode, acl, type, 0);
#endif
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
static int ntfs_xattr_get_acl(struct user_namespace *mnt_userns,
#else
static int ntfs_xattr_get_acl(
#endif
			      struct inode *inode, int type, void *buffer,
			      size_t size)
{
	struct posix_acl *acl;
	int err;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
	if (!(inode->i_sb->s_flags & SB_POSIXACL))
#else
	if (!(inode->i_sb->s_flags & MS_POSIXACL))
#endif
		return -EOPNOTSUPP;

	acl = ntfs_get_acl(inode, type);
	if (IS_ERR(acl))
		return PTR_ERR(acl);

	if (!acl)
		return -ENODATA;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
	err = posix_acl_to_xattr(mnt_userns, acl, buffer, size);
#else
	err = posix_acl_to_xattr(&init_user_ns, acl, buffer, size);
#endif
	ntfs_posix_acl_release(acl);

	return err;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
static int ntfs_xattr_set_acl(struct user_namespace *mnt_userns,
#else
static int ntfs_xattr_set_acl(
#endif
			      struct inode *inode, int type, const void *value,
			      size_t size)
{
	struct posix_acl *acl;
	int err;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
	if (!(inode->i_sb->s_flags & SB_POSIXACL))
#else
	if (!(inode->i_sb->s_flags & MS_POSIXACL))
#endif
		return -EOPNOTSUPP;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
	if (!inode_owner_or_capable(mnt_userns, inode))
#else
	if (!inode_owner_or_capable(inode))
#endif
		return -EPERM;

	if (!value)
		return 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
	acl = posix_acl_from_xattr(mnt_userns, value, size);
#else
	acl = posix_acl_from_xattr(&init_user_ns, value, size);
#endif
	if (IS_ERR(acl))
		return PTR_ERR(acl);

	if (acl) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
		err = posix_acl_valid(mnt_userns, acl);
#else
		err = posix_acl_valid(&init_user_ns, acl);
#endif
		if (err)
			goto release_and_out;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
	err = ntfs_set_acl(mnt_userns, inode, acl, type);
#else
	err = ntfs_set_acl(inode, acl, type);
#endif

release_and_out:
	ntfs_posix_acl_release(acl);
	return err;
}

/*
 * Initialize the ACLs of a new inode. Called from ntfs_create_inode.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
int ntfs_init_acl(struct user_namespace *mnt_userns, struct inode *inode,
#else
int ntfs_init_acl(struct inode *inode,
#endif
		  struct inode *dir)
{
	struct posix_acl *default_acl, *acl;
	int err;

	/*
	 * TODO refactoring lock
	 * ni_lock(dir) ... -> posix_acl_create(dir,...) -> ntfs_get_acl -> ni_lock(dir)
	 */
	inode->i_default_acl = NULL;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
	default_acl = ntfs_get_acl_ex(mnt_userns, dir, ACL_TYPE_DEFAULT, 1);
#else
	default_acl = ntfs_get_acl_ex(dir, ACL_TYPE_DEFAULT, 1);
#endif

	if (!default_acl || default_acl == ERR_PTR(-EOPNOTSUPP)) {
		inode->i_mode &= ~current_umask();
		err = 0;
		goto out;
	}

	if (IS_ERR(default_acl)) {
		err = PTR_ERR(default_acl);
		goto out;
	}

	acl = default_acl;
	err = __posix_acl_create(&acl, GFP_NOFS, &inode->i_mode);
	if (err < 0)
		goto out1;
	if (!err) {
		posix_acl_release(acl);
		acl = NULL;
	}

	if (!S_ISDIR(inode->i_mode)) {
		posix_acl_release(default_acl);
		default_acl = NULL;
	}

	if (default_acl)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
		err = ntfs_set_acl_ex(mnt_userns, inode, default_acl,
#else
		err = ntfs_set_acl_ex(inode, default_acl,
#endif
				      ACL_TYPE_DEFAULT, 1);

	if (!acl)
		inode->i_acl = NULL;
	else if (!err)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
		err = ntfs_set_acl_ex(mnt_userns, inode, acl, ACL_TYPE_ACCESS,
#else
		err = ntfs_set_acl_ex(inode, acl, ACL_TYPE_ACCESS,
#endif
				      1);

	posix_acl_release(acl);
out1:
	posix_acl_release(default_acl);

out:
	return err;
}
#endif

/*
 * ntfs_acl_chmod
 *
 * helper for 'ntfs3_setattr'
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
int ntfs_acl_chmod(struct user_namespace *mnt_userns, struct inode *inode)
#else
int ntfs_acl_chmod(struct inode *inode)
#endif
{
	struct super_block *sb = inode->i_sb;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
	if (!(sb->s_flags & SB_POSIXACL))
#else
	if (!(sb->s_flags & MS_POSIXACL))
#endif
		return 0;

	if (S_ISLNK(inode->i_mode))
		return -EOPNOTSUPP;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
	return posix_acl_chmod(mnt_userns, inode, inode->i_mode);
#else
	return posix_acl_chmod(inode, inode->i_mode);
#endif
}

/*
 * ntfs_permission
 *
 * inode_operations::permission
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
int ntfs_permission(struct user_namespace *mnt_userns, struct inode *inode,
#else
int ntfs_permission(struct inode *inode,
#endif
		    int mask)
{
	if (ntfs_sb(inode->i_sb)->options.no_acs_rules) {
		/* "no access rules" mode - allow all changes */
		return 0;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
	return generic_permission(mnt_userns, inode, mask);
#else
	return generic_permission(inode, mask);
#endif
}

/*
 * ntfs_listxattr
 *
 * inode_operations::listxattr
 */
ssize_t ntfs_listxattr(struct dentry *dentry, char *buffer, size_t size)
{
	struct inode *inode = d_inode(dentry);
	struct ntfs_inode *ni = ntfs_i(inode);
	ssize_t ret;

	if (!(ni->ni_flags & NI_FLAG_EA)) {
		/* no xattr in file */
		return 0;
	}

	ni_lock(ni);

	ret = ntfs_list_ea(ni, buffer, size);

	ni_unlock(ni);

	return ret;
}

static int ntfs_getxattr(const struct xattr_handler *handler, struct dentry *de,
			 struct inode *inode, const char *name, void *buffer,
			 size_t size)
{
	int err;
	struct ntfs_inode *ni = ntfs_i(inode);
	size_t name_len = strlen(name);

	/* Dispatch request */
	if (name_len == sizeof(SYSTEM_DOS_ATTRIB) - 1 &&
	    !memcmp(name, SYSTEM_DOS_ATTRIB, sizeof(SYSTEM_DOS_ATTRIB))) {
		/* system.dos_attrib */
		if (!buffer) {
			err = sizeof(u8);
		} else if (size < sizeof(u8)) {
			err = -ENODATA;
		} else {
			err = sizeof(u8);
			*(u8 *)buffer = le32_to_cpu(ni->std_fa);
		}
		goto out;
	}

	if (name_len == sizeof(SYSTEM_NTFS_ATTRIB) - 1 &&
	    !memcmp(name, SYSTEM_NTFS_ATTRIB, sizeof(SYSTEM_NTFS_ATTRIB))) {
		/* system.ntfs_attrib */
		if (!buffer) {
			err = sizeof(u32);
		} else if (size < sizeof(u32)) {
			err = -ENODATA;
		} else {
			err = sizeof(u32);
			*(u32 *)buffer = le32_to_cpu(ni->std_fa);
		}
		goto out;
	}

	if (name_len == sizeof(SYSTEM_NTFS_SECURITY) - 1 &&
	    !memcmp(name, SYSTEM_NTFS_SECURITY, sizeof(SYSTEM_NTFS_SECURITY))) {
		/* system.ntfs_security*/
		struct SECURITY_DESCRIPTOR_RELATIVE *sd = NULL;
		size_t sd_size = 0;

		if (!is_ntfs3(ni->mi.sbi)) {
			/* we should get nt4 security */
			err = -EINVAL;
			goto out;
		} else if (le32_to_cpu(ni->std_security_id) <
			   SECURITY_ID_FIRST) {
			err = -ENOENT;
			goto out;
		}

		err = ntfs_get_security_by_id(ni->mi.sbi, ni->std_security_id,
					      &sd, &sd_size);
		if (err)
			goto out;

		if (!is_sd_valid(sd, sd_size)) {
			ntfs_inode_warn(
				inode,
				"looks like you get incorrect security descriptor id=%u",
				ni->std_security_id);
		}

		if (!buffer) {
			err = sd_size;
		} else if (size < sd_size) {
			err = -ENODATA;
		} else {
			err = sd_size;
			memcpy(buffer, sd, sd_size);
		}
		ntfs_free(sd);
		goto out;
	}

#ifdef CONFIG_NTFS3_FS_POSIX_ACL
	if ((name_len == sizeof(XATTR_NAME_POSIX_ACL_ACCESS) - 1 &&
	     !memcmp(name, XATTR_NAME_POSIX_ACL_ACCESS,
		     sizeof(XATTR_NAME_POSIX_ACL_ACCESS))) ||
	    (name_len == sizeof(XATTR_NAME_POSIX_ACL_DEFAULT) - 1 &&
	     !memcmp(name, XATTR_NAME_POSIX_ACL_DEFAULT,
		     sizeof(XATTR_NAME_POSIX_ACL_DEFAULT)))) {
		/* TODO: init_user_ns? */
		err = ntfs_xattr_get_acl(
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
			&init_user_ns, inode,
#else
			inode,
#endif
			name_len == sizeof(XATTR_NAME_POSIX_ACL_ACCESS) - 1
				? ACL_TYPE_ACCESS
				: ACL_TYPE_DEFAULT,
			buffer, size);
		goto out;
	}
#endif
	/* deal with ntfs extended attribute */
	err = ntfs_get_ea(inode, name, name_len, buffer, size, NULL);

out:
	return err;
}

/*
 * ntfs_setxattr
 *
 * inode_operations::setxattr
 */
static noinline int ntfs_setxattr(const struct xattr_handler *handler,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
				  struct user_namespace *mnt_userns,
#endif
				  struct dentry *de, struct inode *inode,
				  const char *name, const void *value,
				  size_t size, int flags)
{
	int err = -EINVAL;
	struct ntfs_inode *ni = ntfs_i(inode);
	size_t name_len = strlen(name);
	enum FILE_ATTRIBUTE new_fa;

	/* Dispatch request */
	if (name_len == sizeof(SYSTEM_DOS_ATTRIB) - 1 &&
	    !memcmp(name, SYSTEM_DOS_ATTRIB, sizeof(SYSTEM_DOS_ATTRIB))) {
		if (sizeof(u8) != size)
			goto out;
		new_fa = cpu_to_le32(*(u8 *)value);
		goto set_new_fa;
	}

	if (name_len == sizeof(SYSTEM_NTFS_ATTRIB) - 1 &&
	    !memcmp(name, SYSTEM_NTFS_ATTRIB, sizeof(SYSTEM_NTFS_ATTRIB))) {
		if (size != sizeof(u32))
			goto out;
		new_fa = cpu_to_le32(*(u32 *)value);

		if (S_ISREG(inode->i_mode)) {
			/* Process compressed/sparsed in special way*/
			ni_lock(ni);
			err = ni_new_attr_flags(ni, new_fa);
			ni_unlock(ni);
			if (err)
				goto out;
		}
set_new_fa:
		/*
		 * Thanks Mark Harmstone:
		 * keep directory bit consistency
		 */
		if (S_ISDIR(inode->i_mode))
			new_fa |= FILE_ATTRIBUTE_DIRECTORY;
		else
			new_fa &= ~FILE_ATTRIBUTE_DIRECTORY;

		if (ni->std_fa != new_fa) {
			ni->std_fa = new_fa;
			if (new_fa & FILE_ATTRIBUTE_READONLY)
				inode->i_mode &= ~0222;
			else
				inode->i_mode |= 0222;
			/* std attribute always in primary record */
			ni->mi.dirty = true;
			mark_inode_dirty(inode);
		}
		err = 0;

		goto out;
	}

	if (name_len == sizeof(SYSTEM_NTFS_SECURITY) - 1 &&
	    !memcmp(name, SYSTEM_NTFS_SECURITY, sizeof(SYSTEM_NTFS_SECURITY))) {
		/* system.ntfs_security*/
		__le32 security_id;
		bool inserted;
		struct ATTR_STD_INFO5 *std;

		if (!is_ntfs3(ni->mi.sbi)) {
			/*
			 * we should replace ATTR_SECURE
			 * Skip this way cause it is nt4 feature
			 */
			err = -EINVAL;
			goto out;
		}

		if (!is_sd_valid(value, size)) {
			err = -EINVAL;
			ntfs_inode_warn(
				inode,
				"you try to set invalid security descriptor");
			goto out;
		}

		err = ntfs_insert_security(ni->mi.sbi, value, size,
					   &security_id, &inserted);
		if (err)
			goto out;

		ni_lock(ni);
		std = ni_std5(ni);
		if (!std) {
			err = -EINVAL;
		} else if (std->security_id != security_id) {
			std->security_id = ni->std_security_id = security_id;
			/* std attribute always in primary record */
			ni->mi.dirty = true;
			mark_inode_dirty(&ni->vfs_inode);
		}
		ni_unlock(ni);
		goto out;
	}

#ifdef CONFIG_NTFS3_FS_POSIX_ACL
	if ((name_len == sizeof(XATTR_NAME_POSIX_ACL_ACCESS) - 1 &&
	     !memcmp(name, XATTR_NAME_POSIX_ACL_ACCESS,
		     sizeof(XATTR_NAME_POSIX_ACL_ACCESS))) ||
	    (name_len == sizeof(XATTR_NAME_POSIX_ACL_DEFAULT) - 1 &&
	     !memcmp(name, XATTR_NAME_POSIX_ACL_DEFAULT,
		     sizeof(XATTR_NAME_POSIX_ACL_DEFAULT)))) {
		/* TODO: init_user_ns? */
		err = ntfs_xattr_set_acl(
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0)
			&init_user_ns, inode,
#else
			inode,
#endif
			name_len == sizeof(XATTR_NAME_POSIX_ACL_ACCESS) - 1
				? ACL_TYPE_ACCESS
				: ACL_TYPE_DEFAULT,
			value, size);
		goto out;
	}
#endif
	/* deal with ntfs extended attribute */
	err = ntfs_set_ea(inode, name, name_len, value, size, flags, 0);

out:
	return err;
}

static bool ntfs_xattr_user_list(struct dentry *dentry)
{
	return true;
}

static const struct xattr_handler ntfs_xattr_handler = {
	.prefix = "",
	.get = ntfs_getxattr,
	.set = ntfs_setxattr,
	.list = ntfs_xattr_user_list,
};

const struct xattr_handler *ntfs_xattr_handlers[] = {
	&ntfs_xattr_handler,
	NULL,
};
