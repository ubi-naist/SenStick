/*
 * Copyright (c) 2016 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is confidential property of Nordic Semiconductor. The use,
 * copying, transfer or disclosure of such information is prohibited except by express written
 * agreement with Nordic Semiconductor.
 *
 */
#include "ssi_pal_types.h"
#include "ssi_pal_mem.h"
#include "sns_silib.h"
#include "integration_test_plat_defs.h"
#include "integration_test_ssi_data.h"
#include "integration_test_ssi_defs.h"
#ifdef DX_LINUX_PLATFORM /*for linux platform only !!*/
#include <pthread.h>
#endif
#include "crys_hkdf.h"


/*HKDF test data vectors*/
extern hkdfDataStuct hkdfVectors[];

/*RNG Global variables*/
extern CRYS_RND_Context_t*   rndContext_ptr;
extern CRYS_RND_WorkBuff_t*  rndWorkBuff_ptr;



int hkdf_tests(void)
{
	uint32_t                        Error = 0;
	int                             test_index;
 	int32_t tot_tests=0, tot_errs=0;


	/*Run all HKDF tests*/
	for (test_index = 0; test_index < HKDF_TESTS_NUMBER; test_index++)
	{
		hkdfDataStuct hkdfTestVector;
		CRYS_HKDF_HASH_OpMode_t hkdfHashMode;
		uint8_t   OKMDataBuff[TST_HKDF_MAX_OKM_SIZE];

		SaSi_PalMemCopy(&hkdfTestVector, &hkdfVectors[test_index], sizeof(hkdfTestVector));

		hkdfHashMode  = hkdfTestVector.hkdfHashMode;

		Error = CRYS_HKDF_KeyDerivFunc(
					hkdfHashMode,
					hkdfTestVector.hkdf_Salt,
					(size_t)hkdfTestVector.hkdf_SaltSize,
					hkdfTestVector.hkdf_IKM,
					hkdfTestVector.hkdf_IKMSize,
					hkdfTestVector.hkdf_Info,
					hkdfTestVector.hkdf_InfoSize,
					OKMDataBuff,
					hkdfTestVector.hkdf_LSize,
					SASI_FALSE
					);

		if (Error != CRYS_OK) {
			INTEG_TEST_PRINT("Test %d: %s: CRYS_HKDF_KeyDerivFunc() returned error 0x%08x - FAILED\n",
				 (int)test_index, (uint32_t)hkdfTestVector.hkdf_Name, Error);
			goto end_loop;
		}

		/* compare the result and print result*/
		Error = SaSi_PalMemCmp(&OKMDataBuff[0], &hkdfTestVector.hkdf_ExpectOKM[0], hkdfTestVector.hkdf_LSize);

		if( Error != 0 ) {
			INTEG_TEST_PRINT("Test %d: %s: OKM content is not as expected - FAILED\n",
				 (int)test_index, (uint32_t)hkdfTestVector.hkdf_Name);

		#ifdef PRINT_HKDF_RES
			/* print the actual and expected result */
			INTEG_TEST_PRINT(" Data : actual : expected :  match :\n");
			for(j = 0 ; (j < hkdfTestVector.hkdf_LSize); j++)
			{
				INTEG_TEST_PRINT("  %02d  :  %02X   :    %02X    :   %d   :\n",
					(int)j, (unsigned int)OKMDataBuff[j], (unsigned int)hkdfTestVector.hkdf_ExpectOKM[j],
					(int)(OKMDataBuff[j]==hkdfTestVector.hkdf_ExpectOKM[j]));
			}
			INTEG_TEST_PRINT("\n");
		#endif
			goto end_loop;

		} else {
			INTEG_TEST_PRINT("Test %d: %s: - PASSED\n",
				 (int)test_index, (uint32_t)hkdfTestVector.hkdf_Name);
		}

end_loop:
		tot_errs += Error?1:0; ++tot_tests;

	}

	if( tot_errs != 0 ) {
		INTEG_TEST_PRINT(" HKDF TotalTests= %d tot_errs= %d  FAILED\n", (int)test_index, (int)tot_errs);
		goto end;
	} else {
		INTEG_TEST_PRINT(" HKDF TotalTests= %d PASSED\n", (int)test_index);
	}

	if (Error != SA_SILIB_RET_OK){
		INTEG_TEST_PRINT(" SaSi_PalMemCmp failed \n");
		goto end;
	}

	INTEG_TEST_PRINT("\nPassed \n ==========================\n");

end:

	return Error;
}

