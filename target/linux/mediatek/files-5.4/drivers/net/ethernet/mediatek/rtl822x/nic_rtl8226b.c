/*
 * Copyright (C) 2019 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : PHY 8226 Driver
 *
 * Feature : PHY 8226 Driver
 *
 */


//#include "typedef.h"
//#include "rtl8156_mmd.h"
//#include "phy_rtl8226.h"
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
#include <soc/type.h>
#include <osal/time.h>
//#include <osal/phy_osal.h>
#include <hal/phy/nic_rtl8226/nic_rtl8226b.h>





BOOLEAN
Rtl8226b_phy_reset(
    IN HANDLE hDevice
    )
{
    BOOL status = FAILURE;
    UINT16 phydata0 = 0, phydata1 = 0;
    UINT16 waitcount = 0;

    status = MmdPhyRead(hDevice, MMD_PMAPMD, 0x0, &phydata0);
    if (status != SUCCESS)
        goto exit;

    phydata1 |= BIT_15;

    status = MmdPhyWrite(hDevice, MMD_PMAPMD, 0x0, phydata1);
    if (status != SUCCESS)
        goto exit;

    while(TRUE)
    {
        status = MmdPhyRead(hDevice, MMD_PMAPMD, 0x0, &phydata1);
        if (status != SUCCESS)
            goto exit;

        if (!(phydata1 & BIT_15))
            break;

        if (++waitcount == 500)
        {
            status = FAILURE;
            goto exit;
        }
    }

    status = MmdPhyWrite(hDevice, MMD_PMAPMD, 0x0, phydata0);
    if (status != SUCCESS)
        goto exit;

exit:
    return status;
}

BOOLEAN
Rtl8226b_autoNegoEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_AN, 0, &phydata);
    if (status != SUCCESS)
        goto exit;

    *pEnable = (phydata & BIT_12) ? (TRUE) : (FALSE);

exit:
    return status;
}

BOOLEAN
Rtl8226b_autoNegoEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    // AutoNegotiationEnable
    status = MmdPhyRead(hDevice, MMD_AN, 0, &phydata);
    if (status != SUCCESS)
        goto exit;

    if (Enable)
        phydata |= BIT_12;
    else
        phydata &= (~BIT_12);

    status = MmdPhyWrite(hDevice, MMD_AN, 0, phydata);
    if (status != SUCCESS)
        goto exit;

    // RestartAutoNegotiation
    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA400, &phydata);
    if (status != SUCCESS)
        goto exit;

    phydata |= BIT_9;

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA400, phydata);
    if (status != SUCCESS)
        goto exit;

exit:
    return status;
}

BOOLEAN
Rtl8226b_autoNegoAbility_get(
    IN  HANDLE hDevice,
    OUT PHY_LINK_ABILITY *pPhyAbility
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_AN, 16, &phydata);
    if (status != SUCCESS)
        goto exit;

    // 10M
    pPhyAbility->Half_10 = (phydata & BIT_5) ? (1) : (0);
    pPhyAbility->Full_10 = (phydata & BIT_6) ? (1) : (0);

    // 100M
    pPhyAbility->Half_100 = (phydata & BIT_7) ? (1) : (0);
    pPhyAbility->Full_100 = (phydata & BIT_8) ? (1) : (0);

    pPhyAbility->FC = (phydata & BIT_10) ? (1) : (0);
    pPhyAbility->AsyFC = (phydata & BIT_11) ? (1) : (0);

    // 1G
    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA412, &phydata);
    if (status != SUCCESS)
        goto exit;

    pPhyAbility->Full_1000 = (phydata & BIT_9) ? (1) : (0);

    // 2.5G
    status = MmdPhyRead(hDevice, MMD_AN, 32, &phydata);
    if (status != SUCCESS)
        goto exit;

    pPhyAbility->adv_2_5G = (phydata & BIT_7) ? (1) : (0);

exit:
    return status;
}

BOOLEAN
Rtl8226b_autoNegoAbility_set(
    IN HANDLE hDevice,
    IN PHY_LINK_ABILITY *pPhyAbility
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_AN, 16, &phydata);
    if (status != SUCCESS)
        goto exit;

    phydata &= (~(BIT_5 | BIT_6 | BIT_7 | BIT_8 | BIT_10 | BIT_11));

    // 10M
    phydata |= (pPhyAbility->Half_10 ? (BIT_5) : (0));
    phydata |= (pPhyAbility->Full_10 ? (BIT_6) : (0));

    // 100M
    phydata |= (pPhyAbility->Half_100 ? (BIT_7) : (0));
    phydata |= (pPhyAbility->Full_100 ? (BIT_8) : (0));

    phydata |= (pPhyAbility->FC ? (BIT_10) : (0));
    phydata |= (pPhyAbility->AsyFC ? (BIT_11) : (0));

    status = MmdPhyWrite(hDevice, MMD_AN, 16, phydata);
    if (status != SUCCESS)
        goto exit;

    // 1G
    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA412, &phydata);
    if (status != SUCCESS)
        goto exit;

    phydata &= (~BIT_9);

    phydata |= (pPhyAbility->Full_1000 ? (BIT_9) : (0));

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA412, phydata);
    if (status != SUCCESS)
        goto exit;

    // 2.5G
    status = MmdPhyRead(hDevice, MMD_AN, 32, &phydata);
    if (status != SUCCESS)
        goto exit;

    phydata &= (~BIT_7);

    phydata |= (pPhyAbility->adv_2_5G ? (BIT_7) : (0));
    status = MmdPhyWrite(hDevice, MMD_AN, 32, phydata);
    if (status != SUCCESS)
        goto exit;

    if (MmdPhyRead(hDevice, MMD_AN, 0, &phydata) == SUCCESS)
    {
        if (phydata & BIT_12)   /* AN_ENABLE */
        {
            phydata |= BIT_9; /* RESTART_AN */
            MmdPhyWrite(hDevice, MMD_AN, 0, phydata);
        }
    }

exit:
    return status;
}

BOOLEAN
Rtl8226b_duplex_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA434, &phydata);
    if (status != SUCCESS)
        goto exit;

    *pEnable = (phydata & BIT_3) ? (TRUE) : (FALSE);

exit:
    return status;
}


BOOLEAN
Rtl8226b_duplex_set(                // christy add 0430
        IN HANDLE hDevice,
    IN BOOL Enable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA400, &phydata);
    if (status != SUCCESS)
        goto exit;

    if (Enable)
        phydata |= BIT_8;
    else
        phydata &= (~BIT_8);

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA400, phydata);
    if (status != SUCCESS)
        goto exit;

exit:
    return status;
}



BOOLEAN
Rtl8226b_is_link(
    IN  HANDLE hDevice,
    OUT BOOL *plinkOK
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    UINT8 i = 0;

    // must read twice
    for(i=0;i<2;i++)
    {
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xA402, &phydata);
        if (status != SUCCESS)
            goto exit;
    }

    *plinkOK = (phydata & BIT_2) ? (TRUE) : (FALSE);

exit:
    return status;
}

BOOLEAN
Rtl8226b_speed_get(
    IN  HANDLE hDevice,
    OUT UINT16 *pSpeed
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    UINT8 speed_grp = 0;
    UINT8 speed = NO_LINK;

//    int i = 0;

    BOOL linkOK = FALSE;

    status = Rtl8226b_is_link(hDevice, &linkOK);
    if (status != SUCCESS)
        goto exit;

    if (linkOK)
    {
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xA434, &phydata);
        if (status != SUCCESS)
            goto exit;

        speed_grp = (phydata & (BIT_9 | BIT_10)) >> 9;
        speed = (phydata & (BIT_4 | BIT_5)) >> 4;

        switch(speed_grp)
        {
        case 0:
        {
            switch(speed)
            {
            case 0:
                *pSpeed = LINK_SPEED_10M;
                break;
            case 1:
                *pSpeed = LINK_SPEED_100M;
                break;
            case 2:
                *pSpeed = LINK_SPEED_1G;
                break;
            case 3:
                *pSpeed = LINK_SPEED_500M;
                break;

            default:
                status = FAILURE;
                break;
            }
            break;
        }

        case 1:
        {
            switch(speed)
            {
            case 1:
                *pSpeed = LINK_SPEED_2P5G;
                break;
            case 3:
                *pSpeed = LINK_SPEED_1G;        // 2.5G lite
                break;
            default:
                status = FAILURE;
                break;
            }
            break;
        }

        default:
            status = FAILURE;
            break;
        }
    }
    else
    {
        *pSpeed = NO_LINK;
    }

