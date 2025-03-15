
/* SW_SDK: include files */
//#include <common/rt_type.h>
//#include <common/debug/rt_log.h>
//#include <osal/time.h>
//#include <osal/phy_osal.h>
//#include <hal/phy/nic_rtl8226/nic_rtl8226b.h>
#include "rtl_adapter.h"
#include "rtl8226_typedef.h"

static const MMD_REG Rtl8226b_n0_ramcode[] =
{
	 { 31, 0xa436, 0XA016, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa436, 0XA012, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa436, 0XA014, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8010, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X801a, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8024, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X802f, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8051, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8057, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8063, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8068, },
    { 31, 0xa438, 0Xd093, },
    { 31, 0xa438, 0Xd1c4, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X135c, },
    { 31, 0xa438, 0Xd704, },
    { 31, 0xa438, 0X5fbc, },
    { 31, 0xa438, 0Xd504, },
    { 31, 0xa438, 0Xc9f1, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X0fc9, },
    { 31, 0xa438, 0Xbb50, },
    { 31, 0xa438, 0Xd505, },
    { 31, 0xa438, 0Xa202, },
    { 31, 0xa438, 0Xd504, },
    { 31, 0xa438, 0X8c0f, },
    { 31, 0xa438, 0Xd500, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X1519, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X1548, },
    { 31, 0xa438, 0X2f70, },
    { 31, 0xa438, 0X802a, },
    { 31, 0xa438, 0X2f73, },
    { 31, 0xa438, 0X156a, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X155c, },
    { 31, 0xa438, 0Xd505, },
    { 31, 0xa438, 0Xa202, },
    { 31, 0xa438, 0Xd500, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X1551, },
    { 31, 0xa438, 0Xc0c1, },
    { 31, 0xa438, 0Xc0c0, },
    { 31, 0xa438, 0Xd05a, },
    { 31, 0xa438, 0Xd1ba, },
    { 31, 0xa438, 0Xd701, },
    { 31, 0xa438, 0X2529, },
    { 31, 0xa438, 0X022a, },
    { 31, 0xa438, 0Xd0a7, },
    { 31, 0xa438, 0Xd1b9, },
    { 31, 0xa438, 0Xa208, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X080e, },
    { 31, 0xa438, 0Xd701, },
    { 31, 0xa438, 0X408b, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X0a65, },
    { 31, 0xa438, 0Xf003, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X0a6b, },
    { 31, 0xa438, 0Xd701, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X0920, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X0915, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X0909, },
    { 31, 0xa438, 0X228f, },
    { 31, 0xa438, 0X8038, },
    { 31, 0xa438, 0X9801, },
    { 31, 0xa438, 0Xd71e, },
    { 31, 0xa438, 0X5d61, },
    { 31, 0xa438, 0Xd701, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X022a, },
    { 31, 0xa438, 0X2005, },
    { 31, 0xa438, 0X091a, },
    { 31, 0xa438, 0X3bd9, },
    { 31, 0xa438, 0X0919, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X0916, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X14c5, },
    { 31, 0xa438, 0Xd703, },
    { 31, 0xa438, 0X3181, },
    { 31, 0xa438, 0X8061, },
    { 31, 0xa438, 0X60ad, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X135c, },
    { 31, 0xa438, 0Xd703, },
    { 31, 0xa438, 0X5fba, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X0cc7, },
    { 31, 0xa438, 0Xd096, },
    { 31, 0xa438, 0Xd1a9, },
    { 31, 0xa438, 0Xd503, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X0c94, },
	{ 31, 0xa438, 0Xa802, },
    { 31, 0xa438, 0Xa301, },
    { 31, 0xa438, 0Xa801, },
    { 31, 0xa438, 0Xc004, },
    { 31, 0xa438, 0Xd710, },
    { 31, 0xa438, 0X4000, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X1e79, },
    { 31, 0xa436, 0XA026, },
    { 31, 0xa438, 0X1e78, },
    { 31, 0xa436, 0XA024, },
    { 31, 0xa438, 0X0c93, },
    { 31, 0xa436, 0XA022, },
    { 31, 0xa438, 0X0cc5, },
    { 31, 0xa436, 0XA020, },
    { 31, 0xa438, 0X0915, },
    { 31, 0xa436, 0XA006, },
    { 31, 0xa438, 0X020a, },
    { 31, 0xa436, 0XA004, },
    { 31, 0xa438, 0X155b, },
    { 31, 0xa436, 0XA002, },
    { 31, 0xa438, 0X1542, },
    { 31, 0xa436, 0XA000, },
    { 31, 0xa438, 0X0fc7, },
    { 31, 0xa436, 0XA008, },
    { 31, 0xa438, 0Xff00, },
    

};


static const MMD_REG Rtl8226b_n1_ramcode[] =
{
      { 31, 0xa436, 0XA016, },
    { 31, 0xa438, 0X0010, },
    { 31, 0xa436, 0XA012, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa436, 0XA014, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8010, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X801d, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X802c, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X802c, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X802c, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X802c, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X802c, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X802c, },
    { 31, 0xa438, 0Xd700, },
    { 31, 0xa438, 0X6090, },
    { 31, 0xa438, 0X60d1, },
    { 31, 0xa438, 0Xc95c, },
    { 31, 0xa438, 0Xf007, },
    { 31, 0xa438, 0X60b1, },
    { 31, 0xa438, 0Xc95a, },
    { 31, 0xa438, 0Xf004, },
    { 31, 0xa438, 0Xc956, },
    { 31, 0xa438, 0Xf002, },
    { 31, 0xa438, 0Xc94e, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X00cd, },
    { 31, 0xa438, 0Xd700, },
    { 31, 0xa438, 0X6090, },
    { 31, 0xa438, 0X60d1, },
    { 31, 0xa438, 0Xc95c, },
    { 31, 0xa438, 0Xf007, },
    { 31, 0xa438, 0X60b1, },
    { 31, 0xa438, 0Xc95a, },
    { 31, 0xa438, 0Xf004, },
    { 31, 0xa438, 0Xc956, },
    { 31, 0xa438, 0Xf002, },
    { 31, 0xa438, 0Xc94e, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X022a, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X0132, },
    { 31, 0xa436, 0XA08E, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA08C, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA08A, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA088, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA086, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA084, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA082, },
    { 31, 0xa438, 0X012f, },
    { 31, 0xa436, 0XA080, },
    { 31, 0xa438, 0X00cc, },
    { 31, 0xa436, 0XA090, },
    { 31, 0xa438, 0X0103, },
};


