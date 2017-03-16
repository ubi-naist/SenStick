//-----------------------------------------------------------------------------------
//    The confidential and proprietary information contained in this file may 
//    only be used by a person authorised under and to the extent permitted
//    by a subsisting licensing agreement from ARM Limited or its affiliates 
//    or between you and a party authorised by ARM
//
//           (C) COPYRIGHT [2016] ARM Limited or its affiliates
//               ALL RIGHT RESERVED
//
//    This entire notice must be reproduced on all copies of this file
//    and copies of this files may only be made by a person if such person is
//    permitted to do so under the terms of a subsisting license agreement
//    from ARM Limited or its affiliates or between you and a party authorised by ARM
//-----------------------------------------------------------------------------------

#ifndef _SSI_GENERAL_DEFS_H
#define _SSI_GENERAL_DEFS_H

/*!
@file
@brief This file contains general definitions.
*/

#ifdef __cplusplus
extern "C"
{
#endif

/* general definitions */
/*-------------------------*/
#define SASI_AES_KDR_MAX_SIZE_BYTES	16
#define SASI_AES_KDR_MAX_SIZE_WORDS	(SASI_AES_KDR_MAX_SIZE_BYTES/sizeof(uint32_t))


/* Life cycle state definitions */
#define SASI_LCS_CHIP_MANUFACTURE_LCS		0x0 /*!< Life cycle CM value. */
#define SASI_LCS_SECURE_LCS			0x2 /*!< Life cycle secure value. */

#ifdef __cplusplus
}
#endif

#endif