exit:
    return status;
}

BOOLEAN
Rtl8226b_enable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_PMAPMD, 0, &phydata);
    if (status != SUCCESS)
        goto exit;

    if (Enable)
        phydata &= (~BIT_11);        // set as 0
    else
        phydata |= BIT_11;            // set as 1


    status = MmdPhyWrite(hDevice, MMD_PMAPMD, 0, phydata);
    if (status != SUCCESS)
        goto exit;

exit:
    return status;
}

BOOLEAN
Rtl8226b_force_speed_set(
    IN HANDLE hDevice,
    IN UINT16 Speed
    )
{
    BOOL status = FAILURE;
    UINT16 phydata0 = 0, phydata1 = 0;
    BOOL support = 0;

    status = MmdPhyRead(hDevice, MMD_PMAPMD, 0, &phydata0);
    if (status != SUCCESS)
        goto exit;

    phydata0 &= (~(BIT_6 | BIT_13));

    switch(Speed)
    {
    case 10:
        support = TRUE;
        phydata0 &= (~BIT_6);
        phydata0 &= (~BIT_13);
        break;

    case 100:
        support = TRUE;
        phydata0 &= (~BIT_6);
        phydata0 |= BIT_13;
        break;

    

    default:
        status = FAILURE;
        support = FALSE;
        break;
    }

    if (support)
    {
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xA5B4, &phydata1);
        if (status != SUCCESS)
            goto exit;

        phydata1 |= (BIT_15);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA5B4, phydata1);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyWrite(hDevice, MMD_PMAPMD, 0, phydata0);
        if (status != SUCCESS)
            goto exit;

        status = Rtl8226b_autoNegoEnable_set(hDevice, FALSE);
        if (status != SUCCESS)
            goto exit;
    }
    else
        status = FAILURE;

exit:
    return status;
}

BOOLEAN
Rtl8226b_force_speed_get(
    IN  HANDLE hDevice,
    OUT UINT16 *force_speed
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;
	BOOL AnEnable = 0;
	
	status = Rtl8226b_autoNegoEnable_get(hDevice, &AnEnable);
    if (status != SUCCESS)
        goto exit;
	

  
    if(AnEnable==0)
	{
		status = MmdPhyRead(hDevice, MMD_PMAPMD, 0, &phydata);
		if (status != SUCCESS)
			goto exit;

		if(((phydata & (BIT_6))==0 )&&((phydata & (BIT_13))==0))
			*force_speed = LINK_SPEED_10M;
				   
		else if(((phydata &(BIT_6))==0 )&&((phydata & (BIT_13))==BIT_13))
			*force_speed = LINK_SPEED_100M;		
		else 
			*force_speed = NO_LINK;
		
	}
	else
		status = FAILURE;

exit:
    return status;
}





BOOLEAN
Rtl8226b_greenEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, 0x8011);
    if (status != SUCCESS)
        goto exit;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA438, &phydata);
    if (status != SUCCESS)
        goto exit;

    *pEnable = (phydata & BIT_15) ? (TRUE) : (FALSE);

exit:
    return status;
}

BOOLEAN
Rtl8226b_greenEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, 0x8011);
    if (status != SUCCESS)
        goto exit;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA438, &phydata);
    if (status != SUCCESS)
        goto exit;

    if (Enable)
        phydata |= (BIT_15);
    else
        phydata &= (~BIT_15);

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA438, phydata);
    if (status != SUCCESS)
        goto exit;

exit:
    return status;
}




BOOLEAN
Rtl8226b_eeeEnable_get(
    IN  HANDLE hDevice,
    OUT PHY_EEE_ENABLE *pEeeEnable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_AN, 60, &phydata);
    if (status != SUCCESS)
        goto exit;

    pEeeEnable->EEE_100 = (phydata & BIT_1) ? (TRUE) : (FALSE);
    pEeeEnable->EEE_1000 = (phydata & BIT_2) ? (TRUE) : (FALSE);

    status = MmdPhyRead(hDevice, MMD_AN, 62, &phydata);
    if (status != SUCCESS)
        goto exit;

    pEeeEnable->EEE_2_5G = (phydata & BIT_0) ? (TRUE) : (FALSE);

exit:
    return status;
}

BOOLEAN
Rtl8226b_eeeEnable_set(
    IN HANDLE hDevice,
    IN PHY_EEE_ENABLE *pEeeEnable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    BOOL AnEnable = 0;

    status = Rtl8226b_autoNegoEnable_get(hDevice, &AnEnable);
    if (status != SUCCESS)
        goto exit;

    // 100M/1G EEE
    status = MmdPhyRead(hDevice, MMD_AN, 60, &phydata);
    if (status != SUCCESS)
        goto exit;

    if (pEeeEnable->EEE_100)
        phydata |= BIT_1;
    else
        phydata &= (~BIT_1);


    if (pEeeEnable->EEE_1000)
        phydata |= BIT_2;
    else
        phydata &= (~BIT_2);

    status = MmdPhyWrite(hDevice, MMD_AN, 60, phydata);
    if (status != SUCCESS)
        goto exit;

    // 2.5G EEE
    status = MmdPhyRead(hDevice, MMD_AN, 62, &phydata);
    if (status != SUCCESS)
        goto exit;

    if (pEeeEnable->EEE_2_5G)
        phydata |= BIT_0;
    else
        phydata &= (~BIT_0);

    status = MmdPhyWrite(hDevice, MMD_AN, 62, phydata);
    if (status != SUCCESS)
        goto exit;

    // RestartAutoNegotiation
    if (AnEnable)
    {
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xA400, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= BIT_9;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA400, phydata);
        if (status != SUCCESS)
            goto exit;
    }

exit:
    return status;
}





BOOLEAN
Rtl8226b_PHYmodeEEE_set(IN HANDLE hDevice,int on_off)
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;
	if(on_off){
		
		 status = MmdPhyRead(hDevice, MMD_VEND2, 0xA432, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= BIT_5;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA432, phydata);
        if (status != SUCCESS)
            goto exit;
		
		
	}
	else{
		 status = MmdPhyRead(hDevice, MMD_VEND2, 0xA432, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~BIT_5);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA432, phydata);
        if (status != SUCCESS)
            goto exit;
		
	}
	
	  status = MmdPhyRead(hDevice, MMD_VEND2, 0xA400, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= BIT_9;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA400, phydata);
        if (status != SUCCESS)
            goto exit;
	
	
		

exit:
 return status;


}

BOOLEAN
Rtl8226b_10M_PHYmodeEEEP_set(IN HANDLE hDevice,int on_off)
{
    BOOL status = FAILURE;
    UINT16 phydata = 0; 
	if(on_off){
		
		 status = MmdPhyRead(hDevice, MMD_VEND2, 0xA432, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= BIT_2;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA432, phydata);
        if (status != SUCCESS)
            goto exit;
		
		
		 status = MmdPhyRead(hDevice, MMD_VEND2, 0xC842, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= BIT_2;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xC842, phydata);
        if (status != SUCCESS)
            goto exit;
		
		
		
		
		
	}
	else{
		 status = MmdPhyRead(hDevice, MMD_VEND2, 0xA432, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~BIT_2);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA432, phydata);
        if (status != SUCCESS)
            goto exit;
		
		 status = MmdPhyRead(hDevice, MMD_VEND2, 0xC842, &phydata);
        if (status != SUCCESS)
            goto exit;

          phydata &= (~BIT_2);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xC842, phydata);
        if (status != SUCCESS)
            goto exit;
		
		
		
	}
	
	  status = MmdPhyRead(hDevice, MMD_VEND2, 0xA400, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= BIT_9;

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA400, phydata);
        if (status != SUCCESS)
            goto exit;
	
	
		