static const MMD_REG Rtl8226b_n2_ramcode[] =
{
    { 31, 0xa436, 0XA016, },
    { 31, 0xa438, 0X0020, },
    { 31, 0xa436, 0XA012, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa436, 0XA014, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8010, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8020, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X802a, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X8035, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X803c, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X803c, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X803c, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X803c, },
    { 31, 0xa438, 0Xd107, },
    { 31, 0xa438, 0Xd042, },
    { 31, 0xa438, 0Xa404, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X09df, },
    { 31, 0xa438, 0Xd700, },
    { 31, 0xa438, 0X5fb4, },
    { 31, 0xa438, 0X8280, },
    { 31, 0xa438, 0Xd700, },
    { 31, 0xa438, 0X6065, },
    { 31, 0xa438, 0Xd125, },
    { 31, 0xa438, 0Xf002, },
    { 31, 0xa438, 0Xd12b, },
    { 31, 0xa438, 0Xd040, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X077f, },
    { 31, 0xa438, 0X0cf0, },
    { 31, 0xa438, 0X0c50, },
    { 31, 0xa438, 0Xd104, },
    { 31, 0xa438, 0Xd040, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X0aa8, },
    { 31, 0xa438, 0Xd700, },
    { 31, 0xa438, 0X5fb4, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X0a2e, },
    { 31, 0xa438, 0Xcb9b, },
    { 31, 0xa438, 0Xd110, },
    { 31, 0xa438, 0Xd040, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X0b7b, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X09df, },
    { 31, 0xa438, 0Xd700, },
    { 31, 0xa438, 0X5fb4, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X081b, },
    { 31, 0xa438, 0X1000, },
    { 31, 0xa438, 0X09df, },
    { 31, 0xa438, 0Xd704, },
    { 31, 0xa438, 0X7fb8, },
    { 31, 0xa438, 0Xa718, },
    { 31, 0xa438, 0X1800, },
    { 31, 0xa438, 0X074e, },
    { 31, 0xa436, 0XA10E, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA10C, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA10A, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA108, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0XA106, },
    { 31, 0xa438, 0X074d, },
    { 31, 0xa436, 0XA104, },
    { 31, 0xa438, 0X0818, },
    { 31, 0xa436, 0XA102, },
    { 31, 0xa438, 0X0a2c, },
    { 31, 0xa436, 0XA100, },
    { 31, 0xa438, 0X077e, },
    { 31, 0xa436, 0XA110, },
    { 31, 0xa438, 0X000f, },

};

