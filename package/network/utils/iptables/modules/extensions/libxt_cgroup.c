#include <stdio.h>
#include <xtables.h>
#include <linux/netfilter/xt_cgroup.h>

enum {
	O_CLASSID = 0,
	O_PATH = 1,
};

static void cgroup_help_v0(void)
{
	printf(
"cgroup match options:\n"
"[!] --cgroup classid            Match cgroup classid\n");
}

static void cgroup_help_v1(void)
{
	printf(
"cgroup match options:\n"
"[!] --path path                 Recursively match path relative to cgroup2 root\n"
"[!] --cgroup classid            Match cgroup classid, can't be used with --path\n");
}

static const struct xt_option_entry cgroup_opts_v0[] = {
	{
		.name = "cgroup",
		.id = O_CLASSID,
		.type = XTTYPE_UINT32,
		.flags = XTOPT_INVERT | XTOPT_MAND | XTOPT_PUT,
		XTOPT_POINTER(struct xt_cgroup_info_v0, id)
	},
	XTOPT_TABLEEND,
};

static const struct xt_option_entry cgroup_opts_v1[] = {
	{
		.name = "path",
		.id = O_PATH,
		.type = XTTYPE_STRING,
		.flags = XTOPT_INVERT | XTOPT_PUT,
		XTOPT_POINTER(struct xt_cgroup_info_v1, path)
	},
	{
		.name = "cgroup",
		.id = O_CLASSID,
		.type = XTTYPE_UINT32,
		.flags = XTOPT_INVERT | XTOPT_PUT,
		XTOPT_POINTER(struct xt_cgroup_info_v1, classid)
	},
	XTOPT_TABLEEND,
};

static const struct xt_option_entry cgroup_opts_v2[] = {
	{
		.name = "path",
		.id = O_PATH,
		.type = XTTYPE_STRING,
		.flags = XTOPT_INVERT | XTOPT_PUT,
		XTOPT_POINTER(struct xt_cgroup_info_v2, path)
	},
	{
		.name = "cgroup",
		.id = O_CLASSID,
		.type = XTTYPE_UINT32,
		.flags = XTOPT_INVERT | XTOPT_PUT,
		XTOPT_POINTER(struct xt_cgroup_info_v2, classid)
	},
	XTOPT_TABLEEND,
};

static void cgroup_parse_v0(struct xt_option_call *cb)
{
	struct xt_cgroup_info_v0 *cgroupinfo = cb->data;

	xtables_option_parse(cb);
	if (cb->invert)
		cgroupinfo->invert = true;
}

static void cgroup_parse_v1(struct xt_option_call *cb)
{
	struct xt_cgroup_info_v1 *info = cb->data;

	xtables_option_parse(cb);

	switch (cb->entry->id) {
	case O_PATH:
		info->has_path = true;
		if (cb->invert)
			info->invert_path = true;
		break;
	case O_CLASSID:
		info->has_classid = true;
		if (cb->invert)
			info->invert_classid = true;
		break;
	}
}

static void cgroup_parse_v2(struct xt_option_call *cb)
{
	struct xt_cgroup_info_v2 *info = cb->data;

	xtables_option_parse(cb);

	switch (cb->entry->id) {
	case O_PATH:
		info->has_path = true;
		if (cb->invert)
			info->invert_path = true;
		break;
	case O_CLASSID:
		info->has_classid = true;
		if (cb->invert)
			info->invert_classid = true;
		break;
	}
}

static void
cgroup_print_v0(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_cgroup_info_v0 *info = (void *) match->data;

	printf(" cgroup %s%u", info->invert ? "! ":"", info->id);
}

static void cgroup_save_v0(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_cgroup_info_v0 *info = (void *) match->data;

	printf("%s --cgroup %u", info->invert ? " !" : "", info->id);
}

static void
cgroup_print_v1(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_cgroup_info_v1 *info = (void *)match->data;

	printf(" cgroup");
	if (info->has_path)
		printf(" %s%s", info->invert_path ? "! ":"", info->path);
	if (info->has_classid)
		printf(" %s%u", info->invert_classid ? "! ":"", info->classid);
}