exit:
 return status;


}




BOOLEAN
Rtl8226b_crossOverMode_get(
    IN  HANDLE hDevice,
    OUT PHY_CROSSPVER_MODE *CrossOverMode
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA430, &phydata);
    if (status != SUCCESS)
        goto exit;

    if((phydata & BIT_9) >> 9)
    {
        switch((phydata & BIT_8) >> 8)
        {
        case 1:
            *CrossOverMode = PHY_CROSSPVER_MODE_MDI;
            break;
        case 0:
            *CrossOverMode = PHY_CROSSPVER_MODE_MDIX;
            break;
        default:
            status = FAILURE;
            break;
        }
    }
    else
        *CrossOverMode = PHY_CROSSPVER_MODE_AUTO;

exit:
    return status;
}

BOOLEAN
Rtl8226b_crossOverMode_set(
    IN HANDLE hDevice,
    IN PHY_CROSSPVER_MODE CrossOverMode
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA430, &phydata);
    if (status != SUCCESS)
        goto exit;

    phydata &= (~(BIT_8 | BIT_9));

    switch(CrossOverMode)
    {
    case PHY_CROSSPVER_MODE_MDI:
        phydata |= (BIT_8 | BIT_9);
        break;
    case PHY_CROSSPVER_MODE_MDIX:
        phydata |= BIT_9;
        break;
    case PHY_CROSSPVER_MODE_AUTO:
        break;
    default:
        status = FAILURE;
        goto exit;
    }

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA430, phydata);
    if (status != SUCCESS)
        goto exit;

exit:
    return status;
}

BOOLEAN
Rtl8226b_crossOverStatus_get(
    IN  HANDLE hDevice,
    OUT PHY_CROSSPVER_STATUS *pCrossOverStatus
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA434, &phydata);
    if (status != SUCCESS)
        goto exit;

    *pCrossOverStatus = (phydata & BIT_1) ? (PHY_CROSSPVER_STATUS_MDI) : (PHY_CROSSPVER_STATUS_MDIX);

exit:
    return status;
}

BOOLEAN
Rtl8226b_masterSlave_get(
    IN  HANDLE hDevice,
    OUT PHY_MASTERSLAVE_MODE *MasterSlaveMode
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_AN, 33, &phydata);
    if (status != SUCCESS)
        goto exit;

    switch((phydata >> 14) & 0x3)
    {
    case 0: // 0:Slave, 1:Master
        *MasterSlaveMode = PHY_SLAVE_MODE;
        break;
    case 1:
        *MasterSlaveMode = PHY_MASTER_MODE;
        break;
    default:
        status = FAILURE;
        break;
    }

exit:
    return status;
}

BOOLEAN
Rtl8226b_masterSlave_set(
    IN HANDLE hDevice,
    IN PHY_MASTERSLAVE_MODE MasterSlaveMode
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_AN, 32, &phydata);
    if (status != SUCCESS)
        goto exit;

    phydata &= (~(BIT_14 | BIT_15));

    switch(MasterSlaveMode)
    {
    case PHY_AUTO_MODE:
        break;
    case PHY_SLAVE_MODE:
        phydata |= BIT_15;
        break;
    case PHY_MASTER_MODE:
        phydata |= (BIT_14 | BIT_15);
        break;
    default:
        status = FAILURE;
        goto exit;
    }

    status = MmdPhyWrite(hDevice, MMD_AN, 32, phydata);
    if (status != SUCCESS)
        goto exit;

exit:
    return status;
}

BOOLEAN
Rtl8226b_loopback_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_PCS, 0x0, &phydata);
    if (status != SUCCESS)
        goto exit;

    *pEnable = (phydata & BIT_14) ? (TRUE) : (FALSE);

exit:
    return status;
}

BOOLEAN
Rtl8226b_loopback_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_PCS, 0x0, &phydata);
    if (status != SUCCESS)
        goto exit;

    if (Enable)
        phydata |= BIT_14;
    else
        phydata &= (~BIT_14);

    status = MmdPhyWrite(hDevice, MMD_PCS, 0x0, phydata);
    if (status != SUCCESS)
        goto exit;

exit:
    return status;
}

BOOLEAN
Rtl8226b_downSpeedEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA442, &phydata);
    if (status != SUCCESS)
        goto exit;

    *pEnable = (phydata & BIT_3) ? (TRUE) : (FALSE);

exit:
    return status;
}

BOOLEAN
Rtl8226b_downSpeedEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA442, &phydata);
    if (status != SUCCESS)
        goto exit;

    phydata &= (~BIT_5);

    if (Enable)
        phydata |= BIT_3;
    else
        phydata &= (~BIT_3);

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA442, phydata);
    if (status != SUCCESS)
        goto exit;

exit:
    return status;
}

BOOLEAN
Rtl8226b_gigaLiteEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA428, &phydata);
    if (status != SUCCESS)
        goto exit;

    *pEnable = (phydata & BIT_9) ? (TRUE) : (FALSE);

exit:
    return status;
}

BOOLEAN
Rtl8226b_gigaLiteEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata0 = 0, phydata1 = 0;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA442, &phydata0);
    if (status != SUCCESS)
        goto exit;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA428, &phydata1);
    if (!status)
        goto exit;

    if (Enable)
    {
        phydata0 |= (BIT_2 | BIT_9);
        phydata1 |= BIT_9;
    }
    else
    {
        phydata0 &= (~(BIT_2 | BIT_9));
        phydata1 &= (~BIT_9);
    }

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA442, phydata0);
    if (status != SUCCESS)
        goto exit;

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA428, phydata1);
    if (status != SUCCESS)
        goto exit;

exit:
    return status;
}




BOOLEAN
Rtl8226b_mdiSwapEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_VEND1, 0x6A21, &phydata);
    if (status != SUCCESS)
        goto exit;

    *pEnable = (phydata & BIT_5) ? (TRUE) : (FALSE);

exit:
    return status;
}

BOOLEAN
Rtl8226b_mdiSwapEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata0 = 0;

    status = MmdPhyRead(hDevice, MMD_VEND1, 0x6A21, &phydata0);
    if (status != SUCCESS)
        goto exit;

    if (Enable)
    {
        phydata0 |= (BIT_5);
    }
    else
    {
        phydata0 &= (~(BIT_5));
    }

    status = MmdPhyWrite(hDevice, MMD_VEND1, 0x6A21, phydata0);
    if (status != SUCCESS)
        goto exit;


exit:
    return status;
}




BOOLEAN
Rtl8226b_rtct_start(
    IN HANDLE hDevice
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;
    BOOL linkOK;
    UINT16 Speed;

    status = Rtl8226b_is_link(hDevice, &linkOK);
    if (status != SUCCESS)
        goto exit;

    if (linkOK)
    {
        status = Rtl8226b_speed_get(hDevice, &Speed);
        if (status != SUCCESS)
            goto exit;

        //RTCT is not supported when port link at 10M.
        if (Speed == 10)
        {
            osal_printf("RTCT is not supported when port link at 10M.\n");
            status = FAILURE;
            goto exit;
        }
    }
    else
    {
        // MMD 31.0xA422[15] = 0    // clear rtct_done
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xA422, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~BIT_15);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA422, phydata);
        if (status != SUCCESS)
            goto exit;

        // wait 1ms for chip reset the states
        Sleep(1);

        // MMD 31.0xA422[4] = 1    // RTCT_CH_A
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xA422, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= (BIT_4);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA422, phydata);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA422[5] = 1    // RTCT_CH_B
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xA422, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= (BIT_5);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA422, phydata);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA422[6] = 1    // RTCT_CH_C
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xA422, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= (BIT_6);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA422, phydata);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA422[7] = 1    // RTCT_CH_D
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xA422, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= (BIT_7);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA422, phydata);
        if (status != SUCCESS)
            goto exit;

        // MMD 31.0xA422[0] = 1    // RTCT_ENABLE
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xA422, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata |= (BIT_0);

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA422, phydata);
        if (status != SUCCESS)
            goto exit;
    }

