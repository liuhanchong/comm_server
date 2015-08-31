/*
 * main.c
 *
 *  Created on: 2015年7月21日
 *      Author: liuhanchong
 */
 
#include "../header/servermanage.h"

int main(int argc, char *argv[])
{
	int nReturn = 0;
	nReturn = Start(argc, argv);

	if (nReturn == 0)
	{
		ERROR_DESC("main", ERR_OPESER);
		return 1;
	}

	return 0;
}