static const MMD_REG Rtl8226b_uc2_ramcode[] =
{
    { 31, 0xa436, 0Xb87c, },
    { 31, 0xa438, 0X8625, },
    { 31, 0xa436, 0Xb87e, },
    { 31, 0xa438, 0Xaf86, },
    { 31, 0xa438, 0X3daf, },
    { 31, 0xa438, 0X8689, },
    { 31, 0xa438, 0Xaf88, },
    { 31, 0xa438, 0X69af, },
    { 31, 0xa438, 0X8887, },
    { 31, 0xa438, 0Xaf88, },
    { 31, 0xa438, 0X9caf, },
    { 31, 0xa438, 0X889c, },
    { 31, 0xa438, 0Xaf88, },
    { 31, 0xa438, 0X9caf, },
    { 31, 0xa438, 0X889c, },
    { 31, 0xa438, 0Xbf86, },
    { 31, 0xa438, 0X49d7, },
    { 31, 0xa438, 0X0040, },
    { 31, 0xa438, 0X0277, },
    { 31, 0xa438, 0X7daf, },
    { 31, 0xa438, 0X2727, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7205, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7208, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X71f3, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X71f6, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7229, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X722c, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7217, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X721a, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X721d, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7211, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7220, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7214, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X722f, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7223, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7232, },
    { 31, 0xa438, 0X0000, },
    { 31, 0xa438, 0X7226, },
    { 31, 0xa438, 0Xf8f9, },
    { 31, 0xa438, 0Xfae0, },
    { 31, 0xa438, 0X85b3, },
    { 31, 0xa438, 0X3802, },
    { 31, 0xa438, 0Xad27, },
    { 31, 0xa438, 0X02ae, },
    { 31, 0xa438, 0X03af, },
    { 31, 0xa438, 0X8830, },
    { 31, 0xa438, 0X1f66, },
    { 31, 0xa438, 0Xef65, },
    { 31, 0xa438, 0Xbfc2, },
    { 31, 0xa438, 0X1f1a, },
    { 31, 0xa438, 0X96f7, },
    { 31, 0xa438, 0X05ee, },
    { 31, 0xa438, 0Xffd2, },
    { 31, 0xa438, 0X00da, },
    { 31, 0xa438, 0Xf605, },
    { 31, 0xa438, 0Xbfc2, },
    { 31, 0xa438, 0X2f1a, },
    { 31, 0xa438, 0X96f7, },
    { 31, 0xa438, 0X05ee, },
    { 31, 0xa438, 0Xffd2, },
    { 31, 0xa438, 0X00db, },
    { 31, 0xa438, 0Xf605, },
    { 31, 0xa438, 0Xef02, },
    { 31, 0xa438, 0X1f11, },
    { 31, 0xa438, 0X0d42, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X4202, },
    { 31, 0xa438, 0X6e7d, },
    { 31, 0xa438, 0Xef02, },
    { 31, 0xa438, 0X1b03, },
    { 31, 0xa438, 0X1f11, },
    { 31, 0xa438, 0X0d42, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X4502, },
    { 31, 0xa438, 0X6e7d, },
    { 31, 0xa438, 0Xef02, },
    { 31, 0xa438, 0X1a03, },
    { 31, 0xa438, 0X1f11, },
    { 31, 0xa438, 0X0d42, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X4802, },
    { 31, 0xa438, 0X6e7d, },
    { 31, 0xa438, 0Xbfc2, },
    { 31, 0xa438, 0X3f1a, },
    { 31, 0xa438, 0X96f7, },
    { 31, 0xa438, 0X05ee, },
    { 31, 0xa438, 0Xffd2, },
    { 31, 0xa438, 0X00da, },
    { 31, 0xa438, 0Xf605, },
    { 31, 0xa438, 0Xbfc2, },
    { 31, 0xa438, 0X4f1a, },
    { 31, 0xa438, 0X96f7, },
    { 31, 0xa438, 0X05ee, },
    { 31, 0xa438, 0Xffd2, },
    { 31, 0xa438, 0X00db, },
    { 31, 0xa438, 0Xf605, },
    { 31, 0xa438, 0Xef02, },
    { 31, 0xa438, 0X1f11, },
    { 31, 0xa438, 0X0d42, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X4b02, },
    { 31, 0xa438, 0X6e7d, },
    { 31, 0xa438, 0Xef02, },
    { 31, 0xa438, 0X1b03, },
    { 31, 0xa438, 0X1f11, },
    { 31, 0xa438, 0X0d42, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X4e02, },
    { 31, 0xa438, 0X6e7d, },
    { 31, 0xa438, 0Xef02, },
    { 31, 0xa438, 0X1a03, },
    { 31, 0xa438, 0X1f11, },
    { 31, 0xa438, 0X0d42, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X5102, },
    { 31, 0xa438, 0X6e7d, },
    { 31, 0xa438, 0Xef56, },
    { 31, 0xa438, 0Xd020, },
    { 31, 0xa438, 0X1f11, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X5402, },
    { 31, 0xa438, 0X6e7d, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X5702, },
    { 31, 0xa438, 0X6e7d, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X5a02, },
    { 31, 0xa438, 0X6e7d, },
    { 31, 0xa438, 0Xe185, },
    { 31, 0xa438, 0Xa0ef, },
    { 31, 0xa438, 0X0348, },
    { 31, 0xa438, 0X0a28, },
    { 31, 0xa438, 0X05ef, },
    { 31, 0xa438, 0X201b, },
    { 31, 0xa438, 0X01ad, },
    { 31, 0xa438, 0X2735, },
    { 31, 0xa438, 0X1f44, },
    { 31, 0xa438, 0Xe085, },
    { 31, 0xa438, 0X88e1, },
    { 31, 0xa438, 0X8589, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X5d02, },
    { 31, 0xa438, 0X6e7d, },
    { 31, 0xa438, 0Xe085, },
    { 31, 0xa438, 0X8ee1, },
    { 31, 0xa438, 0X858f, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X6002, },
    { 31, 0xa438, 0X6e7d, },
    { 31, 0xa438, 0Xe085, },
    { 31, 0xa438, 0X94e1, },
    { 31, 0xa438, 0X8595, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X6302, },
    { 31, 0xa438, 0X6e7d, },
    { 31, 0xa438, 0Xe085, },
    { 31, 0xa438, 0X9ae1, },
    { 31, 0xa438, 0X859b, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X6602, },
    { 31, 0xa438, 0X6e7d, },
    { 31, 0xa438, 0Xaf88, },
    { 31, 0xa438, 0X3cbf, },
    { 31, 0xa438, 0X883f, },
    { 31, 0xa438, 0X026e, },
    { 31, 0xa438, 0X9cad, },
    { 31, 0xa438, 0X2835, },
    { 31, 0xa438, 0X1f44, },
    { 31, 0xa438, 0Xe08f, },
    { 31, 0xa438, 0Xf8e1, },
    { 31, 0xa438, 0X8ff9, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X5d02, },
    { 31, 0xa438, 0X6e7d, },
    { 31, 0xa438, 0Xe08f, },
    { 31, 0xa438, 0Xfae1, },
    { 31, 0xa438, 0X8ffb, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X6002, },
    { 31, 0xa438, 0X6e7d, },
    { 31, 0xa438, 0Xe08f, },
    { 31, 0xa438, 0Xfce1, },
    { 31, 0xa438, 0X8ffd, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X6302, },
    { 31, 0xa438, 0X6e7d, },
    { 31, 0xa438, 0Xe08f, },
    { 31, 0xa438, 0Xfee1, },
    { 31, 0xa438, 0X8fff, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X6602, },
    { 31, 0xa438, 0X6e7d, },
    { 31, 0xa438, 0Xaf88, },
    { 31, 0xa438, 0X3ce1, },
    { 31, 0xa438, 0X85a1, },
    { 31, 0xa438, 0X1b21, },
    { 31, 0xa438, 0Xad37, },
    { 31, 0xa438, 0X341f, },
    { 31, 0xa438, 0X44e0, },
    { 31, 0xa438, 0X858a, },
    { 31, 0xa438, 0Xe185, },
    { 31, 0xa438, 0X8bbf, },
    { 31, 0xa438, 0X885d, },
    { 31, 0xa438, 0X026e, },
    { 31, 0xa438, 0X7de0, },
    { 31, 0xa438, 0X8590, },
    { 31, 0xa438, 0Xe185, },
    { 31, 0xa438, 0X91bf, },
    { 31, 0xa438, 0X8860, },
    { 31, 0xa438, 0X026e, },
    { 31, 0xa438, 0X7de0, },
    { 31, 0xa438, 0X8596, },
    { 31, 0xa438, 0Xe185, },
    { 31, 0xa438, 0X97bf, },
    { 31, 0xa438, 0X8863, },
    { 31, 0xa438, 0X026e, },
    { 31, 0xa438, 0X7de0, },
    { 31, 0xa438, 0X859c, },
    { 31, 0xa438, 0Xe185, },
    { 31, 0xa438, 0X9dbf, },
    { 31, 0xa438, 0X8866, },
    { 31, 0xa438, 0X026e, },
    { 31, 0xa438, 0X7dae, },
    { 31, 0xa438, 0X401f, },
    { 31, 0xa438, 0X44e0, },
    { 31, 0xa438, 0X858c, },
    { 31, 0xa438, 0Xe185, },
    { 31, 0xa438, 0X8dbf, },
    { 31, 0xa438, 0X885d, },
    { 31, 0xa438, 0X026e, },
    { 31, 0xa438, 0X7de0, },
    { 31, 0xa438, 0X8592, },
    { 31, 0xa438, 0Xe185, },
    { 31, 0xa438, 0X93bf, },
    { 31, 0xa438, 0X8860, },
    { 31, 0xa438, 0X026e, },
    { 31, 0xa438, 0X7de0, },
    { 31, 0xa438, 0X8598, },
    { 31, 0xa438, 0Xe185, },
    { 31, 0xa438, 0X99bf, },
    { 31, 0xa438, 0X8863, },
    { 31, 0xa438, 0X026e, },
    { 31, 0xa438, 0X7de0, },
    { 31, 0xa438, 0X859e, },
    { 31, 0xa438, 0Xe185, },
    { 31, 0xa438, 0X9fbf, },
    { 31, 0xa438, 0X8866, },
    { 31, 0xa438, 0X026e, },
    { 31, 0xa438, 0X7dae, },
    { 31, 0xa438, 0X0ce1, },
    { 31, 0xa438, 0X85b3, },
    { 31, 0xa438, 0X3904, },
    { 31, 0xa438, 0Xac2f, },
    { 31, 0xa438, 0X04ee, },
    { 31, 0xa438, 0X85b3, },
    { 31, 0xa438, 0X00af, },
    { 31, 0xa438, 0X39d9, },
    { 31, 0xa438, 0X22ac, },
    { 31, 0xa438, 0Xeaf0, },
    { 31, 0xa438, 0Xacf6, },
    { 31, 0xa438, 0Xf0ac, },
    { 31, 0xa438, 0Xfaf0, },
    { 31, 0xa438, 0Xacf8, },
    { 31, 0xa438, 0Xf0ac, },
    { 31, 0xa438, 0Xfcf0, },
    { 31, 0xa438, 0Xad00, },
    { 31, 0xa438, 0Xf0ac, },
    { 31, 0xa438, 0Xfef0, },
    { 31, 0xa438, 0Xacf0, },
    { 31, 0xa438, 0Xf0ac, },
    { 31, 0xa438, 0Xf4f0, },
    { 31, 0xa438, 0Xacf2, },
    { 31, 0xa438, 0Xf0ac, },
    { 31, 0xa438, 0Xb0f0, },
    { 31, 0xa438, 0Xacae, },
    { 31, 0xa438, 0Xf0ac, },
    { 31, 0xa438, 0Xacf0, },
    { 31, 0xa438, 0Xacaa, },
    { 31, 0xa438, 0Xa100, },
    { 31, 0xa438, 0X0ce1, },
    { 31, 0xa438, 0X8ff7, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X8402, },
    { 31, 0xa438, 0X6e7d, },
    { 31, 0xa438, 0Xaf26, },
    { 31, 0xa438, 0Xe9e1, },
    { 31, 0xa438, 0X8ff6, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X8402, },
    { 31, 0xa438, 0X6e7d, },
    { 31, 0xa438, 0Xaf26, },
    { 31, 0xa438, 0Xf520, },
    { 31, 0xa438, 0Xac86, },
    { 31, 0xa438, 0Xbf88, },
    { 31, 0xa438, 0X3f02, },
    { 31, 0xa438, 0X6e9c, },
    { 31, 0xa438, 0Xad28, },
    { 31, 0xa438, 0X03af, },
    { 31, 0xa438, 0X3324, },
    { 31, 0xa438, 0Xad38, },
    { 31, 0xa438, 0X03af, },
    { 31, 0xa438, 0X32e6, },
    { 31, 0xa438, 0Xaf32, },
    { 31, 0xa438, 0Xfb00, },
    { 31, 0xa436, 0Xb87c, },
    { 31, 0xa438, 0X8ff6, },
    { 31, 0xa436, 0Xb87e, },
    { 31, 0xa438, 0X0705, },
    { 31, 0xa436, 0Xb87c, },
    { 31, 0xa438, 0X8ff8, },
    { 31, 0xa436, 0Xb87e, },
    { 31, 0xa438, 0X19cc, },
    { 31, 0xa436, 0Xb87c, },
    { 31, 0xa438, 0X8ffa, },
    { 31, 0xa436, 0Xb87e, },
    { 31, 0xa438, 0X28e3, },
    { 31, 0xa436, 0Xb87c, },
    { 31, 0xa438, 0X8ffc, },
    { 31, 0xa436, 0Xb87e, },
    { 31, 0xa438, 0X1047, },
    { 31, 0xa436, 0Xb87c, },
    { 31, 0xa438, 0X8ffe, },
    { 31, 0xa436, 0Xb87e, },
    { 31, 0xa438, 0X0a45, },
    { 31, 0xa436, 0Xb85e, },
    { 31, 0xa438, 0X271E, },
    { 31, 0xa436, 0Xb860, },
    { 31, 0xa438, 0X3846, },
    { 31, 0xa436, 0Xb862, },
    { 31, 0xa438, 0X26E6, },
    { 31, 0xa436, 0Xb864, },
    { 31, 0xa438, 0X32E3, },
    { 31, 0xa436, 0Xb886, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0Xb888, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0Xb88a, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0Xb88c, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0Xb838, },
    { 31, 0xa438, 0X000f, },

};