exit:
    return status;
}

BOOLEAN
Rtl8226b_rtctResult_convert(
    IN UINT16 phydata,
    OUT PHY_RTCT_STATUS *pRtctStatus
    )
{
    BOOL status = SUCCESS;

    switch(phydata)
    {
    case 0x60: // Normal
        break;
    case 0x48:
        pRtctStatus->Open = TRUE;
        break;
    case 0x50:
        pRtctStatus->Short = TRUE;
        break;
    case 0x42:
        pRtctStatus->Mismatch = MIS_MATCH_OPEN;
        break;
    case 0x44:
        pRtctStatus->Mismatch = MIS_MATCH_SHORT;
        break;

    default:
        status = FAILURE;
        break;
    }

    return status;
}

BOOLEAN
Rtl8226b_rtctResult_get(
    IN HANDLE hDevice,
    OUT PHY_RTCT_RESULT *pRtctResult
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    BOOL linkOK = FALSE;

    UINT16 Speed = NO_LINK;

    BOOL rtct_done;

    status = Rtl8226b_is_link(hDevice, &linkOK);
    if (status != SUCCESS)
        goto exit;

    if (linkOK)
    {
        status = Rtl8226b_speed_get(hDevice, &Speed);
        if (status != SUCCESS)
            goto exit;

       pRtctResult->linkType = Speed;
        if (Speed == 10)
        {
            osal_printf("RTCT is not supported when port link at 10M.\n");
            status = FAILURE;
            goto exit;
        }
        switch(Speed)
        {
        case LINK_SPEED_100M:
            // rxLen = MMD 31.0xA880[7:0] * 100    // unit is meter
            // txLen = MMD 31.0xA880[7:0] * 100
            status = MmdPhyRead(hDevice, MMD_VEND2, 0xA880, &phydata);
            if (status != SUCCESS)
                goto exit;

            pRtctResult->rxLen = (phydata & 0xff) * 100;
            pRtctResult->txLen = (phydata & 0xff) * 100;
            break;

        case LINK_SPEED_1G:
            // channelALen = MMD 31.0xA880[7:0] * 100    // unit is meter
            // channelBLen = MMD 31.0xA880[7:0] * 100
            // channelCLen = MMD 31.0xA880[7:0] * 100
            // channelDLen = MMD 31.0xA880[7:0] * 100
            status = MmdPhyRead(hDevice, MMD_VEND2, 0xA880, &phydata);
            if (status != SUCCESS)
                goto exit;

            pRtctResult->channelALen = (phydata & 0xff) * 100;
            pRtctResult->channelBLen = (phydata & 0xff) * 100;
            pRtctResult->channelCLen = (phydata & 0xff) * 100;
            pRtctResult->channelDLen = (phydata & 0xff) * 100;
            break;

        case LINK_SPEED_2P5G:
            //channelALen = MMD 31.0xACBA[9:2] * 100        // cablen for XG
            status = MmdPhyRead(hDevice, MMD_VEND2, 0xACBA, &phydata);
            if (status != SUCCESS)
                goto exit;

            pRtctResult->channelALen = ((phydata & 0x3fc) >> 2 ) * 100;
            pRtctResult->channelBLen = ((phydata & 0x3fc) >> 2 ) * 100;
            pRtctResult->channelCLen = ((phydata & 0x3fc) >> 2 ) * 100;
            pRtctResult->channelDLen = ((phydata & 0x3fc) >> 2 ) * 100;
            break;

        //RTCT is not supported when port link at 10M.
        default:
            status = FAILURE;
            break;
        }
    }
    else
    {
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xA422, &phydata);
        if (status != SUCCESS)
            goto exit;

        rtct_done = (phydata & BIT_15) ? (TRUE) : (FALSE);
        if (!rtct_done)
        {
            status = FAILURE;
            goto exit;
        }

        // MMD 31.0A436[15:0] = 0x8029
        // phyData = read MMD 31.0A438[15:0]
        // channelALen = phyData * 100 / 80
        // channelALen (unit: cm)
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, 0x8029);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyRead(hDevice, MMD_VEND2, 0x0A438, &phydata);
        if (status != SUCCESS)
            goto exit;

        pRtctResult->channelALen = (phydata * 100) / 80;

        // MMD 31.0A436[15:0] = 0x802D
        // phyData = read MMD 31.0A438[15:0]
        // channelBLen = phyData * 100 / 80
        // channelBLen (unit: cm)
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, 0x802D);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyRead(hDevice, MMD_VEND2, 0x0A438, &phydata);
        if (status != SUCCESS)
            goto exit;

        pRtctResult->channelBLen = (phydata * 100) / 80;

        // MMD 31.0A436[15:0] = 0x8031
        // phyData = read MMD 31.0A438[15:0]
        // channelCLen = phyData * 100 / 80
        // channelCLen (unit: cm)
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, 0x8031);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyRead(hDevice, MMD_VEND2, 0x0A438, &phydata);
        if (status != SUCCESS)
            goto exit;

        pRtctResult->channelCLen = (phydata * 100) / 80;

        // MMD 31.0A436[15:0] = 0x8035
        // phyData = read MMD 31.0A438[15:0]
        // channelDLen = phyData * 100 / 80
        // channelDLen (unit: cm)
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, 0x8035);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyRead(hDevice, MMD_VEND2, 0x0A438, &phydata);
        if (status != SUCCESS)
            goto exit;

        pRtctResult->channelDLen = (phydata * 100) / 80;

        // channelA status
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, 0x8027);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyRead(hDevice, MMD_VEND2, 0x0A438, &phydata);
        if (status != SUCCESS)
            goto exit;

        status = Rtl8226b_rtctResult_convert(phydata, &pRtctResult->channelAStatus);
        if (status != SUCCESS)
            goto exit;

        // channelA status
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, 0x802B);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyRead(hDevice, MMD_VEND2, 0x0A438, &phydata);
        if (status != SUCCESS)
            goto exit;

        status = Rtl8226b_rtctResult_convert(phydata, &pRtctResult->channelBStatus);
        if (status != SUCCESS)
            goto exit;

        // channelC status
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, 0x802F);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyRead(hDevice, MMD_VEND2, 0x0A438, &phydata);
        if (status != SUCCESS)
            goto exit;

        status = Rtl8226b_rtctResult_convert(phydata, &pRtctResult->channelCStatus);
        if (status != SUCCESS)
            goto exit;

        // channelD status
        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, 0x8033);
        if (status != SUCCESS)
            goto exit;

        status = MmdPhyRead(hDevice, MMD_VEND2, 0x0A438, &phydata);
        if (status != SUCCESS)
            goto exit;

        status = Rtl8226b_rtctResult_convert(phydata, &pRtctResult->channelDStatus);
        if (status != SUCCESS)
            goto exit;
    }

exit:
    return status;
}




BOOLEAN
Rtl8226b_rtctdone_get(
    IN  HANDLE hDevice,
    OUT BOOL *prtct_done
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA422, &phydata);
    if (status != SUCCESS)
        goto exit;

      *prtct_done = (phydata & BIT_15) ? (TRUE) : (FALSE);

exit:
    return status;
}


   

BOOLEAN
Rtl8226b_linkDownPowerSavingEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA430, &phydata);
    if (status != SUCCESS)
        goto exit;

    *pEnable = (phydata & BIT_2) ? (TRUE) : (FALSE);