#ifdef DX_LINUX_PLATFORM /*for linux platform only  -> we need to use contiguous memory for stack !!*/
/*hkdf_wrap_tests creates thread with defined stack address to and calls to hkdf test */
void* hkdf_thread(void)
{
   uint32_t* threadReturnValue = malloc(sizeof(uint32_t));

   *threadReturnValue =hkdf_tests();
    if (*threadReturnValue != SA_SILIB_RET_OK) {
        INTEG_TEST_PRINT("Failure in hkdf_tests,ret = 0x%x\n", *threadReturnValue);
        goto exit;
    }

exit:
	pthread_exit(threadReturnValue);
}

int hkdf_wrap_tests(void){
	uint32_t rc = 0;

	pthread_t threadId;
	pthread_attr_t threadAttr;
	int threadRc;
	void *threadRet;
	int num = 6;

	threadRc = pthread_attr_init(&threadAttr);
	if (threadRc != 0) {
		INTEG_TEST_PRINT("pthread_attr_init failed\n");
		return -1;
	}

	threadRc = pthread_attr_setstack(&threadAttr, g_test_stack_base_addr, PTHREAD_STACK_SIZE);
	if (threadRc != 0) {
		INTEG_TEST_PRINT("pthread_attr_setstack failed\n");
		return -1;
	}

	/* Create independent thread which run with  */
	threadRc = pthread_create( &threadId, &threadAttr, (void *)hkdf_thread, &num);
	if (threadRc != 0) {
		INTEG_TEST_PRINT( "pthread_create failed\n");
		return -1;
	}

	/* Wait till thread is complete before main continues */
	threadRc = pthread_join( threadId, &threadRet);
	if (threadRc != 0) {
		INTEG_TEST_PRINT( "pthread_join failed\n");
		return -1;
	}

	rc =*((uint32_t *)*&threadRet);

    free(threadRet);
	threadRc = pthread_attr_destroy(&threadAttr);
	if (threadRc != 0) {
		INTEG_TEST_PRINT("pthread_attr_destroy failed\n");
	}

	return rc;
}
#endif

int main(void)
{
	int ret = 0;

	/*Perform memory mapping*/
	ret = integration_tests_setup();
	if (ret != 0)
	{
		INTEG_TEST_PRINT("integration_tests_setup failed\n");
		return ret;
	}

        /*Init SaSi library*/
	ret = SaSi_LibInit(rndContext_ptr, rndWorkBuff_ptr);
	if (ret != SA_SILIB_RET_OK) {
	    INTEG_TEST_PRINT("Failed SaSi_SiLibInit - ret = 0x%x\n", ret);
	    goto exit_1;
	}

        /*Call hkdf test*/
#ifdef DX_LINUX_PLATFORM
	ret = hkdf_wrap_tests(); /*Call wrap function to create thread and to define stack's address to use contiguous memory*/
#else
	ret = hkdf_tests();
#endif
	if (ret != SA_SILIB_RET_OK) {
	    INTEG_TEST_PRINT("Failure in hkdf_tests,ret = 0x%x\n", ret);
	    goto exit_0;
	}

	INTEG_TEST_PRINT("All tests passed \n");

exit_0:
	/*Finish SaSi library*/
	SaSi_LibFini(rndContext_ptr);

exit_1:
	integration_tests_clear();

	return ret;
}


