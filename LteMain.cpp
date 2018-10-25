#include "stdafx.h"
#include "Kernel.h"

int main(int argc, char* argv[])
{
	CKernel Kernel("ZTE_LTE_ADJ_PRO");
	Kernel.RegisterService(argc, argv);
}