exit:
    return status;
}

BOOLEAN
Rtl8226b_linkDownPowerSavingEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA430, &phydata);
    if (status != SUCCESS)
        goto exit;

    if (Enable)
        phydata |= BIT_2;
    else
        phydata &= (~BIT_2);

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA430, phydata);
    if (status != SUCCESS)
        goto exit;

exit:
    return status;
}

BOOLEAN
Rtl8226b_2p5gLiteEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA5EA, &phydata);
    if (status != SUCCESS)
        goto exit;

    *pEnable = (phydata & BIT_0) ? (TRUE) : (FALSE);

exit:
    return status;
}

BOOLEAN
Rtl8226b_2p5gLiteEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata0 = 0, phydata1 = 0, phydata2 = 0;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA442, &phydata0);
    if (status != SUCCESS)
        goto exit;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA5FA, &phydata1);
    if (status != SUCCESS)
        goto exit;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA5EA, &phydata2);
    if (status != SUCCESS)
        goto exit;

    phydata0 &= (~BIT_2);
    phydata1 &= (~BIT_1);
    phydata2 &= (~BIT_0);

    if (Enable)
    {
        phydata0 |= BIT_2;
        phydata1 |= BIT_1;
        phydata2 |= BIT_0;
    }

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA442, phydata0);
    if (status != SUCCESS)
        goto exit;

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA5FA, phydata1);
    if (status != SUCCESS)
        goto exit;

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA5EA, phydata2);
    if (status != SUCCESS)
        goto exit;

exit:
    return status;
}

BOOLEAN
Rtl8226b_ThermalSensorEnable_get(
    IN  HANDLE hDevice,
    OUT BOOL *pEnable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, 0x817D);
    if (status != SUCCESS)
        goto exit;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA438, &phydata);
    if (status != SUCCESS)
        goto exit;

    *pEnable = (phydata & BIT_12) ? (TRUE) : (FALSE);

exit:
    return status;
}

BOOLEAN
Rtl8226b_ThermalSensorEnable_set(
    IN HANDLE hDevice,
    IN BOOL Enable,
    IN UINT16 threshold
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, 0x817D);
    if (status != SUCCESS)
        goto exit;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA438, &phydata);
    if (status != SUCCESS)
        goto exit;

    if (Enable)
        phydata |= BIT_12;
    else
        phydata &= (~BIT_12);

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA438, phydata);
    if (status != SUCCESS)
        goto exit;

    //Set the thermal thershold, default = 120 degree C
    if(threshold <= 120)
    {
      status = MmdPhyRead(hDevice, MMD_VEND2, 0xB54C, &phydata);
      if (status != SUCCESS)
          goto exit;

      phydata &= (~(BIT_15 | BIT_14 | BIT_13 | BIT_12 | BIT_11 | BIT_10 | BIT_9 | BIT_8 | BIT_7 | BIT_6));
      phydata |= ((threshold * 2) << 6);

      status = MmdPhyWrite(hDevice, MMD_VEND2, 0xB54C, phydata);
      if (status != SUCCESS)
          goto exit;
    }else
    {
     status = FAILURE;
    }


exit:
    return status;
}

