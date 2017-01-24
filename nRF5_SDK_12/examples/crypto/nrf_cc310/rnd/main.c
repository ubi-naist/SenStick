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
#include "crys_rnd_error.h"
#include "integration_test_plat_defs.h"
#include "integration_test_ssi_data.h"
#include "integration_test_ssi_defs.h"

/*RND test data vector*/
extern rndDataStuct rndVectors[];

/*RND global variables*/
extern CRYS_RND_Context_t*   rndContext_ptr;
extern CRYS_RND_WorkBuff_t*  rndWorkBuff_ptr;


/*rnd_test - performs basic integration test for RND module*/
int rnd_tests(void)
{
	uint32_t                        ret = 0/*,MaxVectorSize = 0*/;
	int                             test_index = 0;

	/*Set additional input for rng seed*/
	ret = CRYS_RND_AddAdditionalInput(rndContext_ptr,
		rndVectors[test_index].rndTest_AddInputData,
		rndVectors[test_index].rndTest_AddInputSize);


	if (ret != SA_SILIB_RET_OK){
		INTEG_TEST_PRINT("\n CRYS_RND_AddAdditionalInput failed with 0x%x \n",ret);
		return ret;
	}
	INTEG_TEST_PRINT("\n CRYS_RND_AddAdditionalInput passed\n");

	/*Reseed rnd using added input (new seed will be generated using additional input)*/
	ret = CRYS_RND_Reseeding (rndContext_ptr,
		rndWorkBuff_ptr);
	if (ret != SA_SILIB_RET_OK){
		INTEG_TEST_PRINT("\n CRYS_RND_Reseeding failed with 0x%x \n",ret);
		return ret;
	}
	INTEG_TEST_PRINT("\n CRYS_RND_Reseeding passed\n");

	/*Generate random vector 1*/
	ret = CRYS_RND_GenerateVector(&rndContext_ptr->rndState,
		rndVectors[test_index].rndTest_RandomVectorSize,
		rndVectors[test_index].rndTest_RandomVectorData1);
	if (ret != SA_SILIB_RET_OK){
		INTEG_TEST_PRINT("\n CRYS_RND_GenerateVector for vector 1 failed with 0x%x \n",ret);
		return ret;
	}

	INTEG_TEST_PRINT("\n CRYS_RND_GenerateVector for first vector passed\n");
	/*Generate rnadom vector 2*/
	ret = CRYS_RND_GenerateVector(&rndContext_ptr->rndState,
		rndVectors[test_index].rndTest_RandomVectorSize,
		rndVectors[test_index].rndTest_RandomVectorData2);

	if (ret != SA_SILIB_RET_OK){
		INTEG_TEST_PRINT("\n CRYS_RND_GenerateVector for vector 2 failed with 0x%x \n",ret);
		return ret;
	}

	INTEG_TEST_PRINT("\n CRYS_RND_GenerateVector for second vector passed\n");

	/*Compare two generated vectors - should not be the same value*/
	ret = SaSi_PalMemCmp(rndVectors[test_index].rndTest_RandomVectorData1,
		rndVectors[test_index].rndTest_RandomVectorData2,
		rndVectors[test_index].rndTest_RandomVectorSize);

	if (ret == SA_SILIB_RET_OK){
		INTEG_TEST_PRINT("\n Two random vectors should not are the same \n");
		return ret;
	}

	INTEG_TEST_PRINT("\n Two generated vectors are different as expected\n");

	/*Generate random vector in range when max value is NULL*/
	ret = CRYS_RND_GenerateVectorInRange(rndContext_ptr,
		rndVectors[test_index].rndTest_RandomVectorInRangeSize1,
		NULL,
		rndVectors[test_index].rndTest_RandomVectorInRangeData);


	if (ret != SA_SILIB_RET_OK){
		INTEG_TEST_PRINT("\n CRYS_RND_GenerateVectorInRange for vector with fixed size failed with 0x%x \n",ret);
		return ret;
	}

	INTEG_TEST_PRINT("\n CRYS_RND_GenerateVectorInRange 1 passed\n");

	/*Generate random vector in range with max vector */
	ret = CRYS_RND_GenerateVectorInRange(rndContext_ptr,
		rndVectors[test_index].rndTest_RandomVectorInRangeSize2,
		rndVectors[test_index].rndTest_MaxVectorInRange2,
		rndVectors[test_index].rndTest_RandomVectorInRangeData2);

	if (ret != SA_SILIB_RET_OK){
		INTEG_TEST_PRINT("\n CRYS_RND_GenerateVectorInRange failed with 0x%x \n",ret);
		return ret;
	}
	INTEG_TEST_PRINT("\n CRYS_RND_GenerateVectorInRange 2 passed\n");


	/*Perform UnInstantiation*/
	ret = CRYS_RND_UnInstantiation(rndContext_ptr);

	if (ret != SA_SILIB_RET_OK){
		INTEG_TEST_PRINT("\n CRYS_RND_UnInstantiation failed with 0x%x \n",ret);
		return ret;
	}

	INTEG_TEST_PRINT("\n CRYS_RND_UnInstantiation passed\n");

	/*Try to create random vector without instantiation - should fail*/
	ret = CRYS_RND_GenerateVector(&rndContext_ptr->rndState,
		rndVectors[test_index].rndTest_RandomVectorSize,
		rndVectors[test_index].rndTest_RandomVectorData2);

	if (ret != CRYS_RND_STATE_VALIDATION_TAG_ERROR){
		INTEG_TEST_PRINT("\n CRYS_RND_GenerateVector returned wrong error 0x%x,\n CRYS_RND_STATE_VALIDATION_TAG_ERROR should be returned\n",ret);
		return ret;
	}

	INTEG_TEST_PRINT("\n CRYS_RND_GenerateVector failed as expected\n");

	/*Set additional input for RND seed*/
	ret = CRYS_RND_AddAdditionalInput(rndContext_ptr,
		rndVectors[test_index].rndTest_AddInputData,
		rndVectors[test_index].rndTest_AddInputSize);

	if (ret != SA_SILIB_RET_OK){
		INTEG_TEST_PRINT("\n CRYS_RND_AddAdditionalInput failed with 0x%x \n",ret);
		return ret;
	}

	INTEG_TEST_PRINT("\n CRYS_RND_AddAdditionalInput passed\n");

	/*Perform instantiation for new seed*/
	ret = CRYS_RND_Instantiation(rndContext_ptr,
		rndWorkBuff_ptr);

	if (ret != SA_SILIB_RET_OK){
		INTEG_TEST_PRINT("\n CRYS_RND_Instantiation failed with 0x%x \n",ret);
		return ret;
	}
	INTEG_TEST_PRINT("\n CRYS_RND_Instantiation passed\n");

	/*Try to create two vectors and check that the vectors are different*/
	ret = CRYS_RND_GenerateVector(&rndContext_ptr->rndState,
		rndVectors[test_index].rndTest_RandomVectorSize,
		rndVectors[test_index].rndTest_RandomVectorData1);
	if (ret != SA_SILIB_RET_OK){
		INTEG_TEST_PRINT("\n CRYS_RND_GenerateVector for vector 1 failed with 0x%x \n",ret);
		return ret;
	}
	INTEG_TEST_PRINT("\n CRYS_RND_GenerateVector 1 passed\n");

	ret = CRYS_RND_GenerateVector(&rndContext_ptr->rndState,
		rndVectors[test_index].rndTest_RandomVectorSize,
		rndVectors[test_index].rndTest_RandomVectorData2);

	if (ret != SA_SILIB_RET_OK){
		INTEG_TEST_PRINT(" CRYS_RND_GenerateVector for vector 2 failed with 0x%x \n",ret);
		return ret;
	}
	INTEG_TEST_PRINT("\n CRYS_RND_GenerateVector 2 passed\n");

	ret = SaSi_PalMemCmp(rndVectors[test_index].rndTest_RandomVectorData1,
		rndVectors[test_index].rndTest_RandomVectorData2,
		rndVectors[test_index].rndTest_RandomVectorSize);

	if (ret == SA_SILIB_RET_OK){
		INTEG_TEST_PRINT(" Two random vectors should not are the same \n");
		return ret;
	}
	INTEG_TEST_PRINT("\n Compare passed\n");
	INTEG_TEST_PRINT("\n All RND tests passed\n");
	INTEG_TEST_PRINT("\n==========================\n");

	ret = SA_SILIB_RET_OK;

return ret;
}

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

        /*Call ecc test*/
	ret = rnd_tests();
	if (ret != SA_SILIB_RET_OK) {
	    INTEG_TEST_PRINT("Failure in rnd_test,ret = 0x%x\n", ret);
	    goto exit_0;
	}

	INTEG_TEST_PRINT("All tests passed \n");

exit_0:
	/*Finish SaSi library*/
	SaSi_LibFini(rndContext_ptr);

exit_1:
	/*Unmap memory*/
	integration_tests_clear();

	return ret;
}


