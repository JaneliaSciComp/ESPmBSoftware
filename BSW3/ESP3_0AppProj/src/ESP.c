
#include <stdio.h>
#include "platform.h"
#include "xuartlite_i.h"
#include "string.h"
#include "xparameters.h"							// generated by XPS has BSP parameters
#include "rtephyseng_plbw.h"      					// generated by System Generator

#define BUFFER_SIZE	20								// external command
#define	ACK			6								// ascii code = ack
#define	ENTER 		13								//	ascii code = return
#define HS32_CONTROL_BIT			0b0000000000000001		//  headstage type control register bit
#define DtoAstart0_BIT				0b0000000000000010		// DtoA0 start bit in control register
#define DtoAstart1_BIT				0b0000000000000100		// DtoA0 start bit in control register
#define New_Sample_Available_BIT	0b0000000000001000		// a new sample is available for the engines
#define Eng_Reset_BIT				0b0000000000010000		// Reset the Engines
#define Buffer0Fill_BIT			0b0000000000000001		// buffer 0 fill bit in status
#define NUM_E1_SAMPLES		512						// E1 Sample memory size

#define CONTROL16_BIT		0b1000000000000000		// debug

XUartLite 	UartLite;		//global declaration

void print(char *str);

void processCharacter();
void processCommandString();
void Send_String (unsigned char *, unsigned int);
void processSample();
void processEngineCommand (unsigned char, char *str);
void init_shared_memory (void);
void init_ESP(void);
void usec_wait(Xint32);
void Init_AtoD_Converters(void);
void setDtoAdatapointers(u32, u32, u32);
void setEngineDataPointers(u32, u32, u32);
void showE1DataMemory(void);
u32 htoi(unsigned char *, u32);


u16 ChanHS_2_Offset(int, int);

u16 *DtoA0_Data_RegA, *DtoA0_Data_RegB, *DtoA1_Data_RegA, *DtoA1_Data_RegB;
unsigned *ESP_Control_Reg, *ESP_Status_Reg;
u16 *Input_Data_Memory;
u32 *pEngine1_Data_Memory, *pEngine1_Data_Last, *AtoD_Data_Memory_Engine1_Buf0, *AtoD_Data_Memory_Engine1_Buf1;
u32 *pDtoA_data0[4], *pDtoA_data1[4];
u32 *pEngine_data0[2], *pEngine_data1[2];				// pEngine_dataX[numEngines] holds the data location pointers for each of the engines
														// for the 2 swinging buffers(X says which buffer)
u32 *pE1_data0, *pE1_data1;
u32 currentHS, currentChannel; 							// used by the routines to set the engine parameters

// EngRegPointers holds pointers to the shared memory locations that each engine
// uses to pass out the filter and LSD results

typedef struct
{
	u32 *pEngLSDOut;
	u32 *pEngFilterOut;
	u32 *pEngDecimationReg;
	u32 *pEngFilterLengthReg;
	u32 *pEngTemplateSizeReg;
	u32 *pEngFilterMemory;
	u32 *pEngTemplateMemory;
}EngRegPointerStruct;

EngRegPointerStruct EngRegPointers[4];

u16	engineThresholds[4];									// holds the thresholds for lsd out in 16.12 format

volatile static u32 status, control;
volatile u32 *ESP_status_reg;
static unsigned int numberOfChar;							// number of characters in Inst
static unsigned char Inst[BUFFER_SIZE];						// holds the current instruction string
static unsigned int hsType;									//  type of HS 0:16 chan, 1:32 chan
volatile unsigned char newSample;							// set by A/D process when a new sample is ready
volatile xc_status_t funcStatus;										//for reporting errors
u32 value;
xc_iface_t *iface;
xc_from_reg_t *fromreg_ESP_Status_Reg;
xc_from_reg_t *fromreg_ESP_Control_Reg;
xc_from_reg_t *fromreg_dtoa0_data_regA;
xc_from_reg_t *fromreg_dtoa0_data_regB;
xc_from_reg_t *fromreg_dtoa1_data_regA;
xc_from_reg_t *fromreg_dtoa1_data_regB;