static const MMD_REG Rtl8226b_uc_ramcode[] =
{
	 { 31, 0xa436, 0X846e, },
    { 31, 0xa438, 0Xaf84, },
    { 31, 0xa438, 0X86af, },
    { 31, 0xa438, 0X8690, },
    { 31, 0xa438, 0Xaf86, },
    { 31, 0xa438, 0Xa4af, },
    { 31, 0xa438, 0X86a4, },
    { 31, 0xa438, 0Xaf86, },
    { 31, 0xa438, 0Xa4af, },
    { 31, 0xa438, 0X86a4, },
    { 31, 0xa438, 0Xaf86, },
    { 31, 0xa438, 0Xa4af, },
    { 31, 0xa438, 0X86a4, },
    { 31, 0xa438, 0Xee82, },
    { 31, 0xa438, 0X5f00, },
    { 31, 0xa438, 0X0284, },
    { 31, 0xa438, 0X90af, },
    { 31, 0xa438, 0X0441, },
    { 31, 0xa438, 0Xf8e0, },
    { 31, 0xa438, 0X8ff3, },
    { 31, 0xa438, 0Xa000, },
    { 31, 0xa438, 0X0502, },
    { 31, 0xa438, 0X84a4, },
    { 31, 0xa438, 0Xae06, },
    { 31, 0xa438, 0Xa001, },
    { 31, 0xa438, 0X0302, },
    { 31, 0xa438, 0X84c8, },
    { 31, 0xa438, 0Xfc04, },
    { 31, 0xa438, 0Xf8f9, },
    { 31, 0xa438, 0Xef59, },
    { 31, 0xa438, 0Xe080, },
    { 31, 0xa438, 0X15ad, },
    { 31, 0xa438, 0X2702, },
    { 31, 0xa438, 0Xae03, },
    { 31, 0xa438, 0Xaf84, },
    { 31, 0xa438, 0Xc3bf, },
    { 31, 0xa438, 0X53ca, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xc8ad, },
    { 31, 0xa438, 0X2807, },
    { 31, 0xa438, 0X0285, },
    { 31, 0xa438, 0X2cee, },
    { 31, 0xa438, 0X8ff3, },
    { 31, 0xa438, 0X01ef, },
    { 31, 0xa438, 0X95fd, },
    { 31, 0xa438, 0Xfc04, },
    { 31, 0xa438, 0Xf8f9, },
    { 31, 0xa438, 0Xfaef, },
    { 31, 0xa438, 0X69bf, },
    { 31, 0xa438, 0X53ca, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xc8ac, },
    { 31, 0xa438, 0X2822, },
    { 31, 0xa438, 0Xd480, },
    { 31, 0xa438, 0X00bf, },
    { 31, 0xa438, 0X8684, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xa9bf, },
    { 31, 0xa438, 0X8687, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xa9bf, },
    { 31, 0xa438, 0X868a, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xa9bf, },
    { 31, 0xa438, 0X868d, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xa9ee, },
    { 31, 0xa438, 0X8ff3, },
    { 31, 0xa438, 0X00af, },
    { 31, 0xa438, 0X8526, },
    { 31, 0xa438, 0Xe08f, },
    { 31, 0xa438, 0Xf4e1, },
    { 31, 0xa438, 0X8ff5, },
    { 31, 0xa438, 0Xe28f, },
    { 31, 0xa438, 0Xf6e3, },
    { 31, 0xa438, 0X8ff7, },
    { 31, 0xa438, 0X1b45, },
    { 31, 0xa438, 0Xac27, },
    { 31, 0xa438, 0X0eee, },
    { 31, 0xa438, 0X8ff4, },
    { 31, 0xa438, 0X00ee, },
    { 31, 0xa438, 0X8ff5, },
    { 31, 0xa438, 0X0002, },
    { 31, 0xa438, 0X852c, },
    { 31, 0xa438, 0Xaf85, },
    { 31, 0xa438, 0X26e0, },
    { 31, 0xa438, 0X8ff4, },
    { 31, 0xa438, 0Xe18f, },
    { 31, 0xa438, 0Xf52c, },
    { 31, 0xa438, 0X0001, },
    { 31, 0xa438, 0Xe48f, },
    { 31, 0xa438, 0Xf4e5, },
    { 31, 0xa438, 0X8ff5, },
    { 31, 0xa438, 0Xef96, },
    { 31, 0xa438, 0Xfefd, },
    { 31, 0xa438, 0Xfc04, },
    { 31, 0xa438, 0Xf8f9, },
    { 31, 0xa438, 0Xef59, },
    { 31, 0xa438, 0Xbf53, },
    { 31, 0xa438, 0X2202, },
    { 31, 0xa438, 0X52c8, },
    { 31, 0xa438, 0Xa18b, },
    { 31, 0xa438, 0X02ae, },
    { 31, 0xa438, 0X03af, },
    { 31, 0xa438, 0X85da, },
    { 31, 0xa438, 0Xbf57, },
    { 31, 0xa438, 0X7202, },
    { 31, 0xa438, 0X52c8, },
    { 31, 0xa438, 0Xe48f, },
    { 31, 0xa438, 0Xf8e5, },
    { 31, 0xa438, 0X8ff9, },
    { 31, 0xa438, 0Xbf57, },
    { 31, 0xa438, 0X7502, },
    { 31, 0xa438, 0X52c8, },
    { 31, 0xa438, 0Xe48f, },
    { 31, 0xa438, 0Xfae5, },
    { 31, 0xa438, 0X8ffb, },
    { 31, 0xa438, 0Xbf57, },
    { 31, 0xa438, 0X7802, },
    { 31, 0xa438, 0X52c8, },
    { 31, 0xa438, 0Xe48f, },
    { 31, 0xa438, 0Xfce5, },
    { 31, 0xa438, 0X8ffd, },
    { 31, 0xa438, 0Xbf57, },
    { 31, 0xa438, 0X7b02, },
    { 31, 0xa438, 0X52c8, },
    { 31, 0xa438, 0Xe48f, },
    { 31, 0xa438, 0Xfee5, },
    { 31, 0xa438, 0X8fff, },
    { 31, 0xa438, 0Xbf57, },
    { 31, 0xa438, 0X6c02, },
    { 31, 0xa438, 0X52c8, },
    { 31, 0xa438, 0Xa102, },
    { 31, 0xa438, 0X13ee, },
    { 31, 0xa438, 0X8ffc, },
    { 31, 0xa438, 0X80ee, },
    { 31, 0xa438, 0X8ffd, },
    { 31, 0xa438, 0X00ee, },
    { 31, 0xa438, 0X8ffe, },
    { 31, 0xa438, 0X80ee, },
    { 31, 0xa438, 0X8fff, },
    { 31, 0xa438, 0X00af, },
    { 31, 0xa438, 0X8599, },
    { 31, 0xa438, 0Xa101, },
    { 31, 0xa438, 0X0cbf, },
    { 31, 0xa438, 0X534c, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xc8a1, },
    { 31, 0xa438, 0X0303, },
    { 31, 0xa438, 0Xaf85, },
    { 31, 0xa438, 0X77bf, },
    { 31, 0xa438, 0X5322, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xc8a1, },
    { 31, 0xa438, 0X8b02, },
    { 31, 0xa438, 0Xae03, },
    { 31, 0xa438, 0Xaf86, },
    { 31, 0xa438, 0X64e0, },
    { 31, 0xa438, 0X8ff8, },
    { 31, 0xa438, 0Xe18f, },
    { 31, 0xa438, 0Xf9bf, },
    { 31, 0xa438, 0X8684, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xa9e0, },
    { 31, 0xa438, 0X8ffa, },
    { 31, 0xa438, 0Xe18f, },
    { 31, 0xa438, 0Xfbbf, },
    { 31, 0xa438, 0X8687, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xa9e0, },
    { 31, 0xa438, 0X8ffc, },
    { 31, 0xa438, 0Xe18f, },
    { 31, 0xa438, 0Xfdbf, },
    { 31, 0xa438, 0X868a, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xa9e0, },
    { 31, 0xa438, 0X8ffe, },
    { 31, 0xa438, 0Xe18f, },
    { 31, 0xa438, 0Xffbf, },
    { 31, 0xa438, 0X868d, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xa9af, },
    { 31, 0xa438, 0X867f, },
    { 31, 0xa438, 0Xbf53, },
    { 31, 0xa438, 0X2202, },
    { 31, 0xa438, 0X52c8, },
    { 31, 0xa438, 0Xa144, },
    { 31, 0xa438, 0X3cbf, },
    { 31, 0xa438, 0X547b, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xc8e4, },
    { 31, 0xa438, 0X8ff8, },
    { 31, 0xa438, 0Xe58f, },
    { 31, 0xa438, 0Xf9bf, },
    { 31, 0xa438, 0X547e, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xc8e4, },
    { 31, 0xa438, 0X8ffa, },
    { 31, 0xa438, 0Xe58f, },
    { 31, 0xa438, 0Xfbbf, },
    { 31, 0xa438, 0X5481, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xc8e4, },
    { 31, 0xa438, 0X8ffc, },
    { 31, 0xa438, 0Xe58f, },
    { 31, 0xa438, 0Xfdbf, },
    { 31, 0xa438, 0X5484, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xc8e4, },
    { 31, 0xa438, 0X8ffe, },
    { 31, 0xa438, 0Xe58f, },
    { 31, 0xa438, 0Xffbf, },
    { 31, 0xa438, 0X5322, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xc8a1, },
    { 31, 0xa438, 0X4448, },
    { 31, 0xa438, 0Xaf85, },
    { 31, 0xa438, 0Xa7bf, },
    { 31, 0xa438, 0X5322, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xc8a1, },
    { 31, 0xa438, 0X313c, },
    { 31, 0xa438, 0Xbf54, },
    { 31, 0xa438, 0X7b02, },
    { 31, 0xa438, 0X52c8, },
    { 31, 0xa438, 0Xe48f, },
    { 31, 0xa438, 0Xf8e5, },
    { 31, 0xa438, 0X8ff9, },
    { 31, 0xa438, 0Xbf54, },
    { 31, 0xa438, 0X7e02, },
    { 31, 0xa438, 0X52c8, },
    { 31, 0xa438, 0Xe48f, },
    { 31, 0xa438, 0Xfae5, },
    { 31, 0xa438, 0X8ffb, },
    { 31, 0xa438, 0Xbf54, },
    { 31, 0xa438, 0X8102, },
    { 31, 0xa438, 0X52c8, },
    { 31, 0xa438, 0Xe48f, },
    { 31, 0xa438, 0Xfce5, },
    { 31, 0xa438, 0X8ffd, },
    { 31, 0xa438, 0Xbf54, },
    { 31, 0xa438, 0X8402, },
    { 31, 0xa438, 0X52c8, },
    { 31, 0xa438, 0Xe48f, },
    { 31, 0xa438, 0Xfee5, },
    { 31, 0xa438, 0X8fff, },
    { 31, 0xa438, 0Xbf53, },
    { 31, 0xa438, 0X2202, },
    { 31, 0xa438, 0X52c8, },
    { 31, 0xa438, 0Xa131, },
    { 31, 0xa438, 0X03af, },
    { 31, 0xa438, 0X85a7, },
    { 31, 0xa438, 0Xd480, },
    { 31, 0xa438, 0X00bf, },
    { 31, 0xa438, 0X8684, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xa9bf, },
    { 31, 0xa438, 0X8687, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xa9bf, },
    { 31, 0xa438, 0X868a, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xa9bf, },
    { 31, 0xa438, 0X868d, },
    { 31, 0xa438, 0X0252, },
    { 31, 0xa438, 0Xa9ef, },
    { 31, 0xa438, 0X95fd, },
    { 31, 0xa438, 0Xfc04, },
    { 31, 0xa438, 0Xf0d1, },
    { 31, 0xa438, 0X2af0, },
    { 31, 0xa438, 0Xd12c, },
    { 31, 0xa438, 0Xf0d1, },
    { 31, 0xa438, 0X44f0, },
    { 31, 0xa438, 0Xd146, },
    { 31, 0xa438, 0Xbf86, },
    { 31, 0xa438, 0Xa102, },
    { 31, 0xa438, 0X52c8, },
    { 31, 0xa438, 0Xbf86, },
    { 31, 0xa438, 0Xa102, },
    { 31, 0xa438, 0X52c8, },
    { 31, 0xa438, 0Xd101, },
    { 31, 0xa438, 0Xaf06, },
    { 31, 0xa438, 0Xa570, },
    { 31, 0xa438, 0Xce42, },
    { 31, 0xa436, 0Xb818, },
    { 31, 0xa438, 0X043d, },
    { 31, 0xa436, 0Xb81a, },
    { 31, 0xa438, 0X06a3, },
    { 31, 0xa436, 0Xb81c, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0Xb81e, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0Xb850, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0Xb852, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0Xb878, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0Xb884, },
    { 31, 0xa438, 0Xffff, },
    { 31, 0xa436, 0Xb832, },
    { 31, 0xa438, 0X0003, },
    
};