BOOLEAN
Rtl8226b_ieeeTestMode_set(
    IN HANDLE hDevice,
    IN UINT16 Speed,
    IN PHY_IEEE_TEST_MODE *pIEEEtestmode
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0, phydata_w = 0, phydata2 = 0;

    switch(Speed)
    {
   case LINK_SPEED_10M:
    {
        //initial setting
        //Write MMD1, reg 0x0000, data=0x0000
        status = MmdPhyWrite(hDevice, MMD_PMAPMD, 0x0000, 0x0000);
        if (status != SUCCESS)
            goto exit;

        // wait 1ms
        Sleep(1);

        //Write MMD7, reg 0x0000, bit[12]=0x0
        status = MmdPhyRead(hDevice, MMD_AN, 0x0000, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~BIT_12);

        status = MmdPhyWrite(hDevice, MMD_AN, 0x0000, phydata);
        if (status != SUCCESS)
            goto exit;

        // wait 1ms
        Sleep(1);

        //Write MMD31, reg 0xA412, bit[15:13]=0x0
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xA412, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~(BIT_15 | BIT_14 | BIT_13));

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA412, phydata);
        if (status != SUCCESS)
             goto exit;

        // wait 1ms
        Sleep(1);

        if(pIEEEtestmode -> channel == TESTMODE_CHANNEL_A)
            Rtl8226b_crossOverMode_set(hDevice,PHY_CROSSPVER_MODE_MDI);
        else if(pIEEEtestmode -> channel == TESTMODE_CHANNEL_B)
            Rtl8226b_crossOverMode_set(hDevice,PHY_CROSSPVER_MODE_MDIX);

        if(pIEEEtestmode -> NORMAL)
        {
          //Write MMD31, reg 0xC804, data=0x0115
          status = MmdPhyWrite(hDevice, MMD_VEND2, 0xC804, 0x0115);
          if (status != SUCCESS)
             goto exit;

          //Write MMD31, reg 0xC800, data=0x5a21
          status = MmdPhyWrite(hDevice, MMD_VEND2, 0xC800, 0x5a21);
          if (status != SUCCESS)
             goto exit;
        }

        if(pIEEEtestmode -> HARMONIC)
        {
          //Write MMD31, reg 0xC804, data=0x0015
          status = MmdPhyWrite(hDevice, MMD_VEND2, 0xC804, 0x0015);
          if (status != SUCCESS)
             goto exit;

          //Write MMD31, reg 0xC800, data=0xff21
          status = MmdPhyWrite(hDevice, MMD_VEND2, 0xC800, 0xff21);
          if (status != SUCCESS)
             goto exit;
        }

        if(pIEEEtestmode -> LINKPLUSE)
        {
          //Write MMD31, reg 0xC804, data=0x0115
          status = MmdPhyWrite(hDevice, MMD_VEND2, 0xC804, 0x0115);
          if (status != SUCCESS)
             goto exit;

          //Write MMD31, reg 0xC800, data=0x5a00
          status = MmdPhyWrite(hDevice, MMD_VEND2, 0xC800, 0x5a00);
          if (status != SUCCESS)
             goto exit;
        }

        if (pIEEEtestmode->TMFINISH)
        {
            //MMD31, reg 0xA430, bit[9:8]=0x0
             Rtl8226b_crossOverMode_set(hDevice,PHY_CROSSPVER_MODE_AUTO);

            // wait 1ms
            Sleep(1);

           //Write MMD31, reg 0xC804, data=0x0115
           status = MmdPhyWrite(hDevice, MMD_VEND2, 0xC804, 0x0115);
           if (status != SUCCESS)
              goto exit;

            // wait 1ms
            Sleep(1);

           //Write MMD31, reg 0xC800, data=0x5a00
           status = MmdPhyWrite(hDevice, MMD_VEND2, 0xC800, 0x5a00);
           if (status != SUCCESS)
             goto exit;

            // wait 1ms
            Sleep(1);

            //Write MMD7, reg 0x0000, bit[12]=0x1
            status = MmdPhyRead(hDevice, MMD_AN, 0x0000, &phydata);
            if (status != SUCCESS)
                goto exit;

            phydata |= BIT_12;

            status = MmdPhyWrite(hDevice, MMD_AN, 0x0000, phydata);
            if (status != SUCCESS)
				goto exit;
            // wait 1ms
            Sleep(1);

            //Write MMD1, reg 0x0000, bit[15]=0x1, PMA reset
            status = MmdPhyRead(hDevice, MMD_PMAPMD, 0x0000, &phydata);
            if (status != SUCCESS)
                goto exit;

            phydata |= BIT_15;

            status = MmdPhyWrite(hDevice, MMD_PMAPMD, 0x0000, phydata);
            if (status != SUCCESS)
                  goto exit;
        }

        break;
    }
    case LINK_SPEED_100M:
    {
        //initial setting
        //Write MMD1, reg 0x0000, data=0x2000
        status = MmdPhyWrite(hDevice, MMD_PMAPMD, 0x0000, 0x2000);
        if (status != SUCCESS)
			goto exit;
        // wait 1ms
        Sleep(1);

        //Write MMD7, reg 0x0000, bit[12]=0x0
        status = MmdPhyRead(hDevice, MMD_AN, 0x0000, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~BIT_12);

        status = MmdPhyWrite(hDevice, MMD_AN, 0x0000, phydata);
        if (status != SUCCESS)
				goto exit;
        // wait 1ms
        Sleep(1);

        //Write MMD31, reg 0xA412, bit[15:13]=0x0
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xA412, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~(BIT_15 | BIT_14 | BIT_13));

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA412, phydata);
        if (status != SUCCESS)
             goto exit;

        // wait 1ms
        Sleep(1);

        if(pIEEEtestmode -> channel == TESTMODE_CHANNEL_A)
            Rtl8226b_crossOverMode_set(hDevice,PHY_CROSSPVER_MODE_MDI);
        else if(pIEEEtestmode -> channel == TESTMODE_CHANNEL_B)
            Rtl8226b_crossOverMode_set(hDevice,PHY_CROSSPVER_MODE_MDIX);


        if (pIEEEtestmode->TMFINISH)
        {
            //MMD31, reg 0xA430, bit[9:8]=0x0
             Rtl8226b_crossOverMode_set(hDevice,PHY_CROSSPVER_MODE_AUTO);

            // wait 1ms
            Sleep(1);

            //Write MMD7, reg 0x0000, bit[12]=0x1
            status = MmdPhyRead(hDevice, MMD_AN, 0x0000, &phydata);
            if (status != SUCCESS)
                goto exit;

            phydata |= BIT_12;

            status = MmdPhyWrite(hDevice, MMD_AN, 0x0000, phydata);
            if (status != SUCCESS)
				goto exit;
            // wait 1ms
            Sleep(1);

            //Write MMD1, reg 0x0000, bit[15]=0x1, PMA reset
            status = MmdPhyRead(hDevice, MMD_PMAPMD, 0x0000, &phydata);
            if (status != SUCCESS)
                goto exit;

            phydata |= BIT_15;

            status = MmdPhyWrite(hDevice, MMD_PMAPMD, 0x0000, phydata);
            if (status != SUCCESS)
                  goto exit;
        }

        break;
    }
    case LINK_SPEED_1G:
    {
        //initial setting
        //Write MMD1, reg 0x0000, bit[15]=0x1, PMA reset
        status = MmdPhyRead(hDevice, MMD_PMAPMD, 0x0000, &phydata);
        if (status != SUCCESS)
            goto exit;

         phydata |= BIT_15;

        status = MmdPhyWrite(hDevice, MMD_PMAPMD, 0x0000, phydata);
         if (status != SUCCESS)
				goto exit;
        // wait 1ms
         Sleep(1);

        //Write MMD7, reg 0x0000, bit[12]=0x1
        status = MmdPhyRead(hDevice, MMD_AN, 0x0000, &phydata);
        if (status != SUCCESS)
            goto exit;

         phydata |= BIT_12;

        status = MmdPhyWrite(hDevice, MMD_AN, 0x0000, phydata);
         if (status != SUCCESS)
				goto exit;
        // wait 1ms
         Sleep(1);

       //Config channel
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xBD2C, &phydata);
                if (status != SUCCESS)
                    goto exit;

                status = MmdPhyRead(hDevice, MMD_VEND2, 0xBD32, &phydata2);
                if (status != SUCCESS)
                    goto exit;

                if(pIEEEtestmode -> channel == TESTMODE_CHANNEL_NONE){
                    phydata &= (~(BIT_10));
                }
                else{
                    phydata |= BIT_10;

                    if(pIEEEtestmode -> channel == TESTMODE_CHANNEL_A)
                        phydata_w = 1 << 8;
                    else if(pIEEEtestmode -> channel == TESTMODE_CHANNEL_B)
                        phydata_w = 2 << 8;
                    else if(pIEEEtestmode -> channel == TESTMODE_CHANNEL_C)
                        phydata_w = 4 << 8;
                    else if(pIEEEtestmode -> channel == TESTMODE_CHANNEL_D)
                        phydata_w = 8 << 8;

                    phydata2 &= (~(BIT_11 | BIT_10 | BIT_9 | BIT_8));
                    phydata2 |= phydata_w;
                }

                status = MmdPhyWrite(hDevice, MMD_VEND2, 0xBD32, phydata2);
                if (status != SUCCESS)
                    goto exit;

                status = MmdPhyWrite(hDevice, MMD_VEND2, 0xBD2C, phydata);
                if (status != SUCCESS)
                    goto exit;

        //1G Test Mode
        if (pIEEEtestmode->TM1)
        {
            status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA412, 0x2000);
            if (status != SUCCESS)
                goto exit;
        }

        if (pIEEEtestmode->TM2)
        {
            status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA412, 0x4000);
            if (status != SUCCESS)
                goto exit;
        }

        if (pIEEEtestmode->TM4)
        {
            status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA412, 0x8000);
            if (status != SUCCESS)
                goto exit;
        }

        if (pIEEEtestmode->TMFINISH)
        {
            status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA412, 0x0000);
            if (status != SUCCESS)
                goto exit;

            status = MmdPhyRead(hDevice, MMD_VEND2, 0xBD2C, &phydata);
            if (status != SUCCESS)
                goto exit;

            phydata &= (~(BIT_10));

            status = MmdPhyWrite(hDevice, MMD_VEND2, 0xBD2C, phydata);
            if (status != SUCCESS)
                goto exit;
        }

        break;
    }

    case LINK_SPEED_2P5G:
    {
        //initial setting
        //Write MMD7, reg 0x0000, bit[12]=0x1
        status = MmdPhyRead(hDevice, MMD_AN, 0x0000, &phydata);
        if (status != SUCCESS)
            goto exit;

         phydata |= BIT_12;

         status = MmdPhyWrite(hDevice, MMD_AN, 0x0000, phydata);
         if (status != SUCCESS)
				goto exit;
          // wait 1ms
          Sleep(1);

        //Write MMD31, reg 0xA412, bit[15:13]=0x0
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xA412, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~(BIT_15 | BIT_14 | BIT_13));

        status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA412, phydata);
        if (status != SUCCESS)
				goto exit;
          // wait 1ms
          Sleep(1);

         //Write MMD1, reg 0x0000, data 0x2058
          status = MmdPhyWrite(hDevice, MMD_PMAPMD, 0x0000, 0x2058);
          if (status != SUCCESS)
             goto exit;

          // wait 1ms
          Sleep(1);

        //Config channel
        status = MmdPhyRead(hDevice, MMD_VEND2, 0xBD2C, &phydata);
                if (status != SUCCESS)
                    goto exit;

                status = MmdPhyRead(hDevice, MMD_VEND2, 0xBD32, &phydata2);
                if (status != SUCCESS)
                    goto exit;

                if(pIEEEtestmode -> channel == TESTMODE_CHANNEL_NONE){
                    phydata &= (~(BIT_8));
                }
                else{
                    phydata |= BIT_8;

                    if(pIEEEtestmode -> channel == TESTMODE_CHANNEL_A)
                        phydata_w = 1;
                    else if(pIEEEtestmode -> channel == TESTMODE_CHANNEL_B)
                        phydata_w = 2;
                    else if(pIEEEtestmode -> channel == TESTMODE_CHANNEL_C)
                        phydata_w = 4;
                    else if(pIEEEtestmode -> channel == TESTMODE_CHANNEL_D)
                        phydata_w = 8;

                    phydata2 &= (~(BIT_3 | BIT_2 | BIT_1 | BIT_0));
                    phydata2 |= phydata_w;
                }

                status = MmdPhyWrite(hDevice, MMD_VEND2, 0xBD32, phydata2);
                if (status != SUCCESS)
                    goto exit;

                status = MmdPhyWrite(hDevice, MMD_VEND2, 0xBD2C, phydata);
                if (status != SUCCESS)
                    goto exit;


        // 2.5G Test Mode
        if (pIEEEtestmode->TM1)
        {
            status = MmdPhyWrite(hDevice, MMD_PMAPMD, 132, 0x2400);
            if (status != SUCCESS)
                goto exit;
        }

        if (pIEEEtestmode->TM2)
        {
            status = MmdPhyWrite(hDevice, MMD_PMAPMD, 132, 0x4400);
            if (status != SUCCESS)
                goto exit;
        }

        if (pIEEEtestmode->TM3)
        {
            status = MmdPhyWrite(hDevice, MMD_PMAPMD, 132, 0x6400);
            if (status != SUCCESS)
                goto exit;
        }

        if (pIEEEtestmode->TM4)
        {
            if (pIEEEtestmode->TONE1)
            {
                status = MmdPhyWrite(hDevice, MMD_PMAPMD, 132, 0x8400);
                if (status != SUCCESS)
                    goto exit;
            }

            if (pIEEEtestmode->TONE2)
            {
                status = MmdPhyWrite(hDevice, MMD_PMAPMD, 132, 0x8800);
                if (status != SUCCESS)
                    goto exit;
            }

            if (pIEEEtestmode->TONE3)
            {
                status = MmdPhyWrite(hDevice, MMD_PMAPMD, 132, 0x9000);
                if (status != SUCCESS)
                    goto exit;
            }

            if (pIEEEtestmode->TONE4)
            {
                status = MmdPhyWrite(hDevice, MMD_PMAPMD, 132, 0x9400);
                if (status != SUCCESS)
                    goto exit;
            }

            if (pIEEEtestmode->TONE5)
            {
                status = MmdPhyWrite(hDevice, MMD_PMAPMD, 132, 0x9800);
                if (status != SUCCESS)
                    goto exit;
            }
        }

        if (pIEEEtestmode->TM5)
        {
            status = MmdPhyWrite(hDevice, MMD_PMAPMD, 132, 0xA400);
            if (status != SUCCESS)
                goto exit;
        }

        if (pIEEEtestmode->TM6)
        {
            status = MmdPhyWrite(hDevice, MMD_PMAPMD, 132, 0xC400);
            if (status != SUCCESS)
                goto exit;
        }

        if (pIEEEtestmode->TMFINISH)
        {
            status = MmdPhyRead(hDevice, MMD_VEND2, 0xBD2C, &phydata);
            if (status != SUCCESS)
                goto exit;

            phydata &= (~(BIT_8));

            status = MmdPhyWrite(hDevice, MMD_VEND2, 0xBD2C, phydata);
            if (status != SUCCESS)
                goto exit;

            //re-nway and set phy_rst)
            status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA400, 0x9200);
            if (status != SUCCESS)
                goto exit;
        }

        break;
    }
    default:
        break;
    }




