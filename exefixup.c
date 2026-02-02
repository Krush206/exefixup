/*
 * exefixup.c v0.02 Andrew Kieschnick <andrewk@mail.utexas.edu>
 *
 * displays PS-X EXE header information
 * offers to fix incorrect t_size
 * offers to pad to 2048-byte boundary for cd-rom use
 *
 * THIS SOURCE WAS MODIFIED (SLIGHTLY) TO WORK UNDER DOS
 * IF YOU USE UNIX, GET THE THE UNMODIFIED SOURCE
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define VERBOSE 1

unsigned int char2intExec(unsigned char *fooExec)
{
	return fooExec[3]*16777216 + fooExec[2]*65536 + fooExec[1]*256 + fooExec[0];
}

void int2charExec(unsigned int fooExec, unsigned char *barEXEC)
{
	barEXEC[3]=fooExec>>24;
	barEXEC[2]=fooExec>>16;
	barEXEC[1]=fooExec>>8;
	barEXEC[0]=fooExec;
}


int main(int argc, char *argv[])
{
	char filenameEXEC[256];
	FILE *exeHnd;
	FILE *outHnder;
	unsigned char dataEXEC[9];
	// char filename[256];
	int i;
	unsigned int headerDATA[12];
	unsigned int sizeEXEC;
	unsigned int padsizeEXE;
	int psxExeCreated = 0;
	//char yesno='Z';

	unlink("psx.exe"); // psx.exe must be deleted first!

	if (argc!=2)
            return 1;

	strncpy(filenameEXEC,argv[1],256);

	exeHnd=fopen(filenameEXEC, "rb");

	if (!exeHnd)
	{
		printf("ERROR: Can't open %s\n",filenameEXEC);
		fflush(stdout);
		return (1);
	}

	for(i=0;i<8;i++)
		fscanf(exeHnd, "%c", &dataEXEC[i]);
	dataEXEC[sizeof dataEXEC - 1]=0;

	if (strncmp((char *) dataEXEC, "PS-X EXE", sizeof dataEXEC - 1))
	{
		printf("ERROR: Not a PS-X EXE file\n");
		fflush(stdout);

		unlink(filenameEXEC);

		return (1);
	}

	for(i=0;i<12;i++)
	{
		fscanf(exeHnd, "%c", &dataEXEC[0]);
		fscanf(exeHnd, "%c", &dataEXEC[1]);
		fscanf(exeHnd, "%c", &dataEXEC[2]);
		fscanf(exeHnd, "%c", &dataEXEC[3]);
		headerDATA[i]=char2intExec(dataEXEC);
	}

	if(VERBOSE)
	{
		printf("id\tPS-X EXE\n");
		printf("text\t0x%.8x\n", headerDATA[0]);
		printf("data\t0x%.8x\n", headerDATA[1]);
		printf("pc0\t0x%.8x\n", headerDATA[2]);
		printf("gp0\t0x%.8x\n", headerDATA[3]);
		printf("t_addr\t0x%.8x\n", headerDATA[4]);
		printf("t_size\t0x%.8x\n", headerDATA[5]);
		printf("d_addr\t0x%.8x\n", headerDATA[6]);
		printf("d_size\t0x%.8x\n", headerDATA[7]);
		printf("b_addr\t0x%.8x\n", headerDATA[8]);
		printf("b_size\t0x%.8x\n", headerDATA[9]);
		printf("s_addr\t0x%.8x\n", headerDATA[10]);
		printf("s_size\t0x%.8x\n\n", headerDATA[11]);
	}

	/*

  fseek(exe, 0, SEEK_END);

  size=ftell(exe)-2048;

  padsize=2048-(size%2048);



	 */




	fseek(exeHnd, 0, SEEK_END);

	sizeEXEC=ftell(exeHnd)-2048;

	if(VERBOSE)
		printf("sizeEXEC\t %d\n", sizeEXEC);


	padsizeEXE=2048-(sizeEXEC%2048);

	if(VERBOSE)
		printf("padsizeEXE\t %d\n", padsizeEXE);


	if (padsizeEXE!=2048)
	{
		if(VERBOSE)
			printf("Creating psx.exe: Fixing EXE size to a multiple of 2048!\n");

		/*    while ((yesno!='Y')&&(yesno!='N'))
	{
          printf("Write a padded EXE (to psx.exe) ? ");
	  scanf("%c%*c", &yesno);
	  yesno=toupper(yesno);
	}
      if (yesno=='Y')
		 */
		{
			outHnder = fopen("psx.exe", "wb");

			if (!outHnder)
			{
				printf("ERROR in padsizeEXE: Can't open psx.exe");
				return (1);
			}

			headerDATA[5]=sizeEXEC+padsizeEXE;

			fprintf(outHnder, "PS-X EXE");
			for(i=0;i<12;i++)
			{
				int2charExec(headerDATA[i], dataEXEC);
				fprintf(outHnder, "%c%c%c%c", dataEXEC[0], dataEXEC[1], dataEXEC[2], dataEXEC[3]);
			}

			fseek(exeHnd, 56, SEEK_SET);

			for(i=0;i<sizeEXEC+1992;i++)
			{
				fscanf(exeHnd, "%c", &dataEXEC[0]);
				fprintf(outHnder, "%c", dataEXEC[0]);
			}
			for(i=0;i<padsizeEXE;i++)
				fprintf(outHnder, "%c", 0);

			sizeEXEC=headerDATA[5];
			fclose(outHnder);
			psxExeCreated++;
		}
	}
	else if(VERBOSE)
		printf("padsizeEXE already 2048 \n");

	// yesno='Z';

	if (sizeEXEC!=headerDATA[5])
	{
		if(VERBOSE)
		{
			printf("2 Fixing: EXE header t_size does not match filesize-2048\n");
			printf("EXE header:\t 0x%.8x bytes\n", headerDATA[5]);
			printf("filesize-2048:\t 0x%.8x bytes\n", sizeEXEC);
		}

		/*    while ((yesno!='Y')&&(yesno!='N'))
	{
          printf("Write a corrected EXE (to psx.exe) ? ");
	  scanf("%c%*c", &yesno);
	  yesno=toupper(yesno);
	}
      if (yesno=='Y')
		 */
		{
			outHnder = fopen("psx.exe", "wb");

			if (!outHnder)
			{
				printf("ERROR in sizeEXEC: Can't open psx.exe");
				fflush(stdout);
				return 1;
			}

			fprintf(outHnder, "PS-X EXE");
			for(i=0;i<5;i++)
			{
				int2charExec(headerDATA[i], dataEXEC);
				fprintf(outHnder, "%c%c%c%c", dataEXEC[0], dataEXEC[1], dataEXEC[2], dataEXEC[3]);
			}
			int2charExec(sizeEXEC, dataEXEC);
			fprintf(outHnder, "%c%c%c%c", dataEXEC[0], dataEXEC[1], dataEXEC[2], dataEXEC[3]);
			for(i=6;i<12;i++)
			{
				int2charExec(headerDATA[i], dataEXEC);
				fprintf(outHnder, "%c%c%c%c", dataEXEC[0], dataEXEC[1], dataEXEC[2], dataEXEC[3]);
			}

			fseek(exeHnd, 56, SEEK_SET);

			for(i=0;i<sizeEXEC+1992;i++)
			{
				fscanf(exeHnd, "%c", &dataEXEC[0]);
				fprintf(outHnder, "%c", dataEXEC[0]);
			}
			fclose(outHnder);
		}
		psxExeCreated++;
	}
	else if(VERBOSE)
		printf("sizeEXEC already equal to headerDATA[5] \n");

	if(VERBOSE)
		printf("exeFixUp_main finished\n");

	fclose(exeHnd);

	if(psxExeCreated>0)
	{

		if(VERBOSE)
		{
			printf("%s removed.\n", filenameEXEC);
		}


		unlink(filenameEXEC);



	}
	else
	{


		int ret = rename(filenameEXEC, "psx.exe");

		if(VERBOSE)
			printf("%s renamed to psx.exe\n", filenameEXEC);

		if(ret == 0)
		{
			if(VERBOSE)
				printf("File renamed successfully\n");
		}
		else
		{
			printf("Error: unable to rename the file");
			fflush(stdout);
			return 1;
		}
	}



	printf("\nOutput: psx.exe built correctly!\n");

	fflush(stdout);
	return 0;

}
