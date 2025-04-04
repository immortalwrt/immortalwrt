#ifndef	__ECC_H__
#define	__ECC_H__

#include "security/bn.h"
#include "security/sae_cmm.h"

typedef struct __EC_GROUP_INFO EC_GROUP_INFO;

typedef struct __EC_GROUP_INFO_BI {
	INT32 group_id;
	SAE_BN *prime;
	SAE_BN *order;
	SAE_BN *a;
	SAE_BN *b;
	SAE_BN *gx;
	SAE_BN *gy;
	SAE_BN *z; /* sswu curve parameter */
	SAE_BN *cofactor;
	BIG_INTEGER_EC_POINT *generator;
	MONT_STRUC *mont;
	UCHAR is_minus_3;
	UCHAR is_init;
	EC_GROUP_INFO *ec_group;
} EC_GROUP_INFO_BI, *PEC_GROUP_INFO_BI;

typedef struct __EC_GROUP_INFO {
	INT32 group_id;
	const UCHAR *prime;
	UINT32 prime_len;
	const UCHAR *order;
	UINT32 order_len;
	const UCHAR *a;
	UINT32 a_len;
	const UCHAR *b;
	UINT32 b_len;
	const UCHAR *z; /* sswu curve parameter */
	UINT32 z_len;
	const UCHAR *X;
	UINT32 X_len;
	const UCHAR *R;
	UINT32 R_len;
	const UCHAR *PInverse;
	UINT32 PInverse_len;
	UINT16 prime_len_bit;
} EC_GROUP_INFO, *PEC_GROUP_INFO;

struct __EC_GROUP_INFO *get_ecc_group_info(
	IN INT32 group);

EC_GROUP_INFO_BI *get_ecc_group_info_bi(
	IN INT32 group);

VOID group_info_bi_deinit(
	VOID);

VOID ecc_point_init(
	IN BIG_INTEGER_EC_POINT * *ec_point_res);

VOID ecc_point_free(
	IN BIG_INTEGER_EC_POINT * *ec_point_res);

VOID ecc_point_copy(
	IN BIG_INTEGER_EC_POINT * point,
	OUT BIG_INTEGER_EC_POINT * *ec_point_res);

/* if (x1, y2) != (x2, y2), but x1 = x2 => result is infinity */
/* if point1 + point2 and point1 is infinity => result is point2 */
VOID ecc_point_add(
	IN BIG_INTEGER_EC_POINT * point,
	IN BIG_INTEGER_EC_POINT * point2,
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT * *ec_point_res);
/* if point is infinity, result is infinity */
VOID ecc_point_double(
	IN BIG_INTEGER_EC_POINT * point,
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT * *ec_point_res);

VOID ecc_point_add_3d(
	IN BIG_INTEGER_EC_POINT * point,
	IN BIG_INTEGER_EC_POINT * point2,
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT * *ec_point_res);

VOID ecc_point_double_3d(
	IN BIG_INTEGER_EC_POINT * point,
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT * *ec_point_res);

VOID ecc_point_3d_to_2d(
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	INOUT BIG_INTEGER_EC_POINT * ec_point_res);

VOID ecc_point_set_z_to_one(
	INOUT BIG_INTEGER_EC_POINT * ec_point_res);

/* https://en.wikipedia.org/wiki/Elliptic_curve_point_multiplication */
/* double and add */
VOID ecc_point_mul_dblandadd(
	IN BIG_INTEGER_EC_POINT * point,
	IN SAE_BN *scalar,
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT * *ec_point_res);

/* https://en.wikipedia.org/wiki/Elliptic_curve_point_multiplication */
/* w-ary non-adjacent form (wNAF) method */
VOID ecc_point_mul_wNAF(
	IN BIG_INTEGER_EC_POINT * point,
	IN SAE_BN *scalar,
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT * *ec_point_res);

/* https://en.wikipedia.org/wiki/Elliptic_curve_point_multiplication */
/* w-ary non-adjacent form (wNAF) method */
VOID ecc_point_mul_windowed(
	IN BIG_INTEGER_EC_POINT * point,
	IN SAE_BN *scalar,
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	OUT BIG_INTEGER_EC_POINT * *ec_point_res);

/* (x,y) => (x, p-y) */
VOID ecc_point_inverse(
	IN BIG_INTEGER_EC_POINT * point,
	IN SAE_BN *prime,
	OUT BIG_INTEGER_EC_POINT * *point_res);

UCHAR ecc_point_is_on_curve(
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	IN BIG_INTEGER_EC_POINT * point);

UCHAR ecc_point_is_on_curve_3d(
	IN EC_GROUP_INFO_BI *ec_group_bi,
	IN BIG_INTEGER_EC_POINT * point);

/* y^2 = x^3 + ax + b  */
UCHAR ecc_point_find_by_x(
	IN struct __EC_GROUP_INFO_BI *ec_group_bi,
	IN SAE_BN *x,
	IN SAE_BN **res_y,
	IN UCHAR need_res_y);

VOID ecc_point_dump_time(
	VOID);

BIG_INTEGER_EC_POINT *ecc_point_add_cmm(
	IN BIG_INTEGER_EC_POINT *point,
	IN BIG_INTEGER_EC_POINT *point2,
	IN SAE_BN *lamda,
	IN EC_GROUP_INFO_BI *ec_group_bi);

INT ecc_gen_key(
		IN EC_GROUP_INFO_BI *ec_group_bi,
		INOUT SAE_BN **priv_key,
		INOUT VOID **pub_key);

/* asn1 */
#define MAX_ASN1_SUB_PUB_KEY_INFO_LEN 160
#define MAX_ASN1_ECDSA_SIG_VALUE_LEN (SAE_MAX_ECC_PRIME_LEN * 2 + 2 + 6) /* 2 is zero padding */
UCHAR asn1_get_sub_pub_key_info(
	IN EC_GROUP_INFO_BI *ec_group_bi,
	IN BIG_INTEGER_EC_POINT *pub_key,
	IN UCHAR is_compressed,
	OUT UCHAR *asn1_out,
	OUT UINT32 *asn1_len);

UCHAR asn1_get_pub_key_from_sub_pub_key_info(
	IN UCHAR *asn1,
	IN UINT32 asn1_len,
	OUT UCHAR *group_id,
	OUT VOID **group_bi,
	OUT BIG_INTEGER_EC_POINT **pub_key);

UCHAR asn1_get_ecdsa_sig_value(
	IN EC_GROUP_INFO_BI *ec_group_bi,
	IN SAE_BN *sig_r,
	IN SAE_BN *sig_s,
	OUT UCHAR *asn1_out,
	OUT UINT32 *asn1_len);

UCHAR asn1_get_sig_from_ecdsa_sig_value(
	IN EC_GROUP_INFO_BI *ec_group_bi,
	IN UCHAR *asn1,
	IN UINT32 asn1_len,
	OUT SAE_BN **sig_r,
	OUT SAE_BN **sig_s);

UCHAR asn1_get_private_key(
	IN UCHAR *asn1,
	IN UINT32 asn1_len,
	OUT SAE_BN * *private_key,
	OUT UCHAR *group_id);
#endif /* __ECC_H__ */