exit:
    return status;
}

BOOLEAN
Rtl8226b_serdes_rst(
    IN HANDLE hDevice
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_VEND1, 0x7581, &phydata);
    if (status != SUCCESS)
        goto exit;

    phydata &= (~BIT_4);

    status = MmdPhyWrite(hDevice, MMD_VEND1, 0x7581, phydata);
    if (status != SUCCESS)
        goto exit;

    // wait 1ms
    Sleep(1);

    status = MmdPhyRead(hDevice, MMD_VEND1, 0x7581, &phydata);
    if (status != SUCCESS)
        goto exit;

    phydata |= BIT_4;

    status = MmdPhyWrite(hDevice, MMD_VEND1, 0x7581, phydata);
    if (status != SUCCESS)
        goto exit;

    // wait 1ms
    Sleep(1);
exit:
    return status;
}

BOOLEAN
Rtl8226b_serdes_link_get(
    IN  HANDLE hDevice,
    OUT BOOL *perdesLink,
    OUT PHY_SERDES_MODE *SerdesMode
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

        // serdes link
    status = MmdPhyRead(hDevice, MMD_VEND1, 0x758D, &phydata);
    if (status != SUCCESS)
        goto exit;

    *perdesLink = (phydata & BIT_1) ? (TRUE) : (FALSE);

	status = MmdPhyRead(hDevice, MMD_VEND1, 0x758B, &phydata);
    if (status != SUCCESS)
        goto exit;

    phydata &= BIT_0;

	if (phydata == 1){

        //serdes mode
        status = MmdPhyRead(hDevice, MMD_VEND1, 0x7580, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (BIT_4 | BIT_3 | BIT_2 | BIT_1 | BIT_0);

        if(phydata == 2)
                *SerdesMode = PHY_SERDES_MODE_SGMII;
            else if(phydata == 13)
                *SerdesMode = PHY_SERDES_MODE_USXGMII;
            else if(phydata == 18)
                *SerdesMode = PHY_SERDES_MODE_HiSGMII;
            else if(phydata == 22)
                *SerdesMode = PHY_SERDES_MODE_2500BASEX;
            else if(phydata == 31)
                *SerdesMode = PHY_SERDES_MODE_NO_SDS;
            else
                *SerdesMode = PHY_SERDES_MODE_OTHER;
	}
	else{
		*SerdesMode = PHY_SERDES_MODE_NO_SDS;
	}

exit:
    return status;
}

BOOLEAN
Rtl8226b_serdes_option_set(
    IN HANDLE hDevice,
    IN UINT8 functioninput
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    if ((functioninput >= 0) && (functioninput <= 5))
    {
        status = MmdPhyRead(hDevice, MMD_VEND1, 0x697A, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (~(BIT_0 | BIT_1 | BIT_2 | BIT_3 | BIT_4 | BIT_5));
        phydata |= functioninput;

        status = MmdPhyWrite(hDevice, MMD_VEND1, 0x697A, phydata);
        if (status != SUCCESS)
            goto exit;
    }

exit:
    return status;
}


BOOLEAN
Rtl8226b_serdes_option_get(
    IN HANDLE hDevice,
    OUT PHY_SERDES_OPTION *SerdesOption
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

        //serdes option
        status = MmdPhyRead(hDevice, MMD_VEND1, 0x697A, &phydata);
        if (status != SUCCESS)
            goto exit;

        phydata &= (BIT_5 |BIT_4 | BIT_3 | BIT_2 | BIT_1 | BIT_0);

        if(phydata == 0)
                *SerdesOption = PHY_SERDES_OPTION_2500BASEX_SGMII;
            else if(phydata == 1)
                *SerdesOption = PHY_SERDES_OPTION_HiSGMII_SGMII;
            else if(phydata == 2)
                *SerdesOption = PHY_SERDES_OPTION_2500BASEX;
            else if(phydata == 3)
                *SerdesOption = PHY_SERDES_OPTION_HiSGMII;
            else
                *SerdesOption = PHY_SERDES_OPTION_OTHER;
exit:
    return status;
}

BOOLEAN
Rtl8226b_serdes_polarity_swap(
    IN HANDLE hDevice,
    IN PHY_SERDES_POLARITY_SWAP *ppolarityswap
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyWrite(hDevice, MMD_VEND1, 0x7588, 0x0);
    if (status != SUCCESS)
        goto exit;

    if (ppolarityswap->TX_SWAP && ppolarityswap->RX_SWAP)
        phydata = 0x1703;
    else if (ppolarityswap->TX_SWAP)
        phydata = 0x1503;
    else if (ppolarityswap->RX_SWAP)
        phydata = 0x1603;
    else
        phydata = 0x1403;

    status = MmdPhyWrite(hDevice, MMD_VEND1, 0x7589, phydata);
    if (status != SUCCESS)
        goto exit;

    status = MmdPhyWrite(hDevice, MMD_VEND1, 0x7587, 0x0003);
    if (status != SUCCESS)
        goto exit;

exit:
    return status;
}

BOOLEAN
Rtl8226b_serdes_autoNego_set(
    IN HANDLE hDevice,
    IN BOOL Enable
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyWrite(hDevice, MMD_VEND1, 0x7588, 0x0002);
    if (status != SUCCESS)
        goto exit;

    if (Enable)
        phydata = 0x70D0;
    else
        phydata = 0x71D0;

    status = MmdPhyWrite(hDevice, MMD_VEND1, 0x7589, phydata);
    if (status != SUCCESS)
        goto exit;

    status = MmdPhyWrite(hDevice, MMD_VEND1, 0x7587, 0x0003);
    if (status != SUCCESS)
        goto exit;

exit:
    return status;
}

BOOLEAN
Rtl8226b_wol_set(
    IN HANDLE hDevice,
    IN PHY_WOL_EVENT *pwolevent
    )
{
    BOOL status = FAILURE;
    BOOL linkOK;
    UINT16 speed;
    UINT16 phydata = 0, phydata2=0;

    status = Rtl8226b_is_link(hDevice, &linkOK);
    if (status != SUCCESS)
        goto exit;

    if (linkOK)
    {
        status = Rtl8226b_speed_get(hDevice, &speed);
        if (status != SUCCESS)
            goto exit;

        //WOL is not supported when port link at 2.5G.
        if (speed == LINK_SPEED_2P5G)
        {
            osal_printf("WOL/PME is not supported when port link at 2.5G.\n");
            status = FAILURE;
            goto exit;
        }
     }

    //Set INTB/PMEB pinas PMEB(optional) MMD31, reg 0xD05C, bit0=1
    status = MmdPhyRead(hDevice, MMD_VEND2, 0xD05C, &phydata);
    if (status != SUCCESS)
        goto exit;

     phydata |= BIT_0;

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD05C, phydata);
     if (status != SUCCESS)
        goto exit;

    // wait 1ms
     Sleep(1);

    //Set MAC address  MMD31, reg 0xD8C0,0xD8C2,0xD8C4
     status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD8C0, pwolevent->macaddress.ADDR15_0);
     if (status != SUCCESS)
        goto exit;
    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD8C2, pwolevent->macaddress.ADDR31_16);
     if (status != SUCCESS)
        goto exit;
    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD8C4, pwolevent->macaddress.ADDR47_32);
     if (status != SUCCESS)
        goto exit;

    //Set Max packet length MMD31, reg 0xD8A2
    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD8A2,  pwolevent->MAXPKTLENGTH);
     if (status != SUCCESS)
        goto exit;

    //WOL event select and enable  MMD31, reg 0xD8A0
     if(pwolevent->LINKCHG)
      phydata2 |= BIT_13;
     if(pwolevent->MAGIC)
      phydata2 |= BIT_12;
     if(pwolevent->ARBITRARY)
      phydata2 |= BIT_11;
     if(pwolevent->UNICAST)
      phydata2 |= BIT_10;
     if(pwolevent->MULTICAST)
      phydata2 |= BIT_9;
     if(pwolevent->BROADCAST)
      phydata2 |= BIT_8;
     if(pwolevent->FRAME7)
      phydata2 |= BIT_7;
     if(pwolevent->FRAME6)
      phydata2 |= BIT_6;
     if(pwolevent->FRAME5)
      phydata2 |= BIT_5;
     if(pwolevent->FRAME4)
      phydata2 |= BIT_4;
     if(pwolevent->FRAME3)
      phydata2 |= BIT_3;
     if(pwolevent->FRAME2)
      phydata2 |= BIT_2;
     if(pwolevent->FRAME1)
      phydata2 |= BIT_1;
     if(pwolevent->FRAME0)
      phydata2 |= BIT_0;

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD8A0,  phydata2);
     if (status != SUCCESS)
        goto exit;

    if(pwolevent->MULTICAST)
    {
     //Set Multicast register  MMD31, reg 0xD8C6,0xD8C8,0xD8CA,0xD8CC
     status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD8C6, pwolevent->multicast.REG15_0);
     if (status != SUCCESS)
        goto exit;
    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD8C8, pwolevent->multicast.REG31_16);
     if (status != SUCCESS)
        goto exit;
    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD8CA, pwolevent->multicast.REG47_32);
     if (status != SUCCESS)
        goto exit;
    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD8CC, pwolevent->multicast.REG63_48);
     if (status != SUCCESS)
        goto exit;
    }

    //Set Wakeup Frame #0
    if(pwolevent->FRAME0)
    {
      //Set Mask
      status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD800, pwolevent->wakeframe0.MASK15_0);
       if (status != SUCCESS)
          goto exit;
      status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD802, pwolevent->wakeframe0.MASK31_16);
       if (status != SUCCESS)
          goto exit;
      status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD804, pwolevent->wakeframe0.MASK47_32);
       if (status != SUCCESS)
          goto exit;
      status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD806, pwolevent->wakeframe0.MASK63_48);
       if (status != SUCCESS)
          goto exit;
      status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD808, pwolevent->wakeframe0.MASK79_64);
       if (status != SUCCESS)
          goto exit;
      status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD80A, pwolevent->wakeframe0.MASK95_80);
       if (status != SUCCESS)
          goto exit;
      status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD80C, pwolevent->wakeframe0.MASK111_96);
       if (status != SUCCESS)
          goto exit;
      status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD80E, pwolevent->wakeframe0.MASK127_112);
       if (status != SUCCESS)
          goto exit;

      //Set CRC
      status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD880, pwolevent->wakeframe0.CRC);
       if (status != SUCCESS)
          goto exit;
      }

