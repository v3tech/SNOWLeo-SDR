/*
 * si5351.c
 *
 *  Created on: 2014-12-15
 *      Author: v3
 */
#include <stdio.h>
#include <math.h>

extern unsigned char snowleo_sdr_Config5[79][2];

/// Splits float into fraction integers A + B/C
void realToFrac(const float real, int *A, int *B, int *C)
{
	*A = (int)real;
	*B = (int)((real - (*A)) * 1048576 + 0.5);
	*C = 1048576;

	int a = *B;
	int b = *C;
	int temp;
	while( b!= 0) // greatest common divider
	{
		temp = a % b;
		a = b;
		b = temp;
	}
	*B = *B/a;
	*C = *C/a;
}
int fvco;
float feedbackDivider;
int R_array[8] ={1,2,4,8,16,32,64,128};
int R_best;
float abc1, abc2;
int FindVCO(int fout)
{
	int i = 1, temp;
	if(fout>20e6){
		printf("sample rate out of range!\n");
		return -1;
	}
	while(1){
		fvco = fout*i;
		if(fvco>700e6&&fvco<=850e6){
			temp = i;
			break;
		}
		i += 1;
	}
	for(i = 0; i < 8; i++){
		abc1 = (float)temp/R_array[i];
		if(abc1>6&&abc1<1800){
			R_best = i;
			break;
		}
		i += 1;
	}
	abc2 = (float)fvco/30.72e6;
	if(abc2<15||abc2>90){
		return -1;
	}
	return 0;
}
/** @brief Modifies register map with clock settings
    @return true if success
 */