xc_shram_t	*AtoDInputShram;
xc_shram_t	*E1_InputShram;
xc_shram_t	*E1_FilterShram;
xc_shram_t	*E1_TemplateShram;

volatile u16 Buffer0Fill, newBuffer0Fill;

//typedef  struct
//{
//		unsigned short int hs;
//		unsigned short int chan;
//		unsigned int decimation;
//		unsigned int numFilterCoef;
//		unsigned int numTemplateCoef;
//		int filterCoef[MAX_NUM_FILTER_COEF];
//		int templateCoef[MAX_NUMBER_TEMPLATE_COEF];
//}Engine;
//
//Engine Engines[MAX_NUM_ENGINES];

int main()

{
    init_platform();
    init_shared_memory();
    init_ESP();
    numberOfChar = 0;
    XUartLite_Initialize(&UartLite, XPAR_XPS_UARTLITE_0_DEVICE_ID);
	xil_printf("\n\rESP Command Processor (pdw)\n\r");
    while(1){
    	// If there is a character send it to the character handler process
		if (!XUartLite_IsReceiveEmpty(XPAR_XPS_UARTLITE_0_BASEADDR)) processCharacter();
		if (~(Eng_Reset_BIT && control)){							// check if the device is on - processing samples.
			newBuffer0Fill = (*ESP_Status_Reg) & Buffer0Fill_BIT ;		// Buffer0Fill says which buffer is filling-- wait for a change
			//funcStatus = xc_read(iface, fromreg_ESP_Status_Reg->dout, &newBuffer0Fill);
			//xil_printf("%x\n", *ESP_Status_Reg);
			control ^= CONTROL16_BIT;		//debug
			*ESP_Control_Reg = control;		//debug
			if (newBuffer0Fill ^ Buffer0Fill) {
				Buffer0Fill = newBuffer0Fill;
				//xil_printf("s\n");
				processSample();
			}
		}
	}

    cleanup_platform();
}
u32 value;
xc_iface_t *iface;
xc_from_reg_t *fromreg_ESP_Status_Reg;
xc_from_reg_t *fromreg_ESP_Control_Reg;

// set DtoA pointers to output either filter or LSD output from the engine
// set both pointers to the same address because there is not swinging buffers for
// engine output.

void setEngineOutputPointers(DtoAchan, engine, outputType)
u32 DtoAchan, engine, outputType;
{
	xil_printf("setting eop\n");
	if (outputType == 0){
		pDtoA_data0[DtoAchan] = EngRegPointers[engine].pEngLSDOut;
		pDtoA_data1[DtoAchan] = EngRegPointers[engine].pEngLSDOut;
	}
	else{
		pDtoA_data0[DtoAchan] = EngRegPointers[engine].pEngFilterOut;
		pDtoA_data1[DtoAchan] = EngRegPointers[engine].pEngFilterOut;
	}
}

void setDtoAdatapointers(DtoAchan, HS, chan)
u32 DtoAchan, HS, chan;
{
	u32 offset;
	offset = HS*32 + chan;
	pDtoA_data0[DtoAchan] = (u32 *)(XPAR_RTEPHYSENG_PLBW_0_MEMMAP_ATOD_DATAMEM + offset*sizeof(u32));
	pDtoA_data1[DtoAchan] = (u32 *)(XPAR_RTEPHYSENG_PLBW_0_MEMMAP_ATOD_DATAMEM + (256+offset)*sizeof(u32));
	xil_printf("dtoa pnt: %x %x\n", pDtoA_data0[DtoAchan], pDtoA_data1[DtoAchan] );
}