static const MMD_REG Rtl8226b_data_ramcode[] =
{
   
};

static const MMD_REG Rtl8226b_isram_patch[] =
{
   
};

static BOOL
Rtl8226b_wait_for_bit(
    IN HANDLE hDevice,
    IN UINT16 dev,
    IN UINT16 addr,
    IN UINT16 mask,
    IN BOOL   set,
    IN UINT16 timeoutms)
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    while (--timeoutms) {
        status = MmdPhyRead(hDevice, MMD_VEND2, addr, &phydata);
        if (!status)
            goto exit;

        if (!set)
            phydata = ~phydata;

        if ((phydata & mask) == mask)
            return 1;

        Sleep(1);
    }

    osal_printf("Timeout (dev=%02x addr=0x%02x mask=0x%02x timeout=%d)\n",
          dev, addr, mask, timeoutms);

exit:
    return 0;
}

BOOLEAN
Rtl8226b_phy_init(
    IN HANDLE hDevice,
    IN PHY_LINK_ABILITY *pphylinkability,
    IN BOOL singlephy
    )
{
    BOOL status = FAILURE;
    UINT16 i = 0;   /* SW_SDK: use UINT16 instead of UINT8, for MMD_REG array may over 255 entries */
    UINT16 phydata = 0;
    const UINT16 patchver = 0x0019, patchaddr = 0x8024;

    // Polling PHY Status
    status = Rtl8226b_wait_for_bit(hDevice, MMD_VEND2, 0xA420, 0x3, 1, 100);
    if (status != SUCCESS)
        goto exit;

    // MMD 31.0xA436[15:0] = 0x801E
    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, 0x801E);
    if (status != SUCCESS)
        goto exit;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA438, &phydata);
    if (status != SUCCESS)
        goto exit;

    // Already patched.
    if (phydata == patchver)
    {
        status = 1;
        goto exit;
    }
    else
    {
        // Patch request & wait patch_rdy (for normal patch flow - Driver Initialize)
        // MMD 31.0xB820[4] = 1'b1     //(patch request)
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB820, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= BIT_4;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB820, phydata);
        if (status != SUCCESS)
            goto exit;

        //wait for patch ready = 1 (MMD 31.0xB800[6])
        status = Rtl8226b_wait_for_bit(hDevice, MMD_VEND2, 0xB800, BIT_6, 1, 100);
        if (status != SUCCESS)
            goto exit;

        //Set patch_key & patch_lock
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, patchaddr);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA438[15:0] = 0x3701
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA438, 0x3701);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA436[15:0] = 0xB82E
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, 0xB82E);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA438[15:0] = 0x0001
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA438, 0x0001);
        if (status != SUCCESS)
            goto exit;

        // NC & UC patch
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB820, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= BIT_7;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB820, phydata);
        if (status != SUCCESS)
            goto exit;

        // patch nc0
        for(i=0; i<sizeof(Rtl8226b_n0_ramcode)/sizeof(MMD_REG); i++)
        {
            status = MmdPhyWrite(hDevice, Rtl8226b_n0_ramcode[i].dev, Rtl8226b_n0_ramcode[i].addr, Rtl8226b_n0_ramcode[i].value);
            if (status != SUCCESS)
                goto exit;
        }
        phy_osal_printf("\n");
        phy_osal_printf("load nc0 ramcode complete!\n");

        // patch nc1
        for(i=0; i<sizeof(Rtl8226b_n1_ramcode)/sizeof(MMD_REG); i++)
        {
            status = MmdPhyWrite(hDevice, Rtl8226b_n1_ramcode[i].dev, Rtl8226b_n1_ramcode[i].addr, Rtl8226b_n1_ramcode[i].value);
            if (status != SUCCESS)
                goto exit;
        }
        phy_osal_printf("\n");
        phy_osal_printf("load nc1 ramcode complete!\n");


        // patch nc2
        for(i=0; i<sizeof(Rtl8226b_n2_ramcode)/sizeof(MMD_REG); i++)
        {
            status = MmdPhyWrite(hDevice, Rtl8226b_n2_ramcode[i].dev, Rtl8226b_n2_ramcode[i].addr, Rtl8226b_n2_ramcode[i].value);
            if (status != SUCCESS)
                goto exit;
        }
        phy_osal_printf("\n");
        phy_osal_printf("load nc2 ramcode complete!\n");

        // patch uc2
        for(i=0; i<sizeof(Rtl8226b_uc2_ramcode)/sizeof(MMD_REG); i++)
        {
            status = MmdPhyWrite(hDevice, Rtl8226b_uc2_ramcode[i].dev, Rtl8226b_uc2_ramcode[i].addr, Rtl8226b_uc2_ramcode[i].value);
            
			if (status != SUCCESS)
                goto exit;
        }
        phy_osal_printf("\n");
        phy_osal_printf("load uc2 ramcode complete!\n");

        // MMD 31.0xB820[7] = 1'b0
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB820, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~BIT_7);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB820, phydata);
        if (status != SUCCESS)
            goto exit;

        // patch uc
        for(i=0; i<sizeof(Rtl8226b_uc_ramcode)/sizeof(MMD_REG); i++)
        {
            status = MmdPhyWrite(hDevice, Rtl8226b_uc_ramcode[i].dev, Rtl8226b_uc_ramcode[i].addr, Rtl8226b_uc_ramcode[i].value);
            if (status != SUCCESS)
                goto exit;
        }
        phy_osal_printf("\n");
        phy_osal_printf("load uc ramcode complete!\n");

        // GPHY OCP 0xB896 bit[0] = 0x0
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB896, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~BIT_0);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB896, phydata);
        if (status != SUCCESS)
            goto exit;

        // GPHY OCP 0xB892 bit[15:8] = 0x0
        phydata = 0;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB892, phydata);
        if (status != SUCCESS)
            goto exit;

        // patch ram code
        for(i=0; i<sizeof(Rtl8226b_data_ramcode)/sizeof(MMD_REG); i++)
        {
            status = MmdPhyWrite(hDevice, Rtl8226b_data_ramcode[i].dev, Rtl8226b_data_ramcode[i].addr, Rtl8226b_data_ramcode[i].value);
            if (status != SUCCESS)
                goto exit;
        }
        phy_osal_printf("\n");
        phy_osal_printf("load data ramcode complete!\n");

        // GPHY OCP 0xB896 bit[0] = 0x1
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB896, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= BIT_0;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB896, phydata);
        if (status != SUCCESS)
            goto exit;

        // Clear patch_key & patch_lock
        phydata = 0x0;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, phydata);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA438[15:0] = 0x0000
        phydata = 0x0;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA438, phydata);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xB82E[0] = 1'b0
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB82E, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~BIT_0);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB82E, phydata);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA436[15:0] = patch_key_addr
        phydata = patchaddr;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, phydata);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA438[15:0] = 0x0000
        phydata = 0x0;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA438, phydata);
        if (status != SUCCESS)
            goto exit;

        // Release patch request
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xB820, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~BIT_4);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB820, phydata);
        if (status != SUCCESS)
            goto exit;

        status = Rtl8226b_wait_for_bit(hDevice, MMD_VEND2, 0xB800, BIT_6, 0, 100);
        if (status != SUCCESS)
            goto exit;

      


		
        // Lock Main
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa46A, 0x0302);
        if (status != SUCCESS)
            goto exit;


	