exit:
    return status;
}

BOOLEAN
Rtl8226b_wol_exit(
    IN HANDLE hDevice
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;
    //Disable all WOL events
    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD8A0,  0x0000);
     if (status != SUCCESS)
        goto exit;

    //WOL reset
    status = MmdPhyRead(hDevice, MMD_VEND2, 0xD8A2, &phydata);
    if (status != SUCCESS)
        goto exit;

     phydata &= (~BIT_15);

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xD8A2, phydata);
     if (status != SUCCESS)
        goto exit;

exit:
    return status;
}
BOOLEAN
Rtl8226b_ThermalSensor_get(
    IN HANDLE hDevice,
    OUT PHY_THERMAL_RESULT *pTsResult
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA436, 0x817D);
    if (status != SUCCESS)
        goto exit;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xA438, &phydata);
    if (status != SUCCESS)
        goto exit;

    pTsResult->Enable = (phydata & BIT_12) ? (TRUE) : (FALSE);


    status = MmdPhyRead(hDevice, MMD_VEND2, 0xBD84, &phydata);
    if (status != SUCCESS)
        goto exit;

    pTsResult->Temperature = (phydata & 0x03ff)/2;

    status = MmdPhyRead(hDevice, MMD_VEND2, 0xB54C, &phydata);
    if (status != SUCCESS)
        goto exit;

     pTsResult->Temperature_threshold = ((phydata & 0xffc0)>>6)/2;

exit:
    return status;
}

BOOLEAN
Rtl8226b_ThermalSensor_resume_2P5G(
    IN HANDLE hDevice
    )
{
    BOOL status = FAILURE;
    UINT16 phydata = 0;

    status = MmdPhyRead(hDevice, MMD_VEND2,0xA662, &phydata);
    if (status != SUCCESS)
        goto exit;

        phydata &= (~(BIT_1 | BIT_0));

    status = MmdPhyWrite(hDevice, MMD_VEND2, 0xA662, phydata);
    if (status != SUCCESS)
        goto exit;

    //The setting will take effect after a link change event
    Rtl8226b_enable_set(hDevice, FALSE);
    Sleep (1);
    Rtl8226b_enable_set(hDevice, TRUE);


exit:
    return status;
}
