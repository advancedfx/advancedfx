#include "AfxTypes.h"
#include "AfxInterface.h"


void uuid_test()
{
	ADVANCEDFX_UUID_VAR(test_var, ADVANCEDFX_IFACTORY_UUID_FN);

	unsigned char equal = ADVANCEDFX_UUID_EQUAL(ADVANCEDFX_IFACTORY_UUID_FN, test_var) ? 1 : 0;

	int cmpt = ADVANCEFX_CMP_UUID_VARS(test_var, test_var);
}