// GPHY REG
  // Patch XG INRX parameters
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xac46, 0xB794);
        if (status != SUCCESS)
            goto exit;	
		
		
			
		
		
		
        // Patch Fnet/ Giga CHNEST

        // normal patch
    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa412, 0x0200);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa5d4, 0x0081);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xad30, 0x0A57);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xad30, 0x0A55);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xb87c, 0x80F5);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xb87e, 0x760E);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xb87c, 0x8107);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xb87e, 0x360E);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xb87c, 0x8551);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xb87e, 0x80E);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xbf00, 0xB202);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xbf46, 0x0300);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8044);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x804A);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8050);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8056);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x805C);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8062);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8068);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x806E);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8074);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x807A);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x240F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8045);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x804B);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8051);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8057);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x805D);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8063);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8069);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x806F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8075);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x807B);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x1700);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA4CA, 0x6A50);
        if (status != SUCCESS)
            goto exit;
		
		status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8FF4);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x0000);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8FF6);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x03E8);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8FF8);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x8000);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8FFA);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x8000);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8FFC);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x8000);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8FFE);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0x8000);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8015);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0xF9BF);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xd12A, 0x8000);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xd12C, 0x8000);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xd144, 0x8000);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xd146, 0x8000);
        if (status != SUCCESS)
            goto exit;
		
		
		status = MmdPhyWrite(hDevice, MMD_VEND2, 0xbf84, 0xAC00);
        if (status != SUCCESS)
            goto exit;
		
		status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa436, 0x8170);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa438, 0xd8a0);
        if (status != SUCCESS)
            goto exit;

		
		
        // Release Lock Main
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xa46A, 0x0300);
        if (status != SUCCESS)
            goto exit;
		
	