int ConfigureClocks(int fout)
{
	if(FindVCO(fout) < 0)
		return -1;
	int addr, i, j;
	for(i=2; i<6; ++i)
	{

		addr = 42+i*8;

		int DivA;
		int DivB;
		int DivC;

		realToFrac(abc1, &DivA, &DivB, &DivC);

		if(fout <= 150000000)
		{
			unsigned MSX_P1 = 128 * DivA + floor(128 * ( (float)DivB/DivC)) - 512;
			unsigned MSX_P2 = 128 * DivB - DivC * floor( 128 * DivB/DivC );
			unsigned MSX_P3 = DivC;
			for(j = 0; j < 79; j++){
				if(snowleo_sdr_Config5[j][0] == addr)
					snowleo_sdr_Config5[j][1] = MSX_P3 >> 8;
				if(snowleo_sdr_Config5[j][0] == addr+1)
					snowleo_sdr_Config5[j][1] = MSX_P3;
				if(snowleo_sdr_Config5[j][0] == addr+2 )
					snowleo_sdr_Config5[j][1] = (R_best << 4);
				if(snowleo_sdr_Config5[j][0] == addr+2)
					snowleo_sdr_Config5[j][1] |= (MSX_P1 >> 16 ) & 0x03;
				if(snowleo_sdr_Config5[j][0] == addr+3)
					snowleo_sdr_Config5[j][1] = MSX_P1 >> 8;
				if(snowleo_sdr_Config5[j][0] == addr+4)
					snowleo_sdr_Config5[j][1] = MSX_P1;
				if(snowleo_sdr_Config5[j][0] == addr+5)
					snowleo_sdr_Config5[j][1] = 0;
				if(snowleo_sdr_Config5[j][0] == addr+5)
					snowleo_sdr_Config5[j][1] = (MSX_P2 >> 16) & 0x0F;
				if(snowleo_sdr_Config5[j][0] == addr+5)
					snowleo_sdr_Config5[j][1] |= (MSX_P3 >> 16) << 4;
				if(snowleo_sdr_Config5[j][0] == addr+6)
					snowleo_sdr_Config5[j][1] = MSX_P2;
				if(snowleo_sdr_Config5[j][0] == addr+7)
					snowleo_sdr_Config5[j][1] = MSX_P2 >> 8;

				/*m_newConfiguration[addr] = MSX_P3 >> 8;
				m_newConfiguration[addr+1] = MSX_P3;

				m_newConfiguration[addr+2] = 0;
				m_newConfiguration[addr+2] |= (MSX_P1 >> 16 ) & 0x03;
				m_newConfiguration[addr+3] = MSX_P1 >> 8;
				m_newConfiguration[addr+4] = MSX_P1;

				m_newConfiguration[addr+5] = 0;
				m_newConfiguration[addr+5] = (MSX_P2 >> 16) & 0x0F;
				m_newConfiguration[addr+5] |= (MSX_P3 >> 16) << 4;

				m_newConfiguration[addr+6] = MSX_P2;
				m_newConfiguration[addr+7] = MSX_P2 >> 8;*/
			}
		}
		else if( fout <= 160000000) // AVAILABLE ONLY ON 0-5 MULTISYNTHS
		{
			return -1;
		}
	}

	//configure pll
	for(i=0; i<2; ++i)
	{
		addr = 26+i*8;
		//calculate MSNx_P1, MSNx_P2, MSNx_P3
		int MSNx_P1;
		int MSNx_P2;
		int MSNx_P3;

		int DivA = 0;
		int DivB = 0;
		int DivC = 0;
		realToFrac(abc2, &DivA, &DivB, &DivC);

		MSNx_P1 = 128 * DivA + floor(128 * ( (float)DivB/DivC)) - 512;
		MSNx_P2 = 128 * DivB - DivC * floor( 128 * DivB/DivC );
		MSNx_P3 = DivC;

		for(j = 0; j < 79; j++){
			if(snowleo_sdr_Config5[j][0] == addr+4)
				snowleo_sdr_Config5[j][1] = MSNx_P1;
			if(snowleo_sdr_Config5[j][0] == addr+3)
				snowleo_sdr_Config5[j][1] = MSNx_P1 >> 8;
			if(snowleo_sdr_Config5[j][0] == addr+2)
				snowleo_sdr_Config5[j][1] = MSNx_P1 >> 16;
			if(snowleo_sdr_Config5[j][0] == addr+7)
				snowleo_sdr_Config5[j][1] = MSNx_P2;
			if(snowleo_sdr_Config5[j][0] == addr+6)
				snowleo_sdr_Config5[j][1] = MSNx_P2 >> 8;
			if(snowleo_sdr_Config5[j][0] == addr+5)
				snowleo_sdr_Config5[j][1] = 0;
			if(snowleo_sdr_Config5[j][0] == addr+5)
				snowleo_sdr_Config5[j][1] = (MSNx_P2 >> 16) & 0x0F;
			if(snowleo_sdr_Config5[j][0] == addr+5)
				snowleo_sdr_Config5[j][1] |= (MSNx_P3 >> 16) << 4;
			if(snowleo_sdr_Config5[j][0] == addr+1)
				snowleo_sdr_Config5[j][1] |= MSNx_P3;
			if(snowleo_sdr_Config5[j][0] == addr)
				snowleo_sdr_Config5[j][1] = MSNx_P3 >> 8;

			/*	m_newConfiguration[addr+4] = MSNx_P1;
			m_newConfiguration[addr+3] = MSNx_P1 >> 8;
			m_newConfiguration[addr+2] = MSNx_P1 >> 16;

			m_newConfiguration[addr+7] = MSNx_P2;
			m_newConfiguration[addr+6] = MSNx_P2 >> 8;
			m_newConfiguration[addr+5] = 0;
			m_newConfiguration[addr+5] = (MSNx_P2 >> 16) & 0x0F;

			m_newConfiguration[addr+5] |= (MSNx_P3 >> 16) << 4;
			m_newConfiguration[addr+1] |= MSNx_P3;
			m_newConfiguration[addr] |= MSNx_P3 >> 8;*/
		}
	}
//	for(i = 0; i < 79; i++){
//		printf("{0x%x,0x%x}\n", snowleo_sdr_Config5[i][0], snowleo_sdr_Config5[i][1]);
//	}
	return 0;
}