// This routine sets the engine pointers for the two swinging data buffers.  Remember that an Engine processes data from one channel. The
// data for that channel will be in the same buffer location in the A/D data memory each time. We have two pointers because there are swinging buffers
// so while one is filling (seperate process) the other is having the data moved of to the engine memory.  These pointers are used for extracting the
// data from the input A/D buffer for a specific channel, they are not incremented or changed unless the channel that is being operated upon by the
// engine changes

void setEngineDataPointers(Engine, HS, chan)
u32 Engine, HS, chan;
{
	u32 offset;
	offset = HS*32 + chan;
	pEngine_data0[Engine] = (u32 *)(XPAR_RTEPHYSENG_PLBW_0_MEMMAP_ATOD_DATAMEM + offset*sizeof(u32));
	pEngine_data1[Engine] = (u32 *)(XPAR_RTEPHYSENG_PLBW_0_MEMMAP_ATOD_DATAMEM + (256+offset)*sizeof(u32));
	xil_printf("eng pointers %x %x\n", pEngine_data0[Engine],pEngine_data1[Engine]);
}
void init_ESP(void)
{
	control = 0;							//initialize the control register
	// set the DtoA pointers using some default channels
	setDtoAdatapointers(0, 1, 0);			//DtoA:0 HS:1 ch:0
	setDtoAdatapointers(1, 1, 1);			//DtoA:0 HS:1 ch:1
	setDtoAdatapointers(2, 1, 0);			//DtoA:0 HS:1 ch:0  for debug
	setDtoAdatapointers(3, 1, 3);

	setEngineDataPointers(0, 1, 0);			// engine(0->numEng-1, HS(0->numHS-1), Chan(0->15 or 31)
	setEngineOutputPointers(3, 0, 1);		// debug set a/d 3 to output filter from E0
	*EngRegPointers[0].pEngDecimationReg = 0;
	*EngRegPointers[0].pEngFilterLengthReg = 1;    // these two instructions should set the filter length to 1
	*EngRegPointers[0].pEngFilterMemory = 0x7FFF;	// and the filter coefficient to .9999 (pass thru)

	*EngRegPointers[0].pEngTemplateSizeReg = 1;     //these two make template 1 long with 0 as a value
	*EngRegPointers[0].pEngTemplateMemory = 0x0000;   // should get (filter_data - 0)^^2
	currentHS = 1;
	currentChannel = 0;
}
void init_shared_memory(void)
{
	XC_CfgInitialize(&iface, &RTEPHYSENG_PLBW_ConfigTable[0]);

	// obtain the memory location for storing the settings of shared memory "t"
	funcStatus = xc_get_shmem(iface, "ESP_Status_Reg", (void **) &fromreg_ESP_Status_Reg);
	if (funcStatus != XC_SUCCESS) xil_printf("ESP_status_reg get error %i\n", funcStatus);

	funcStatus = xc_get_shmem(iface, "ESP_Control_Reg", (void **) &fromreg_ESP_Control_Reg);
	if (funcStatus != XC_SUCCESS) xil_printf("ESP_Control_Reg get error %d\n", funcStatus);

	funcStatus = xc_get_shmem(iface, "dtoa0_data_regA", (void **) &fromreg_dtoa0_data_regA);
	if (funcStatus != XC_SUCCESS) xil_printf("dtoa0_data_regA get error %i\n", funcStatus);

	funcStatus = xc_get_shmem(iface, "dtoa0_data_regB", (void **) &fromreg_dtoa0_data_regB);
	if (funcStatus != XC_SUCCESS) xil_printf("dtoa0_data_regB get error %i\n", funcStatus);

	funcStatus = xc_get_shmem(iface, "dtoa1_data_regA", (void **) &fromreg_dtoa1_data_regA);
	if (funcStatus != XC_SUCCESS) xil_printf("dtoa1_data_regA get error %i\n", funcStatus);

	funcStatus = xc_get_shmem(iface, "dtoa1_data_regB", (void **) &fromreg_dtoa1_data_regB);
	if (funcStatus != XC_SUCCESS) xil_printf("dtoa1_data_regB get error %i\n", funcStatus);

	funcStatus = xc_get_shmem(iface, "AtoD_DataMem", (void **) &AtoDInputShram);
	if (funcStatus != XC_SUCCESS) xil_printf("AtoD_DataMem get error %i\n", funcStatus);

	funcStatus = xc_get_shmem(iface, "E1_Data_RAM", (void **) &E1_InputShram);
	if (funcStatus != XC_SUCCESS) xil_printf("E1_Data_Mem get error %i\n", funcStatus);

	funcStatus = xc_get_shmem(iface, "E1_filter_RAM", (void **) &E1_FilterShram);
	if (funcStatus != XC_SUCCESS) xil_printf("E1_Filter_Mem get error %i\n", funcStatus);

	funcStatus = xc_get_shmem(iface, "E1_Template_RAM", (void **) &E1_TemplateShram);
	if (funcStatus != XC_SUCCESS) xil_printf("E1_Template_Mem get error %i\n", funcStatus);

	//Set up pointers to the memory mapped data registers. Defines in xparameters.h//
	// Control and Status Registers //
	ESP_Control_Reg = (unsigned *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_ESP_CONTROL_REG;
	ESP_Status_Reg = (unsigned *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_ESP_STATUS_REG;

	// D to A data registers //
	DtoA0_Data_RegA = (u16 *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_DTOA0_DATA_REGA;
	DtoA0_Data_RegB = (u16 *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_DTOA0_DATA_REGB;
	DtoA1_Data_RegA = (u16 *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_DTOA1_DATA_REGA;
	DtoA1_Data_RegB = (u16 *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_DTOA1_DATA_REGB;


	// Initial Data Input Swinging Buffer and Engine Pointers
	AtoD_Data_Memory_Engine1_Buf0 = (u32 *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_ATOD_DATAMEM;  // pointers to first data location
	AtoD_Data_Memory_Engine1_Buf1 = (u32 *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_ATOD_DATAMEM;
	pEngine1_Data_Memory = (u32 *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_E1_DATA_RAM;
	pEngine1_Data_Last = pEngine1_Data_Memory + 128;
	xil_printf("E1 %x %x\n", pEngine1_Data_Memory, pEngine1_Data_Last);

	// set the pointers to the engine registers in the array holding the pointers
	EngRegPointers[0].pEngFilterOut = (u32 *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_E1_FILTER_OUT_REG;
	EngRegPointers[0].pEngLSDOut = (u32 *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_E1_TEMPLATE_ACC_REG;
	EngRegPointers[0].pEngDecimationReg = (u32 *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_E1_DECIMATION;
	EngRegPointers[0].pEngFilterLengthReg = (u32 *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_E1_FILTER_LENGTH;
	EngRegPointers[0].pEngTemplateSizeReg = (u32 *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_E1_TEMPLATE_SIZE;
	EngRegPointers[0].pEngFilterMemory = (u32 *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_E1_FILTER_RAM;
	EngRegPointers[0].pEngTemplateMemory = (u32 *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_E1_TEMPLATE_RAM;

	// Include the pointers for other engines here as they are added //
}


void Send_String(unsigned char Text[BUFFER_SIZE], unsigned int numberOfChar)
{	XUartLite_Send(&UartLite, &Text[0], numberOfChar);
	while (XUartLite_IsSending(&UartLite));
}

void processCharacter(void)
{	unsigned char		nextChar;

	nextChar =	XUartLite_RecvByte(XPAR_XPS_UARTLITE_0_BASEADDR) ;    //get character
//	xil_printf("%c", nextChar);									// echo char.
	if (nextChar == ENTER) {									// ENTER signals end of the string
		Inst[numberOfChar++] = nextChar;						// save the ENTER
		processCommandString();										// and process the string
		return;
	}
	if (numberOfChar == 0){										// if  0, then we are starting a command
		if (nextChar != '~')return;								// if its not a "~" then toss it and return
		Inst[numberOfChar++] = nextChar;						// save the ~ and increment the index
	}
	else {														// not the first character and not the last
		Inst[numberOfChar++] = nextChar;						// save it and increment index
		if (numberOfChar >= BUFFER_SIZE){						// check for overflow
			xil_printf("command overflow\n");
			numberOfChar = 0;
			return;
		}
	}
	return;
}

// string Inst[] should come here with at least 1 character and no ENTER
void processCommandString(void) {
	unsigned char engineNum;
	int i, j;
	u32 value0, value1, value2;
	u32 *ptr;

	Inst[numberOfChar++] = '\n';				// add a 'new line c' character
	//Inst[i++] = '\r';
	//Inst[numberOfCharacters] = 0;				// terminate the string
	Inst[0] = '>';								// Replace the first character (sync)
	Send_String(Inst, numberOfChar);  			// return Instruction Received
	numberOfChar = 0;

	switch (Inst[1])
	{

	case 'D':										// set signal for D/A converter channel
													// format is D[0-3]:h[0-5]:c[00-31]  where D is the D to A channel[0-3]  h is the head stage, c is chan
													// or D[0-3]:e[0-3]:s[0,1] where E[0-3] is the engine  and s[0,1] is S:0  is LSD  output, S:1 is filter output
		value0 = Inst[2] - '0';							// value 0 is the d to a channel
		if(Inst[4] == 'h'){								// Display a data channel
			value1 = Inst[5] - '0';						// value 1 is the HS number
			value2 = (Inst[8]-'0')*10+Inst[9]-'0';		// value 2 is the channel
			setDtoAdatapointers(value0, value1, value2);
			xil_printf("dtoa:%d HS:%d CH:%d\n", value0, value1,value2);
		}
		else {											// set up DtoA pointers to display engine outputs
			value1 = Inst[5] - '0';							// value 1 is the Engine Number
			value2 = Inst[8]-'0';							// value 2 is the Signal 0 or 1
			xil_printf("dtoa:%d Eng:%d Sig:%d\n", value0, value1,value2);
			setEngineOutputPointers(value0, value1, value2);
		}
		break;

	case 'H':									//set the HS type, should be one digit following the H:
												// format is ~H:<t>  where <t> is the head stage type number
												// 0 for 16 chan, 1 for 32 chan
		hsType = Inst[3] - '0';					// convert number to int
		if (hsType == 0) control &= (~HS32_CONTROL_BIT);   // clear the bit
		else control |=  HS32_CONTROL_BIT;
		xil_printf("hs type:%d  control %x\n", hsType, control);
		funcStatus = xc_write(iface, fromreg_ESP_Control_Reg->dout, control);
		if (funcStatus != XC_SUCCESS) xil_printf("ESP_control_reg write error %i\n", funcStatus);
		xil_printf("hs type:%d  control %d\n", hsType, control);
		break;

	case 'h':									// command to set the Head stage for an engine
												// format is ~h:Ey:z where x is engine, and y is the value [0-5]
		engineNum = Inst[4] - '0';				// convert the engine digit after the E:
		currentHS = Inst[6] - '0';				// set the currentHS variable
		setEngineDataPointers(engineNum, currentHS, currentChannel);     // when HS changes, the Engine Data Pointers change
		xil_printf("Eng:%d  HS %x\n", engineNum, currentHS);
		break;

	case 'c':									// command to set the Channel for an engine
												// format is ~c:Ey:z where x is engine, and y is the value [0-31], two digits with leading zero
		engineNum = Inst[4] - '0';				// convert the engine digit after the E:
		currentChannel = (Inst[6] - '0') * 10 + Inst[7] - '0';				// set the currentChannel variable
		setEngineDataPointers(engineNum, currentHS, currentChannel);     // when HS changes, the Engine Data Pointers change
		xil_printf("Eng:%d chan %d\n", engineNum, currentChannel);
		break;

	case 'T':									// set the Threshold for an Engine
												// format is ~T:Ey:z  where y is the engine and z is the threshold in hex(16.4) format
		engineNum = Inst[4] - '0';				// convert the engine digit after the E:
		engineThresholds[engineNum] = htoi(&Inst[7], 4);  // convert the hex to int and store in Thresholds array
		xil_printf("Eng:%d  Th:%x\n", engineNum, currentHS);
		break;

	case 'd':									// set decimation (number of samples to skip) for an engine
												// format is ~d:Ey:z where y is the engine and z is the integer number of samples to skip
		engineNum = Inst[4] - '0';				// convert the engine digit after the E:
		*(EngRegPointers[engineNum].pEngDecimationReg) = (Inst[6] - '0') * 10 + Inst[7] - '0';   //set into decimation register
		xil_printf("Eng:%d reg:%x  dec:%d\n", engineNum, (EngRegPointers[engineNum].pEngDecimationReg), ((Inst[6] - '0') * 10 + Inst[7] - '0') );
		break;


	case 'f':									// set a filter coefficient
												// format is ~f:Ey:<addr><sp><value> where addr is 3 digit hex value is 4 digit hex
		engineNum = Inst[4] - '0';				// convert the engine digit after the E:
		value1 = htoi(&Inst[6], 3);				// convert adddress
		value2 = htoi(&Inst[10], 4);			// convert data
		//*(EngRegPointers[engineNum].pEngFilterMemory + value1) = value2;
		//*(XC_GetAddr(EngRegPointers[engineNum].pEngFilterMemory, value1)) = value2;
		// write value to the shared memory "shram1"
		XC_Write(iface, XC_GetAddr(E1_FilterShram->addr, value1), (const unsigned) value2);
		xil_printf("Eng:%d  add:%4x Fval:%4x\n", engineNum, EngRegPointers[engineNum].pEngFilterMemory + value1, value2 );
		break;


	case 't':									// set a template coefficient
												// format is ~t:Ey:<addr><sp><value>   where addr and value are in hex
		engineNum = Inst[4] - '0';				// convert the engine digit after the E:
		value1 = htoi(&Inst[6], 3);				// convert adddress
		value2 = htoi(&Inst[10], 4);			// convert data
		XC_Write(iface, XC_GetAddr(E1_TemplateShram->addr, value1), (const unsigned) value2);
		xil_printf("Eng:%d  add:%4x Tval:%4x\n", engineNum, XC_GetAddr(EngRegPointers[engineNum].pEngTemplateMemory, value1), value2 );
		break;

	case 'l':									// set filter length register
												// format is ~l:Ey:zzz	where y is engine # zzz is filter length
		engineNum = Inst[4] - '0';				// convert the engine digit after the E:
		value1 = ((Inst[6] - '0')*10 + (Inst[7] - '0'))*10 + (Inst[8] - '0');
		*(EngRegPointers[engineNum].pEngFilterLengthReg) = value1;
		xil_printf("F length %d add:%x\n", value1, (EngRegPointers[engineNum].pEngFilterLengthReg) );
		break;

	case 's':									// set template size register
												// format is ~s:Ey:zzz	where y is engine # zzz is filter length
		engineNum = Inst[4] - '0';				// convert the engine digit after the E:
		value1 = ((Inst[6] - '0')*10 + (Inst[7] - '0'))*10 + (Inst[8] - '0');
		*(EngRegPointers[engineNum].pEngTemplateSizeReg) = value1;
		xil_printf("T size:%d  add %x\n", value1, (EngRegPointers[engineNum].pEngTemplateSizeReg) );

		break;

	case 'r':									//reset the engines
												// format is ~r
		control |=  (u32)Eng_Reset_BIT;				// set the start bits in the control register
		*ESP_Control_Reg = control;						// write the control register, the start bit must be high for >50nSec
		control &= (u32)(~Eng_Reset_BIT);				//clear the start bits in the control register
		*ESP_Control_Reg = control;
		pEngine1_Data_Memory = (u32 *)(XPAR_RTEPHYSENG_PLBW_0_MEMMAP_E1_DATA_RAM); // start filling at 0
		break;

	case 'I':									//dump data memory
		i=0;
		//ptr=DtoA0_data_pointer0;
		for (i = 0; i < 1024; i++) XC_Write(iface, XC_GetAddr(AtoDInputShram->addr, i), (u16) i);
		xil_printf("  done\n");
		break;

	case 'M':

		xil_printf("read\n");
		for (i = 0; i < 64; i++){
			XC_Read(iface, XC_GetAddr(AtoDInputShram->addr, i), &value);
			xil_printf("%x %x %x\n", i, XC_GetAddr(AtoDInputShram->addr, i), value);
		}
		break;
	case 'y': 	// debug										// dump filter or template memory memory
																// format is ~y:Ex:<t,f> where x is engine #
		engineNum = Inst[4] - '0';
		xil_printf("engine:%d\n", engineNum);
		if (Inst[6] == 'f')ptr = EngRegPointers[engineNum].pEngFilterMemory;
		else	ptr = EngRegPointers[engineNum].pEngTemplateMemory;
			j=0;
			for (i=0; i<128; i++){
				xil_printf("%3d %04x ", i, *ptr);
				ptr++;
				if(j == 7){
					xil_printf("\n");
					j=0;
				}
				else j++;
			}
		break;
	case 'Z':										// print 50 values from HS 1 (0-5) chan 2 (0-15 or 0-31)
		showE1DataMemory();
		break;
	case 'z':
		xil_printf("d to a(2) %x %d\n", pDtoA_data1[2], *pDtoA_data1[2] );
		break;
	}
	return;
}

// minimalist hex to binary int converter -- no error checking of any sort
// assumes a-f are capital letters

u32 htoi(unsigned char *string, u32 numdigits)
{
	u32 i, out;
	out = 0;
	for (i=0; i<numdigits; i++, string++){
		if(*string <= '9') out = (out << 4) + (*string - '0');
		else out = (out << 4) + (*string - 87);					// 87 is ('a' - 10)
	}
	return (out);
}

u16 ChanHS_2_Offset(int HS, int chan)
{
	return (HS*32+chan);
}

// processSample is a routine whenever there is a new sample available for distribution. It should be fast.
// It has to know the number of active Engines that need samples. It has to know which of the two
// swinging buffers it should get the samples from and it has to know where in the buffer the sample for
// each Engine should come from and where it should go. So the source will be in an array of pointers
// called dataSources and the destinations in an array of pointers called dataSinks
// 	AtoD_Data_Memory_Engine1_Buf0 = (u32 *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_ATOD_DATAMEM;  // pointers to first data location
// AtoD_Data_Memory_Engine1_Buf1 = (u32 *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_ATOD_DATAMEM;
// pEngine1_Data_Memory = (u32 *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_E1_DATA_RAM;
// pEngine1_Data_Last = pEngine1_Data_Memory + 256;

//
void processSample(void){
	if (Buffer0Fill != 0)
		{
		// buffer 0 is filling get samples from buffer 1-- repeat for other engines
		*pEngine1_Data_Memory = (*pEngine_data1[0])<<1;             		// this actually moves the sample
			if(++pEngine1_Data_Memory >= pEngine1_Data_Last){			// now check to see if the pointer is at the end of the buffer
				pEngine1_Data_Memory = (u32 *)(XPAR_RTEPHYSENG_PLBW_0_MEMMAP_E1_DATA_RAM);  // if so set it back to the start
			}
		*DtoA0_Data_RegA = (*pDtoA_data1[0]);
		*DtoA0_Data_RegB = (*pDtoA_data1[1]);
		*DtoA1_Data_RegA = (*pDtoA_data1[2]);
		//xil_printf("E0 %x %d\n", pDtoA_data1[2], *pDtoA_data1[2] );
		*DtoA1_Data_RegB = (*pDtoA_data1[3])^0x8000;		//the ^0x8000 compliments highest order bit
	} else
		{
		// buffer 1 is filling, get samples from buffer 0
		*pEngine1_Data_Memory++ = (*pEngine_data0[0])<<1 ;					// store the sample from the input buffer
			if(pEngine1_Data_Memory >= pEngine1_Data_Last){				// check for end of buffer
				pEngine1_Data_Memory = (u32 *)(XPAR_RTEPHYSENG_PLBW_0_MEMMAP_E1_DATA_RAM);
			}
		*DtoA0_Data_RegA = (*pDtoA_data0[0]);
		*DtoA0_Data_RegB = (*pDtoA_data0[1]);
		*DtoA1_Data_RegA = (*pDtoA_data0[2]);
		*DtoA1_Data_RegB = (*pDtoA_data0[3])^0x8000;
	}
	control |=  (u32)(DtoAstart0_BIT | DtoAstart1_BIT | New_Sample_Available_BIT);	// set the start bits in the control register
	*ESP_Control_Reg = control;						// write the control register, the start bit must be high for >50nSec
	//xil_printf("control%x\n", control);
												//clear the start bits in the control register
	control &= (u32)(~(DtoAstart0_BIT | DtoAstart1_BIT | New_Sample_Available_BIT));
	*ESP_Control_Reg = control;

	//xil_printf("%x\n", control);
}


//====================================================
//	Wait/Delay
//====================================================
void usec_wait(Xint32 delay)
{		Xint32 val = (delay * (Xint32)(XPAR_MICROBLAZE_CORE_CLOCK_FREQ_HZ * 17) / 100000000 );
		while(val--)
		asm("nop");
}




// showE1DataMemory debug routine to play the E1 data memory out to D/A 0

//void showE1DataMemory(void){
//		u32 *pE1;
//		int  i, j, l;
//
//		control &= (u32)(Eng_Reset_BIT);				// stop processing samples
//		*ESP_Control_Reg = control;
//		for (i=1;i<100;i++){
//			j=0;
//					pE1=(u32 *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_E1_DATA_RAM;
//					while (j<256){
//						asm("nop");
//						*DtoA0_Data_RegA = *pE1;
//						asm("nop");
//			//			xil_printf("%d %x %x\n", j, pE1, *pE1);
//						control |=  (u32)DtoAstart0_BIT;				// set the start bits in the control register
//						*ESP_Control_Reg = control;						// write the control register, the start bit must be high for >50nSec
//						pE1++;
//						control &= (u32)(~DtoAstart0_BIT);				//clear the start bits in the control register
//						*ESP_Control_Reg = control;
//						j++;
//						for(l=0;l<400;l++);
//					}
//		}
//		control |= (u32)(~Eng_Reset_BIT);				// start processing samples again
//
//
//
//	xil_printf("Done E1 display\n");
//	return;
//}
void showE1DataMemory(void){
		u32 *pE1;
		int  i, j;

		control |= (u32)(Eng_Reset_BIT);				// stop processing samples
		*ESP_Control_Reg = control;
//		engineNum = Inst[4] - '0';
//		xil_printf("engine:%d\n", engineNum);
//		if (Inst[6] == 'f')ptr = EngRegPointers[engineNum].pEngFilterMemory;
//		else	ptr = EngRegPointers[engineNum].pEngTemplateMemory;
		pE1=(u32 *)XPAR_RTEPHYSENG_PLBW_0_MEMMAP_E1_DATA_RAM;
			j=0;
			for (i=0; i<128; i++){
				xil_printf("%3d %04x ", i, *pE1);
				pE1++;
				if(j == 7){
					xil_printf("\n");
					j=0;
				}
				else j++;
			}
		control &= (u32)(~Eng_Reset_BIT);				// start processing samples again



	xil_printf("Done E1 display\n");
	return;
}