//--------------- SDS patch --------------n
    
       status = MmdPhyWrite(hDevice, MMD_VEND1, 0x75B5, 0xE086);
        if (status != SUCCESS)
            goto exit;
		
		
		
//ISRAM PATCH
        if (singlephy)
        {
            phy_osal_printf("load isram patch ramcode:\n");
            for(i=0; i<sizeof(Rtl8226b_isram_patch)/sizeof(MMD_REG); i++)
            {
                status = MmdPhyWrite(hDevice, Rtl8226b_isram_patch[i].dev, Rtl8226b_isram_patch[i].addr, Rtl8226b_isram_patch[i].value);
                if (status != SUCCESS)
                    goto exit;
            }
            phy_osal_printf("\n");
            phy_osal_printf("load isram patch complete!\n");
        }


      status = MmdPhyWrite(hDevice, MMD_VEND1, 0x75B6, 0x0024);
        if (status != SUCCESS)
            goto exit;




  // Update patch version
        phydata = 0x801E;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, phydata);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA438[15:0] =  driver_note_ver
        phydata = patchver;
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA438, phydata);
        if (status != SUCCESS)
            goto exit;
		



        // PHYRST & Restart Nway
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA400, 0x9200);
        if (status != SUCCESS)
            goto exit;
    }

exit:
    return status;
}

