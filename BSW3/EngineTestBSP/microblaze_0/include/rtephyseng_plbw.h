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

#ifndef __XL_RTEPHYSENG_PLBW_H__
#define __XL_RTEPHYSENG_PLBW_H__

#include "xbasic_types.h"
#include "xstatus.h"
#include "xcope.h"

typedef struct {
    uint32_t version;
    // Pointers to low-level functions
    xc_status_t (*xc_create)(xc_iface_t **, void *);
    xc_status_t (*xc_release)(xc_iface_t **);
    xc_status_t (*xc_open)(xc_iface_t *);
    xc_status_t (*xc_close)(xc_iface_t *);
    xc_status_t (*xc_read)(xc_iface_t *, xc_r_addr_t, uint32_t *);
    xc_status_t (*xc_write)(xc_iface_t *, xc_w_addr_t, const uint32_t);
    xc_status_t (*xc_get_shmem)(xc_iface_t *, const char *, void **shmem);
    // Optional parameters
    // (empty)
    // Memory map information
    uint32_t ESP_Status_Reg;
    uint32_t ESP_Status_Reg_n_bits;
    uint32_t ESP_Status_Reg_bin_pt;
    // uint32_t ESP_Status_Reg_attr;
    uint32_t E1_Template_Acc_Reg;
    uint32_t E1_Template_Acc_Reg_n_bits;
    uint32_t E1_Template_Acc_Reg_bin_pt;
    // uint32_t E1_Template_Acc_Reg_attr;
    uint32_t E1_Filter_Out_Reg;
    uint32_t E1_Filter_Out_Reg_n_bits;
    uint32_t E1_Filter_Out_Reg_bin_pt;
    // uint32_t E1_Filter_Out_Reg_attr;
    uint32_t ESP_Contol_Reg;
    uint32_t ESP_Contol_Reg_n_bits;
    uint32_t ESP_Contol_Reg_bin_pt;
    // uint32_t ESP_Contol_Reg_attr;
    uint32_t dtoa1_data_regB;
    uint32_t dtoa1_data_regB_n_bits;
    uint32_t dtoa1_data_regB_bin_pt;
    // uint32_t dtoa1_data_regB_attr;
    uint32_t dtoa1_data_regA;
    uint32_t dtoa1_data_regA_n_bits;
    uint32_t dtoa1_data_regA_bin_pt;
    // uint32_t dtoa1_data_regA_attr;
    uint32_t dtoa0_data_regB;
    uint32_t dtoa0_data_regB_n_bits;
    uint32_t dtoa0_data_regB_bin_pt;
    // uint32_t dtoa0_data_regB_attr;
    uint32_t dtoa0_data_regA;
    uint32_t dtoa0_data_regA_n_bits;
    uint32_t dtoa0_data_regA_bin_pt;
    // uint32_t dtoa0_data_regA_attr;
    uint32_t E1_Decimation;
    uint32_t E1_Decimation_n_bits;
    uint32_t E1_Decimation_bin_pt;
    // uint32_t E1_Decimation_attr;
    uint32_t E1_Filter_Length;
    uint32_t E1_Filter_Length_n_bits;
    uint32_t E1_Filter_Length_bin_pt;
    // uint32_t E1_Filter_Length_attr;
    uint32_t E1_Template_Size;
    uint32_t E1_Template_Size_n_bits;
    uint32_t E1_Template_Size_bin_pt;
    // uint32_t E1_Template_Size_attr;
    uint32_t AtoD_DataMem;
    // uint32_t AtoD_DataMem_grant;
    // uint32_t AtoD_DataMem_req;
    uint32_t AtoD_DataMem_n_bits;
    uint32_t AtoD_DataMem_bin_pt;
    uint32_t AtoD_DataMem_depth;
    // uint32_t AtoD_DataMem_attr;
    uint32_t E1_filter_RAM;
    // uint32_t E1_filter_RAM_grant;
    // uint32_t E1_filter_RAM_req;
    uint32_t E1_filter_RAM_n_bits;
    uint32_t E1_filter_RAM_bin_pt;
    uint32_t E1_filter_RAM_depth;
    // uint32_t E1_filter_RAM_attr;
    uint32_t E1_Data_RAM;
    // uint32_t E1_Data_RAM_grant;
    // uint32_t E1_Data_RAM_req;
    uint32_t E1_Data_RAM_n_bits;
    uint32_t E1_Data_RAM_bin_pt;
    uint32_t E1_Data_RAM_depth;
    // uint32_t E1_Data_RAM_attr;
    uint32_t E1_template_RAM;
    // uint32_t E1_template_RAM_grant;
    // uint32_t E1_template_RAM_req;
    uint32_t E1_template_RAM_n_bits;
    uint32_t E1_template_RAM_bin_pt;
    uint32_t E1_template_RAM_depth;
    // uint32_t E1_template_RAM_attr;
    // XPS parameters
    Xuint16  DeviceId;
    uint32_t  BaseAddr;
} RTEPHYSENG_PLBW_Config;

extern RTEPHYSENG_PLBW_Config RTEPHYSENG_PLBW_ConfigTable[];

// forward declaration of low-level functions
xc_status_t xc_rtephyseng_plbw_create(xc_iface_t **iface, void *config_table);
xc_status_t xc_rtephyseng_plbw_release(xc_iface_t **iface) ;
xc_status_t xc_rtephyseng_plbw_open(xc_iface_t *iface);
xc_status_t xc_rtephyseng_plbw_close(xc_iface_t *iface);
xc_status_t xc_rtephyseng_plbw_read(xc_iface_t *iface, xc_r_addr_t addr, uint32_t *value);
xc_status_t xc_rtephyseng_plbw_write(xc_iface_t *iface, xc_w_addr_t addr, const uint32_t value);
xc_status_t xc_rtephyseng_plbw_getshmem(xc_iface_t *iface, const char *name, void **shmem);

#endif