static void cgroup_save_v1(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_cgroup_info_v1 *info = (void *)match->data;

	if (info->has_path) {
		printf("%s --path", info->invert_path ? " !" : "");
		xtables_save_string(info->path);
	}

	if (info->has_classid)
		printf("%s --cgroup %u", info->invert_classid ? " !" : "",
		       info->classid);
}

static void
cgroup_print_v2(const void *ip, const struct xt_entry_match *match, int numeric)
{
	const struct xt_cgroup_info_v2 *info = (void *)match->data;

	printf(" cgroup");
	if (info->has_path)
		printf(" %s%s", info->invert_path ? "! ":"", info->path);
	if (info->has_classid)
		printf(" %s%u", info->invert_classid ? "! ":"", info->classid);
}

static void cgroup_save_v2(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_cgroup_info_v2 *info = (void *)match->data;

	if (info->has_path) {
		printf("%s --path", info->invert_path ? " !" : "");
		xtables_save_string(info->path);
	}

	if (info->has_classid)
		printf("%s --cgroup %u", info->invert_classid ? " !" : "",
		       info->classid);
}

static int cgroup_xlate_v0(struct xt_xlate *xl,
			   const struct xt_xlate_mt_params *params)
{
	const struct xt_cgroup_info_v0 *info = (void *)params->match->data;

	xt_xlate_add(xl, "meta cgroup %s%u", info->invert ? "!= " : "",
		     info->id);
	return 1;
}

static int cgroup_xlate_v1(struct xt_xlate *xl,
			   const struct xt_xlate_mt_params *params)
{
	const struct xt_cgroup_info_v1 *info = (void *)params->match->data;

	if (info->has_path)
		return 0;

	if (info->has_classid)
		xt_xlate_add(xl, "meta cgroup %s%u",
			     info->invert_classid ? "!= " : "",
			     info->classid);

	return 1;
}

static int cgroup_xlate_v2(struct xt_xlate *xl,
			   const struct xt_xlate_mt_params *params)
{
	const struct xt_cgroup_info_v2 *info = (void *)params->match->data;

	if (info->has_path)
		return 0;

	if (info->has_classid)
		xt_xlate_add(xl, "meta cgroup %s%u",
			     info->invert_classid ? "!= " : "",
			     info->classid);

	return 1;
}

static struct xtables_match cgroup_match[] = {
	{
		.family		= NFPROTO_UNSPEC,
		.revision	= 0,
		.name		= "cgroup",
		.version	= XTABLES_VERSION,
		.size		= XT_ALIGN(sizeof(struct xt_cgroup_info_v0)),
		.userspacesize	= XT_ALIGN(sizeof(struct xt_cgroup_info_v0)),
		.help		= cgroup_help_v0,
		.print		= cgroup_print_v0,
		.save		= cgroup_save_v0,
		.x6_parse	= cgroup_parse_v0,
		.x6_options	= cgroup_opts_v0,
		.xlate		= cgroup_xlate_v0,
	},
	{
		.family		= NFPROTO_UNSPEC,
		.revision	= 1,
		.name		= "cgroup",
		.version	= XTABLES_VERSION,
		.size		= XT_ALIGN(sizeof(struct xt_cgroup_info_v1)),
		.userspacesize	= offsetof(struct xt_cgroup_info_v1, priv),
		.help		= cgroup_help_v1,
		.print		= cgroup_print_v1,
		.save		= cgroup_save_v1,
		.x6_parse	= cgroup_parse_v1,
		.x6_options	= cgroup_opts_v1,
		.xlate		= cgroup_xlate_v1,
	},
	{
		.family		= NFPROTO_UNSPEC,
		.revision	= 2,
		.name		= "cgroup",
		.version	= XTABLES_VERSION,
		.size		= XT_ALIGN(sizeof(struct xt_cgroup_info_v2)),
		.userspacesize	= offsetof(struct xt_cgroup_info_v2, priv),
		.help		= cgroup_help_v1,
		.print		= cgroup_print_v2,
		.save		= cgroup_save_v2,
		.x6_parse	= cgroup_parse_v2,
		.x6_options	= cgroup_opts_v2,
		.xlate		= cgroup_xlate_v2,
	},
};

void _init(void)
{
	xtables_register_matches(cgroup_match, ARRAY_SIZE(cgroup_match));
}
