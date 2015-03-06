///////////////////////////////////////////////////////////////-*-C-*-
//
// Copyright (c) 2010 Xilinx, Inc.  All rights reserved.
//
// Xilinx, Inc.  XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION
// "AS IS" AS  A COURTESY TO YOU.  BY PROVIDING  THIS DESIGN, CODE, OR
// INFORMATION  AS  ONE   POSSIBLE  IMPLEMENTATION  OF  THIS  FEATURE,
// APPLICATION OR  STANDARD, XILINX  IS MAKING NO  REPRESENTATION THAT
// THIS IMPLEMENTATION  IS FREE FROM  ANY CLAIMS OF  INFRINGEMENT, AND
// YOU ARE  RESPONSIBLE FOR OBTAINING  ANY RIGHTS YOU MAY  REQUIRE FOR
// YOUR  IMPLEMENTATION.   XILINX  EXPRESSLY  DISCLAIMS  ANY  WARRANTY
// WHATSOEVER  WITH RESPECT  TO  THE ADEQUACY  OF THE  IMPLEMENTATION,
// INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR REPRESENTATIONS THAT
// THIS IMPLEMENTATION  IS FREE  FROM CLAIMS OF  INFRINGEMENT, IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
// 
//////////////////////////////////////////////////////////////////////

#include "rtephyseng_plbw.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xcope.h"

inline xc_status_t xc_rtephyseng_plbw_create(xc_iface_t **iface, void *config_table)
{
    // set up iface
    *iface = (xc_iface_t *) config_table;

#ifdef XC_DEBUG
    RTEPHYSENG_PLBW_Config *_config_table = config_table;

    if (_config_table->xc_create == NULL) {
        print("config_table.xc_create == NULL\r\n");
        exit(1);
    }
#endif

    // does nothing
    return XC_SUCCESS;
}

inline xc_status_t xc_rtephyseng_plbw_release(xc_iface_t **iface) 
{
    // does nothing
    return XC_SUCCESS;
}

inline xc_status_t xc_rtephyseng_plbw_open(xc_iface_t *iface)
{
    // does nothing
    return XC_SUCCESS;
}

inline xc_status_t xc_rtephyseng_plbw_close(xc_iface_t *iface)
{
    // does nothing
    return XC_SUCCESS;
}

inline xc_status_t xc_rtephyseng_plbw_read(xc_iface_t *iface, xc_r_addr_t addr, uint32_t *value)
{
    *value = Xil_In32((uint32_t *) addr);
    return XC_SUCCESS;
}

inline xc_status_t xc_rtephyseng_plbw_write(xc_iface_t *iface, xc_w_addr_t addr, const uint32_t value)
{
    Xil_Out32((uint32_t *) addr, value);
    return XC_SUCCESS;
}

xc_status_t xc_rtephyseng_plbw_getshmem(xc_iface_t *iface, const char *name, void **shmem)
{
    RTEPHYSENG_PLBW_Config *_config_table = (RTEPHYSENG_PLBW_Config *) iface;

    if (strcmp("ESP_Status_Reg", name) == 0) {
        *shmem = (void *) & _config_table->ESP_Status_Reg;
    } else if (strcmp("E1_Template_Acc_Reg", name) == 0) {
        *shmem = (void *) & _config_table->E1_Template_Acc_Reg;
    } else if (strcmp("E1_Filter_Out_Reg", name) == 0) {
        *shmem = (void *) & _config_table->E1_Filter_Out_Reg;
    } else if (strcmp("ESP_Contol_Reg", name) == 0) {
        *shmem = (void *) & _config_table->ESP_Contol_Reg;
    } else if (strcmp("dtoa1_data_regB", name) == 0) {
        *shmem = (void *) & _config_table->dtoa1_data_regB;
    } else if (strcmp("dtoa1_data_regA", name) == 0) {
        *shmem = (void *) & _config_table->dtoa1_data_regA;
    } else if (strcmp("dtoa0_data_regB", name) == 0) {
        *shmem = (void *) & _config_table->dtoa0_data_regB;
    } else if (strcmp("dtoa0_data_regA", name) == 0) {
        *shmem = (void *) & _config_table->dtoa0_data_regA;
    } else if (strcmp("E1_Decimation", name) == 0) {
        *shmem = (void *) & _config_table->E1_Decimation;
    } else if (strcmp("E1_Filter_Length", name) == 0) {
        *shmem = (void *) & _config_table->E1_Filter_Length;
    } else if (strcmp("E1_Template_Size", name) == 0) {
        *shmem = (void *) & _config_table->E1_Template_Size;
    } else if (strcmp("AtoD_DataMem", name) == 0) {
        *shmem = (void *) & _config_table->AtoD_DataMem;
    } else if (strcmp("E1_filter_RAM", name) == 0) {
        *shmem = (void *) & _config_table->E1_filter_RAM;
    } else if (strcmp("E1_Data_RAM", name) == 0) {
        *shmem = (void *) & _config_table->E1_Data_RAM;
    } else if (strcmp("E1_template_RAM", name) == 0) {
        *shmem = (void *) & _config_table->E1_template_RAM;
    }
    else { *shmem = NULL; return XC_FAILURE; }

    return XC_SUCCESS;
}
