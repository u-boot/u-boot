/**
 * @file IxPerfProfAcc.h
 *
 * @brief  Header file for the IXP400 Perf Prof component (IxPerfProfAcc)
 *
 * 
 * @par
 * IXP400 SW Release version 2.0
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright 2001-2005, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * SPDX-License-Identifier:	BSD-3-Clause
 * @par
 * -- End of Copyright Notice --
 */

/**
 * @defgroup IxPerfProfAcc IXP400 Performance Profiling (IxPerfProfAcc) API 
 *
 * @brief IXP400 Performance Profiling Utility component Public API. 
 * @li NOTE: Xcycle measurement is not supported in Linux.
 *
 *
 * @{
 */
#ifndef IXPERFPROFACC_H
#define IXPERFPROFACC_H

#include "IxOsal.h"

#ifdef __linux
#include <linux/proc_fs.h>
#endif

/*
 * Section for #define
 */
/**
 * @ingroup IxPerfProfAcc
 *
 * @def IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES
 *
 * @brief This is the maximum number of profiling samples allowed, which can be
 * modified according to the user's discretion
 */
#define IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES	0xFFFF

/**
 * @ingroup IxPerfProfAcc
 *
 * @def IX_PERFPROF_ACC_BUS_PMU_MAX_PECS      
 *
 * @brief This is the maximum number of Programmable Event Counters available. 
 *        This is a hardware specific and fixed value. Do not change.
 *        
 */
#define IX_PERFPROF_ACC_BUS_PMU_MAX_PECS        7

/**  
 * @ingroup IxPerfProfAcc
 *
 * @def IX_PERFPROF_ACC_XCYCLE_MAX_NUM_OF_MEASUREMENTS
 *
 * @brief Max number of measurement allowed. This constant is used when 
 *        creating storage array for Xcycle. When run in continuous mode,  
 *        Xcycle will wrap around and re-use buffer. 
 */
#define IX_PERFPROF_ACC_XCYCLE_MAX_NUM_OF_MEASUREMENTS 600

#ifdef __linux
/**
 * @ingroup IxPerfProfAcc
 *
 * @def IX_PERFPROF_ACC_XSCALE_PMU_SYMBOL_ACCURACY
 *
 * @brief Level of accuracy required for matching the PC Address to
 *        symbol address. This is used when the XScale PMU time/event
 *        sampling functions get the PC address and search for the 
 *        corresponding symbol address.
 */
#define IX_PERFPROF_ACC_XSCALE_PMU_SYMBOL_ACCURACY 0xffff

#endif /*__linux*/

/**  
 * @ingroup IxPerfProfAcc
 *
 * @def IX_PERFPROF_ACC_LOG
 *
 * @brief Mechanism for logging a formatted message for the PerfProfAcc component
 *
 * @param level UINT32 [in] - trace level
 * @param device UINT32 [in] - output device
 * @param str char* [in] - format string, similar to printf().
 * @param a UINT32 [in] - first argument to display
 * @param b UINT32 [in] - second argument to display
 * @param c UINT32 [in] - third argument to display
 * @param d UINT32 [in] - fourth argument to display
 * @param e UINT32 [in] - fifth argument to display
 * @param f UINT32 [in] - sixth argument to display
 *
 * @return none
 */
#ifndef NDEBUG
#define IX_PERFPROF_ACC_LOG(level, device, str, a, b, c, d, e, f)\
             (ixOsalLog (level, device, str, a, b, c, d, e, f))
#else /*do nothing*/
#define IX_PERFPROF_ACC_LOG(level, device, str, a, b, c, d, e, f) 
#endif /*ifdef NDEBUG */

/*
 * Section for struct
 */

/**
 * @brief contains summary of samples taken 
 * 
 * Structure contains all details of each program counter value - frequency 
 * that PC occurs 
 */
typedef struct
{
	UINT32 programCounter;  /**<the program counter value of the sample*/
	UINT32 freq;		/**<the frequency of the occurence of the sample*/
} IxPerfProfAccXscalePmuSamplePcProfile;

/**
 * @brief contains results of a counter
 *
 * Structure contains the results of a counter, which are split into the lower 
 * and upper 32 bits of the final count
 */
typedef struct
{
    UINT32 lower32BitsEventCount; /**<lower 32bits value of the event counter*/        
    UINT32 upper32BitsEventCount; /**<upper 32bits value of the event counter*/
}   IxPerfProfAccXscalePmuEvtCnt;

/**
 * @brief contains results of counters and their overflow 
 * 
 * Structure contains all values of counters and associated overflows.  The 
 * specific event and clock counters are determined by the user
 */
typedef struct
{
    UINT32 clk_value;           /**<current value of clock counter*/                 
    UINT32 clk_samples;        /**<number of clock counter overflows*/
    UINT32 event1_value;        /**<current value of event 1 counter*/    
    UINT32 event1_samples;     /**<number of event 1 counter overflows*/
    UINT32 event2_value;        /**<current value of event 2 counter*/
    UINT32 event2_samples;     /**<number of event 2 counter overflows*/
    UINT32 event3_value;        /**<current value of event 3 counter*/
    UINT32 event3_samples;     /**<number of event 3 counter overflows*/
    UINT32 event4_value;        /**<current value of event 4 counter*/
    UINT32 event4_samples;     /**<number of event 4 counter overflows*/
} IxPerfProfAccXscalePmuResults;

/** 
 * 
 * @brief Results obtained from Xcycle run 
 */ 
typedef struct 
{
    float maxIdlePercentage; 		/**<maximum percentage of Idle cycles*/
    float minIdlePercentage; 		/**<minimum percentage of Idle cycles*/
    float aveIdlePercentage;		/**<average percentage of Idle cycles*/
    UINT32 totalMeasurements;		/**<total number of measurement made */
} IxPerfProfAccXcycleResults; 

/**
 *
 * @brief Results obtained from running the Bus Pmu component. The results
 *        are obtained when the get functions is called.
 *        
 */
typedef struct
{
    UINT32 statsToGetLower27Bit[IX_PERFPROF_ACC_BUS_PMU_MAX_PECS]; /**<Lower 27 Bit of counter value */
    UINT32 statsToGetUpper32Bit[IX_PERFPROF_ACC_BUS_PMU_MAX_PECS]; /**<Upper 32 Bit of counter value */
} IxPerfProfAccBusPmuResults;

/*
 * Section for enum
 */

/**
 * @ingroup IxPerfProfAcc
 * 
 * @enum IxPerfProfAccBusPmuEventCounters1
 *
 * @brief Type of bus pmu events supported on PEC 1.
 *
 * Lists all bus pmu events.  
 */
typedef enum
{
	IX_PERFPROF_ACC_BUS_PMU_PEC1_NORTH_NPEA_GRANT_SELECT = 1, /**< Select North NPEA grant on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_NORTH_NPEB_GRANT_SELECT, /**< Select North NPEB grant on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_NORTH_NPEC_GRANT_SELECT, /**< Select North NPEC grant on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_NORTH_BUS_IDLE_SELECT, /**< Select North bus idle on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_NORTH_NPEA_REQ_SELECT, /**< Select North NPEA req on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_NORTH_NPEB_REQ_SELECT, /**< Select North NPEB req on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_NORTH_NPEC_REQ_SELECT, /**< Select North NPEC req on PEC1*/

	IX_PERFPROF_ACC_BUS_PMU_PEC1_SOUTH_GSKT_GRANT_SELECT, /**< Select south gasket grant on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_SOUTH_ABB_GRANT_SELECT, /**< Select south abb grant on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_SOUTH_PCI_GRANT_SELECT, /**< Select south pci grant on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_SOUTH_APB_GRANT_SELECT, /**< Select south apb grant on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_SOUTH_GSKT_REQ_SELECT, /**< Select south gasket request on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_SOUTH_ABB_REQ_SELECT, /**< Select south abb request on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_SOUTH_PCI_REQ_SELECT, /**< Select south pci request on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_SOUTH_APB_REQ_SELECT, /**< Select south apb request on PEC1*/

	IX_PERFPROF_ACC_BUS_PMU_PEC1_SDR_0_HIT_SELECT, /**< Select sdram0 hit on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_SDR_1_HIT_SELECT, /**< Select sdram1 hit on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_SDR_2_HIT_SELECT, /**< Select sdram2 hit on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_SDR_3_HIT_SELECT, /**< Select sdram3 hit on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_SDR_4_MISS_SELECT, /**< Select sdram4 miss on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_SDR_5_MISS_SELECT, /**< Select sdram5 miss on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_SDR_6_MISS_SELECT, /**< Select sdram6 miss on PEC1*/
	IX_PERFPROF_ACC_BUS_PMU_PEC1_SDR_7_MISS_SELECT /**< Select sdram7 miss on PEC1*/
} IxPerfProfAccBusPmuEventCounters1;

/**
 * @ingroup IxPerfProfAcc
 *
 * @enum IxPerfProfAccBusPmuEventCounters2
 *
 * @brief Type of bus pmu events supported on PEC 2.
 *
 * Lists all bus pmu events.
 */
typedef enum
{
	IX_PERFPROF_ACC_BUS_PMU_PEC2_NORTH_NPEA_XFER_SELECT = 24, /**< Select North NPEA transfer on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_NORTH_NPEB_XFER_SELECT, /**< Select North NPEB transfer on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_NORTH_NPEC_XFER_SELECT, /**< Select North NPEC transfer on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_NORTH_BUS_WRITE_SELECT, /**< Select North bus write on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_NORTH_NPEA_OWN_SELECT, /**< Select North NPEA own on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_NORTH_NPEB_OWN_SELECT, /**< Select North NPEB own on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_NORTH_NPEC_OWN_SELECT, /**< Select North NPEC own on PEC2*/

	IX_PERFPROF_ACC_BUS_PMU_PEC2_SOUTH_GSKT_XFER_SELECT, /**< Select South gasket transfer on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_SOUTH_ABB_XFER_SELECT, /**< Select South abb transfer on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_SOUTH_PCI_XFER_SELECT, /**< Select South pci transfer on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_SOUTH_APB_XFER_SELECT, /**< Select South apb transfer on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_SOUTH_GSKT_OWN_SELECT, /**< Select South gasket own on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_SOUTH_ABB_OWN_SELECT, /**< Select South abb own on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_SOUTH_PCI_OWN_SELECT, /**< Select South pci own on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_SOUTH_APB_OWN_SELECT, /**< Select South apb own transfer on PEC2*/

	IX_PERFPROF_ACC_BUS_PMU_PEC2_SDR_1_HIT_SELECT, /**< Select sdram1 hit on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_SDR_2_HIT_SELECT, /**< Select sdram2 hit on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_SDR_3_HIT_SELECT, /**< Select sdram3 hit on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_SDR_4_HIT_SELECT, /**< Select sdram4 hit on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_SDR_5_MISS_SELECT, /**< Select sdram5 miss on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_SDR_6_MISS_SELECT, /**< Select sdram6 miss on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_SDR_7_MISS_SELECT, /**< Select sdram7 miss on PEC2*/
	IX_PERFPROF_ACC_BUS_PMU_PEC2_SDR_0_MISS_SELECT /**< Select sdram0 miss on PEC2*/
} IxPerfProfAccBusPmuEventCounters2;

/**
 * @ingroup IxPerfProfAcc
 *
 * @enum IxPerfProfAccBusPmuEventCounters3
 *
 * @brief Type of bus pmu events supported on PEC 3.
 *
 * Lists all bus pmu events.
 */
typedef enum
{
	IX_PERFPROF_ACC_BUS_PMU_PEC3_NORTH_NPEA_RETRY_SELECT = 47, /**< Select north NPEA retry on PEC3*/
	IX_PERFPROF_ACC_BUS_PMU_PEC3_NORTH_NPEB_RETRY_SELECT, /**< Select north NPEB retry on PEC3*/
	IX_PERFPROF_ACC_BUS_PMU_PEC3_NORTH_NPEC_RETRY_SELECT, /**< Select north NPEC retry on PEC3*/
	IX_PERFPROF_ACC_BUS_PMU_PEC3_NORTH_BUS_READ_SELECT, /**< Select north bus read on PEC3*/
	IX_PERFPROF_ACC_BUS_PMU_PEC3_NORTH_NPEA_WRITE_SELECT, /**< Select north NPEA write on PEC3*/
	IX_PERFPROF_ACC_BUS_PMU_PEC3_NORTH_NPEB_WRITE_SELECT, /**< Select north NPEB write on PEC3*/
	IX_PERFPROF_ACC_BUS_PMU_PEC3_NORTH_NPEC_WRITE_SELECT, /**< Select north NPEC wirte on PEC3*/

	IX_PERFPROF_ACC_BUS_PMU_PEC3_SOUTH_GSKT_RETRY_SELECT, /**< Select south gasket retry on PEC3*/
	IX_PERFPROF_ACC_BUS_PMU_PEC3_SOUTH_ABB_RETRY_SELECT, /**< Select south abb retry on PEC3*/
	IX_PERFPROF_ACC_BUS_PMU_PEC3_SOUTH_PCI_RETRY_SELECT, /**< Select south pci retry on PEC3*/
	IX_PERFPROF_ACC_BUS_PMU_PEC3_SOUTH_APB_RETRY_SELECT, /**< Select south apb retry on PEC3*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC3_SOUTH_GSKT_WRITE_SELECT, /**< Select south gasket write on PEC3*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC3_SOUTH_ABB_WRITE_SELECT, /**< Select south abb write on PEC3*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC3_SOUTH_PCI_WRITE_SELECT, /**< Select south pci write on PEC3*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC3_SOUTH_APB_WRITE_SELECT, /**< Select south apb write on PEC3*/

 	IX_PERFPROF_ACC_BUS_PMU_PEC3_SDR_2_HIT_SELECT, /**< Select sdram2 hit on PEC3*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC3_SDR_3_HIT_SELECT, /**< Select sdram3 hit on PEC3*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC3_SDR_4_HIT_SELECT, /**< Select sdram4 hit on PEC3*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC3_SDR_5_HIT_SELECT, /**< Select sdram5 hit on PEC3*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC3_SDR_6_MISS_SELECT, /**< Select sdram6 miss on PEC3*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC3_SDR_7_MISS_SELECT, /**< Select sdram7 miss on PEC3*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC3_SDR_0_MISS_SELECT, /**< Select sdram0 miss on PEC3*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC3_SDR_1_MISS_SELECT /**< Select sdram1 miss on PEC3*/
} IxPerfProfAccBusPmuEventCounters3;

/**
 * @ingroup IxPerfProfAcc
 *
 * @enum IxPerfProfAccBusPmuEventCounters4
 *
 * @brief Type of bus pmu events supported on PEC 4.
 *
 * Lists all bus pmu events.
 */
typedef enum
{
 	IX_PERFPROF_ACC_BUS_PMU_PEC4_SOUTH_PCI_SPLIT_SELECT = 70, /**< Select south pci split on PEC4*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC4_SOUTH_EXP_SPLIT_SELECT, /**< Select south expansion split on PEC4*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC4_SOUTH_APB_GRANT_SELECT, /**< Select south apb grant on PEC4*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC4_SOUTH_APB_XFER_SELECT, /**< Select south apb transfer on PEC4*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC4_SOUTH_GSKT_READ_SELECT, /**< Select south gasket read on PEC4*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC4_SOUTH_ABB_READ_SELECT, /**< Select south abb read on PEC4*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC4_SOUTH_PCI_READ_SELECT, /**< Select south pci read on PEC4*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC4_SOUTH_APB_READ_SELECT, /**< Select south apb read on PEC4*/

 	IX_PERFPROF_ACC_BUS_PMU_PEC4_NORTH_ABB_SPLIT_SELECT, /**< Select north abb split on PEC4*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC4_NORTH_NPEA_REQ_SELECT, /**< Select north NPEA req on PEC4*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC4_NORTH_NPEA_READ_SELECT, /**< Select north NPEA read on PEC4*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC4_NORTH_NPEB_READ_SELECT, /**< Select north NPEB read on PEC4*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC4_NORTH_NPEC_READ_SELECT, /**< Select north NPEC read on PEC4*/

 	IX_PERFPROF_ACC_BUS_PMU_PEC4_SDR_3_HIT_SELECT, /**< Select sdram3 hit on PEC4*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC4_SDR_4_HIT_SELECT, /**< Select sdram4 hit on PEC4*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC4_SDR_5_HIT_SELECT, /**< Select sdram5 hit on PEC4*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC4_SDR_6_HIT_SELECT, /**< Select sdram6 hit on PEC4*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC4_SDR_7_MISS_SELECT, /**< Select sdram7 miss on PEC4*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC4_SDR_0_MISS_SELECT, /**< Select sdram0 miss on PEC4*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC4_SDR_1_MISS_SELECT, /**< Select sdram1 miss on PEC4*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC4_SDR_2_MISS_SELECT /**< Select sdram2 miss on PEC4*/
} IxPerfProfAccBusPmuEventCounters4;

/**
 * @ingroup IxPerfProfAcc
 *
 * @enum IxPerfProfAccBusPmuEventCounters5
 *
 * @brief Type of bus pmu events supported on PEC 5.
 *
 * Lists all bus pmu events.
 */
typedef enum
{
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_SOUTH_ABB_GRANT_SELECT = 91, /**< Select south abb grant on PEC5*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_SOUTH_ABB_XFER_SELECT, /**< Select south abb transfer on PEC5*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_SOUTH_ABB_RETRY_SELECT, /**< Select south abb retry on PEC5*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_SOUTH_EXP_SPLIT_SELECT, /**< Select south expansion split on PEC5*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_SOUTH_ABB_REQ_SELECT, /**< Select south abb request on PEC5*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_SOUTH_ABB_OWN_SELECT, /**< Select south abb own on PEC5*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_SOUTH_BUS_IDLE_SELECT, /**< Select south bus idle on PEC5*/

 	IX_PERFPROF_ACC_BUS_PMU_PEC5_NORTH_NPEB_GRANT_SELECT, /**< Select north NPEB grant on PEC5*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_NORTH_NPEB_XFER_SELECT, /**< Select north NPEB transfer on PEC5*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_NORTH_NPEB_RETRY_SELECT, /**< Select north NPEB retry on PEC5*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_NORTH_NPEB_REQ_SELECT, /**< Select north NPEB request on PEC5*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_NORTH_NPEB_OWN_SELECT, /**< Select north NPEB own on PEC5*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_NORTH_NPEB_WRITE_SELECT, /**< Select north NPEB write on PEC5*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_NORTH_NPEB_READ_SELECT, /**< Select north NPEB read on PEC5*/

 	IX_PERFPROF_ACC_BUS_PMU_PEC5_SDR_4_HIT_SELECT, /**< Select north sdram4 hit on PEC5*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_SDR_5_HIT_SELECT, /**< Select north sdram5 hit on PEC5*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_SDR_6_HIT_SELECT, /**< Select north sdram6 hit on PEC5*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_SDR_7_HIT_SELECT, /**< Select north sdram7 hit on PEC5*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_SDR_0_MISS_SELECT, /**< Select north sdram0 miss on PEC5*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_SDR_1_MISS_SELECT, /**< Select north sdram1 miss on PEC5*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_SDR_2_MISS_SELECT, /**< Select north sdram2 miss on PEC5*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC5_SDR_3_MISS_SELECT /**< Select north sdram3 miss on PEC5*/
} IxPerfProfAccBusPmuEventCounters5;

/**
 * @ingroup IxPerfProfAcc
 *
 * @enum IxPerfProfAccBusPmuEventCounters6
 *
 * @brief Type of bus pmu events supported on PEC 6.
 *
 * Lists all bus pmu events.
 */
typedef enum
{
	IX_PERFPROF_ACC_BUS_PMU_PEC6_SOUTH_PCI_GRANT_SELECT = 113, /**< Select south pci grant on PEC6*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC6_SOUTH_PCI_XFER_SELECT, /**< Select south pci transfer on PEC6*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC6_SOUTH_PCI_RETRY_SELECT, /**< Select south pci retry on PEC6*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC6_SOUTH_PCI_SPLIT_SELECT, /**< Select south pci split on PEC6*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC6_SOUTH_PCI_REQ_SELECT, /**< Select south pci request on PEC6*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC6_SOUTH_PCI_OWN_SELECT, /**< Select south pci own on PEC6*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC6_SOUTH_BUS_WRITE_SELECT, /**< Select south pci write on PEC6*/

 	IX_PERFPROF_ACC_BUS_PMU_PEC6_NORTH_NPEC_GRANT_SELECT, /**< Select north NPEC grant on PEC6*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC6_NORTH_NPEC_XFER_SELECT, /**< Select north NPEC transfer on PEC6*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC6_NORTH_NPEC_RETRY_SELECT, /**< Select north NPEC retry on PEC6*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC6_NORTH_NPEC_REQ_SELECT, /**< Select north NPEC request on PEC6*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC6_NORTH_NPEC_OWN_SELECT, /**< Select north NPEC own on PEC6*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC6_NORTH_NPEB_WRITE_SELECT, /**< Select north NPEB write on PEC6*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC6_NORTH_NPEC_READ_SELECT, /**< Select north NPEC read on PEC6*/

 	IX_PERFPROF_ACC_BUS_PMU_PEC6_SDR_5_HIT_SELECT, /**< Select sdram5 hit on PEC6*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC6_SDR_6_HIT_SELECT, /**< Select sdram6 hit on PEC6*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC6_SDR_7_HIT_SELECT, /**< Select sdram7 hit on PEC6*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC6_SDR_0_HIT_SELECT, /**< Select sdram0 hit on PEC6*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC6_SDR_1_MISS_SELECT, /**< Select sdram1 miss on PEC6*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC6_SDR_2_MISS_SELECT, /**< Select sdram2 miss on PEC6*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC6_SDR_3_MISS_SELECT, /**< Select sdram3 miss on PEC6*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC6_SDR_4_MISS_SELECT /**< Select sdram4 miss on PEC6*/
} IxPerfProfAccBusPmuEventCounters6;

/**
 * @ingroup IxPerfProfAcc
 *
 * @enum IxPerfProfAccBusPmuEventCounters7
 *
 * @brief Type of bus pmu events supported on PEC 7.
 *
 * Lists all bus pmu events.
 */
typedef enum
{
 	IX_PERFPROF_ACC_BUS_PMU_PEC7_SOUTH_APB_RETRY_SELECT = 135, /**< Select south apb retry on PEC7*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC7_SOUTH_APB_REQ_SELECT, /**< Select south apb request on PEC7*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC7_SOUTH_APB_OWN_SELECT, /**< Select south apb own on PEC7*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC7_SOUTH_BUS_READ_SELECT, /**< Select south bus read on PEC7*/
 	IX_PERFPROF_ACC_BUS_PMU_PEC7_CYCLE_COUNT_SELECT /**< Select cycle count on PEC7*/
} IxPerfProfAccBusPmuEventCounters7;

/** 
 * @ingroup IxPerfProfAcc 
 * 
 * @enum IxPerfProfAccXscalePmuEvent 
 * 
 * @brief Type of xscale pmu events supported 
 * 
 * Lists all xscale pmu events.  The maximum is a default value that the user 
 * should not exceed. 
 */ 
typedef enum 
{
    IX_PERFPROF_ACC_XSCALE_PMU_EVENT_CACHE_MISS=0,      /**< cache miss*/ 
    IX_PERFPROF_ACC_XSCALE_PMU_EVENT_CACHE_INSTRUCTION,/**< cache instruction*/ 
    IX_PERFPROF_ACC_XSCALE_PMU_EVENT_STALL,     /**< event stall*/ 
    IX_PERFPROF_ACC_XSCALE_PMU_EVENT_INST_TLB_MISS, /**< instruction tlb miss*/ 
    IX_PERFPROF_ACC_XSCALE_PMU_EVENT_DATA_TLB_MISS, /**< data tlb miss*/ 
    IX_PERFPROF_ACC_XSCALE_PMU_EVENT_BRANCH_EXEC,   /**< branch executed*/ 
    IX_PERFPROF_ACC_XSCALE_PMU_EVENT_BRANCH_MISPREDICT, /**<branch mispredict*/ 
    IX_PERFPROF_ACC_XSCALE_PMU_EVENT_INST_EXEC, /**< instruction executed*/ 
    IX_PERFPROF_ACC_XSCALE_PMU_EVENT_FULL_EVERYCYCLE,   /**< 
                                                         *Stall - data cache 
                                                         *buffers are full. 
							 							 *This event occurs 
							 							 *every cycle where 
							 							 *condition present 
							 							 */ 
    IX_PERFPROF_ACC_XSCALE_PMU_EVENT_ONCE,    /**< 
                                               *Stall - data cache buffers are 
					       					   *full.This event occurs once 
					       					   *for each contiguous sequence 
					       					   */ 
    IX_PERFPROF_ACC_XSCALE_PMU_EVENT_DATA_CACHE_ACCESS, /**< data cache access*/ 
    IX_PERFPROF_ACC_XSCALE_PMU_EVENT_DATA_CACHE_MISS,   /**< data cache miss*/ 
    IX_PERFPROF_ACC_XSCALE_PMU_EVENT_DATA_CACHE_WRITEBACK,  /**<data cache 
                                                             *writeback 
						             						 */ 
    IX_PERFPROF_ACC_XSCALE_PMU_EVENT_SW_CHANGE_PC,  /**< sw change pc*/ 
    IX_PERFPROF_ACC_XSCALE_PMU_EVENT_MAX    /**< max value*/ 
} IxPerfProfAccXscalePmuEvent;

/**
 * @ingroup IxPerfProfAcc
 *
 * @enum IxPerfProfAccStatus
 *
 * @brief Invalid Status Definitions  
 *
 * These status will be used by the APIs to return to the user.
 */
typedef enum
{
	IX_PERFPROF_ACC_STATUS_SUCCESS = IX_SUCCESS,    /**< success*/
	IX_PERFPROF_ACC_STATUS_FAIL = IX_FAIL,          /**< fail*/
	IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS,/**<another utility in 
													 *progress
													 */ 
	IX_PERFPROF_ACC_STATUS_XCYCLE_MEASUREMENT_IN_PROGRESS, /**<measurement in
															*progress
															*/
	IX_PERFPROF_ACC_STATUS_XCYCLE_NO_BASELINE, /**<no baseline yet*/	 
	IX_PERFPROF_ACC_STATUS_XCYCLE_MEASUREMENT_REQUEST_OUT_OF_RANGE, /**< 
															* Measurement chosen 
															* is out of range
															*/
	IX_PERFPROF_ACC_STATUS_XCYCLE_PRIORITY_SET_FAIL,  /**<
	                                                     * Cannot set 
														 * task priority
	                                                     */
	IX_PERFPROF_ACC_STATUS_XCYCLE_THREAD_CREATE_FAIL, /**< 
	                                                     * Fail create thread
	                                                     */
	IX_PERFPROF_ACC_STATUS_XCYCLE_PRIORITY_RESTORE_FAIL,  /**<
	                                                         *cannot restore
	                                                         *priority
	                                                         */
	IX_PERFPROF_ACC_STATUS_XCYCLE_MEASUREMENT_NOT_RUNNING, /**< xcycle not running*/
   	IX_PERFPROF_ACC_STATUS_XSCALE_PMU_NUM_INVALID, /**< invalid number 
   	                                                *entered
   	                                                */
	IX_PERFPROF_ACC_STATUS_XSCALE_PMU_EVENT_INVALID, /**< invalid pmu event*/
	IX_PERFPROF_ACC_STATUS_XSCALE_PMU_START_NOT_CALLED, /**<a start process
							     						 *was not called 
							     						 *before attempting
							     						 *a stop or results
							     						 *get
							     						 */
	IX_PERFPROF_ACC_STATUS_BUS_PMU_MODE_ERROR,  /**< invalid mode*/
	IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC1_ERROR,	/**< invalid pec1 entered*/
	IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC2_ERROR,	/**< invalid pec2 entered*/
	IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC3_ERROR,	/**< invalid pec3 entered*/
	IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC4_ERROR,	/**< invalid pec4 entered*/
	IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC5_ERROR,	/**< invalid pec5 entered*/
	IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC6_ERROR,	/**< invalid pec6 entered*/
	IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC7_ERROR,	/**< invalid pec7 entered*/
	IX_PERFPROF_ACC_STATUS_BUS_PMU_START_NOT_CALLED, /**<a start process
							     					 *was not called 
							     					 *before attempting
							     					 *a stop 
													 */
        IX_PERFPROF_ACC_STATUS_COMPONENT_NOT_SUPPORTED /**<Device or OS does not support component*/
} IxPerfProfAccStatus;

/**
 * @ingroup IxPerfProfAcc 
 * 
 * @enum IxPerfProfAccBusPmuMode
 *
 * @brief State selection of counters.
 *
 * These states will be used to determine the counters whose values are to be
 * read.
 */
typedef enum
{
        IX_PERFPROF_ACC_BUS_PMU_MODE_HALT=0,   /**< halt state*/
        IX_PERFPROF_ACC_BUS_PMU_MODE_SOUTH,    /**< south state*/
        IX_PERFPROF_ACC_BUS_PMU_MODE_NORTH,    /**< north state*/
        IX_PERFPROF_ACC_BUS_PMU_MODE_SDRAM    /**< SDRAM state*/
} IxPerfProfAccBusPmuMode;

/*
 * Section for prototypes interface functions
 */

/**
 * @ingroup IxPerfProfAcc
 *
 * @fn ixPerfProfAccXscalePmuEventCountStart(
   		BOOL clkCntDiv,
   		UINT32 numEvents,
  	 	IxPerfProfAccXscalePmuEvent pmuEvent1,
   		IxPerfProfAccXscalePmuEvent pmuEvent2,
   		IxPerfProfAccXscalePmuEvent pmuEvent3,
   		IxPerfProfAccXscalePmuEvent pmuEvent4 )
 *
 * @brief This API will start the clock and event counting
 *
 * @param	clkCntDiv BOOL [in] - enables/disables the clock divider. When 
 *			true, the divider is enabled and the clock count will be incremented
 *          by one at each 64th processor clock cycle.  When false, the divider
 *			is disabled and the clock count will be incremented at every 
 *			processor clock cycle.
 * @param 	numEvents UINT32 [in] - the number of PMU events that are to be 
 *	  		monitored as specified by the user. For clock counting only, this
 *	  	 	is set to zero.
 * @param 	pmuEvent1 @ref IxPerfProfAccXscalePmuEvent [in] - the specific PMU 
 *    		event to be monitored by counter 1
 * @param 	pmuEvent2 @ref IxPerfProfAccXscalePmuEvent [in] - the specific PMU 
 *    		event to be monitored by counter 2 
 * @param	pmuEvent3 @ref IxPerfProfAccXscalePmuEvent [in] - the specific PMU 
 *    		event to be monitored by counter 3
 * @param 	pmuEvent4 @ref IxPerfProfAccXscalePmuEvent [in] - the specific PMU 
 *    		event to be monitored by counter 4
 *
 * This API will start the clock and xscale PMU event counting.  Up to 
 * 4 events can be monitored simultaneously. This API has to be called before
 * ixPerfProfAccXscalePmuEventCountStop can be called.
 *
 * @return 
 *	- IX_PERFPROF_ACC_STATUS_SUCCESS if clock and events counting are 
 *        started successfully
 *	- IX_PERFPROF_ACC_STATUS_FAIL if unable to start the counting
 *	- IX_PERFPROF_ACC_STATUS_XSCALE_PMU_NUM_INVALID if the number of events 
 *        specified is out of the valid range
 *	- IX_PERFPROF_ACC_STATUS_XSCALE_PMU_EVENT_INVALID if the value of the PMU 
 * 	  event specified does not exist
 * 	- IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS - another utility is 
 * 	  running 
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */
PUBLIC IxPerfProfAccStatus
ixPerfProfAccXscalePmuEventCountStart(
	BOOL clkCntDiv,
	UINT32 numEvents,
	IxPerfProfAccXscalePmuEvent pmuEvent1,
	IxPerfProfAccXscalePmuEvent pmuEvent2,
	IxPerfProfAccXscalePmuEvent pmuEvent3,
	IxPerfProfAccXscalePmuEvent pmuEvent4 );

/**
 * @ingroup IxPerfProfAcc
 *
 * @fn ixPerfProfAccXscalePmuEventCountStop (
   IxPerfProfAccXscalePmuResults *eventCountStopResults) 
 *
 * @brief This API will stop the clock and event counting
 *
 * @param *eventCountStopResults @ref IxPerfProfAccXscalePmuResults [out] - pointer 
 *		  to struct containing results of counters and their overflow. It is the 
 * 		  users's responsibility to allocate the memory for this pointer. 
 *
 * This API will stop the clock and xscale PMU events that are being counted.
 * The results of the clock and events count will be stored in the pointer 
 * allocated by the user. It can only be called once 
 * IxPerfProfAccEventCountStart has been called. 
 *
 * @return 
 *      - IX_PERFPROF_ACC_STATUS_SUCCESS if clock and events counting are 
 *        stopped successfully
 *      - IX_PERFPROF_ACC_STATUS_XSCALE_PMU_START_NOT_CALLED if
 *        ixPerfProfAccXscalePmuEventCountStart is not called first.
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */

PUBLIC IxPerfProfAccStatus 
ixPerfProfAccXscalePmuEventCountStop(
   IxPerfProfAccXscalePmuResults *eventCountStopResults); 

/**
 * @ingroup IxPerfProfAcc
 *
 * @fn ixPerfProfAccXscalePmuTimeSampStart(
   UINT32 samplingRate,
   BOOL clkCntDiv) 
 *
 * @brief Starts the time based sampling
 *
 * @param	samplingRate UINT32 [in] - sampling rate is the number of
 *       	clock counts before a counter overflow interrupt is generated,
 *        	at which, a sample is taken; the rate specified cannot be greater
 *          than the counter size of 32bits or set to zero.
 * @param 	clkCntDiv BOOL [in] - enables/disables the clock divider. When 
 *		true, the divider is enabled and the clock count will be incremented
 *          by one at each 64th processor clock cycle.  When false, the divider
 *			is disabled and the clock count will be incremented at every 
 *			processor clock cycle. 
 *
 * This API starts the time based sampling to determine the frequency with 
 * which lines of code are being executed.  Sampling is done at the rate 
 * specified by the user.  At each sample,the value of the program counter
 * is determined.  Each of these occurrences are recorded to determine the 
 * frequency with which the Xscale code is being executed. This API has to be 
 * called before ixPerfProfAccXscalePmuTimeSampStop can be called.
 *
 * @return
 *	- IX_PERFPROF_ACC_STATUS_SUCCESS if time based sampling is started 
 *        successfully
 *	- IX_PERFPROF_ACC_STATUS_FAIL if unable to start the sampling
 * 	- IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS - another utility is 
 *	  running 
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */
PUBLIC IxPerfProfAccStatus 
ixPerfProfAccXscalePmuTimeSampStart(
	UINT32 samplingRate, 
	BOOL clkCntDiv);

/**
 * @ingroup IxPerfProfAcc
 *
 * @fn ixPerfProfAccXscalePmuTimeSampStop(
   IxPerfProfAccXscalePmuEvtCnt *clkCount,
   IxPerfProfAccXscalePmuSamplePcProfile *timeProfile)
 *
 * @brief Stops the time based sampling
 *
 * @param	*clkCount @ref IxPerfProfAccXscalePmuEvtCnt [out]  - pointer to the 
 *			struct containing the final clock count and its overflow.  It is the
 *			user's responsibility to allocate the memory for this pointer.
 * @param 	*timeProfile @ref IxPerfProfAccXscalePmuSamplePcProfile [out] -
 *          pointer to the array of profiles for each program counter value;
 *          the user should set the size of the array to 
 *			IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES.  It is the user's 
 * 			responsibility to allocate the memory for this pointer.
 *
 * This API stops the time based sampling.  The results are stored in the 
 * pointers allocated by the user.  It can only be called once
 * ixPerfProfAccXscalePmuTimeSampStart has been called.
 *
 * @return
 *	- IX_PERFPROF_ACC_STATUS_SUCCESS if time based sampling is stopped 
 *        successfully
 *	- IX_PERFPROF_ACC_STATUS_XSCALE_PMU_START_NOT_CALLED if
 *	  ixPerfProfAccXscalePmuTimeSampStart not called first
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */
PUBLIC IxPerfProfAccStatus
ixPerfProfAccXscalePmuTimeSampStop(
	IxPerfProfAccXscalePmuEvtCnt *clkCount,
	IxPerfProfAccXscalePmuSamplePcProfile *timeProfile);

/**
 * @ingroup IxPerfProfAcc
 *
 * @fn ixPerfProfAccXscalePmuEventSampStart(
   UINT32 numEvents,
   IxPerfProfAccXscalePmuEvent pmuEvent1,
   UINT32 eventRate1,
   IxPerfProfAccXscalePmuEvent pmuEvent2,
   UINT32 eventRate2,
   IxPerfProfAccXscalePmuEvent pmuEvent3,
   UINT32 eventRate3,
   IxPerfProfAccXscalePmuEvent pmuEvent4,
   UINT32 eventRate4)
 *
 * @brief Starts the event based sampling
 *
 * @param	numEvents UINT32 [in] - the number of PMU events that are 
 *	  		to be monitored as specified by the user. The value should be
 *          between 1-4 events at a time.
 * @param 	pmuEvent1 @ref IxPerfProfAccXscalePmuEvent [in] - the specific PMU 
 *    		event to be monitored by counter 1
 * @param 	eventRate1 UINT32 [in] - sampling rate of counter 1. The rate is 
 *			the number of events before a sample taken.  If 0 is specified, the
 *			the full counter value (0xFFFFFFFF) is used. The rate must not be 
 *			greater than the full counter value.
 * @param 	pmuEvent2 @ref IxPerfProfAccXscalePmuEvent [in] - the specific PMU 
 *    		event to be monitored by counter 2 
 * @param 	eventRate2 UINT32 [in] -  sampling rate of counter 2. The rate is 
 *			the number of events before a sample taken. If 0 is specified, the 
 *			full counter value (0xFFFFFFFF) is used. The rate must not be 
 *          greater than the full counter value.
 * @param 	pmuEvent3 @ref IxPerfProfAccXscalePmuEvent [in] - the specific PMU 
 *    		event to be monitored by counter 3
 * @param 	eventRate3 UINT32 [in] -  sampling rate of counter 3. The rate is 
 *			the number of events before a sample taken.  If 0 is specified, the 
 *			full counter value (0xFFFFFFFF) is used.  The rate must not be 
 *          greater than the full counter value.
 * @param 	pmuEvent4 @ref IxPerfProfAccXscalePmuEvent [in] - the specific PMU 
 *    		event to be monitored by counter 4
 * @param 	eventRate4 UINT32 [in] -  sampling rate of counter 4. The rate is 
 *			the number of events before a sample taken.  If 0 is specified, the 
 *			full counter value (0xFFFFFFFF) is used. The rate must not be 
 *          greater than the full counter value.
 *
 * Starts the event based sampling to determine the frequency with 
 * which events are being executed.  The sampling rate is the number of events,
 * as specified by the user,  before a counter overflow interrupt is 
 * generated.  A sample is taken at each counter overflow interrupt.  At each
 * sample,the value of the program counter determines the corresponding 
 * location in the code.  Each of these occurrences are recorded to determine 
 * the frequency with which the Xscale code in each event is executed. This API
 * has to be called before ixPerfProfAccXscalePmuEventSampStop can be called.
 *
 * @return
 *	- IX_PERFPROF_ACC_STATUS_SUCCESS if event based sampling is started 
 *	  successfully
 *	- IX_PERFPROF_ACC_STATUS_FAIL if unable to start the sampling
 *	- IX_PERFPROF_ACC_STATUS_XSCALE_PMU_NUM_INVALID if the number of events 
 *        specified is out of the valid range
 *	- IX_PERFPROF_ACC_STATUS_XSCALE_PMU_EVENT_INVALID if the value of the
 *        PMU event specified does not exist
 *      - IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS - another utility is
 *        running 
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */
PUBLIC IxPerfProfAccStatus
ixPerfProfAccXscalePmuEventSampStart(
	UINT32 numEvents,
	IxPerfProfAccXscalePmuEvent pmuEvent1,
	UINT32 eventRate1,
    IxPerfProfAccXscalePmuEvent pmuEvent2,
    UINT32 eventRate2,
    IxPerfProfAccXscalePmuEvent pmuEvent3,
    UINT32 eventRate3,
    IxPerfProfAccXscalePmuEvent pmuEvent4,
    UINT32 eventRate4);

/**
 * @ingroup IxPerfProfAcc
 *
 * @fn ixPerfProfAccXscalePmuEventSampStop(
   IxPerfProfAccXscalePmuSamplePcProfile *eventProfile1,
   IxPerfProfAccXscalePmuSamplePcProfile *eventProfile2,
   IxPerfProfAccXscalePmuSamplePcProfile *eventProfile3,
   IxPerfProfAccXscalePmuSamplePcProfile *eventProfile4)
 *
 * @brief Stops the event based sampling
 *
 * @param       *eventProfile1 @ref IxPerfProfAccXscalePmuSamplePcProfile [out] - 
 *              pointer to the array of profiles for each program counter value;
 *              the user should set the size of the array to 
 *				IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES. It is the 
 *				users's responsibility to allocate memory for this pointer.
 * @param       *eventProfile2 @ref IxPerfProfAccXscalePmuSamplePcProfile [out] - 
 *              pointer to the array of profiles for each program counter value;
 *              the user should set the size of the array to 
 *				IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES.  It is the 
 *				users's responsibility to allocate memory for this pointer.
 * @param       *eventProfile3 @ref IxPerfProfAccXscalePmuSamplePcProfile [out] - 
 *              pointer to the array of profiles for each program counter value;
 *              the user should set the size of the array to 
 *				IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES.  It is the 
 *				users's responsibility to allocate memory for this pointer.
 * @param       *eventProfile4 @ref IxPerfProfAccXscalePmuSamplePcProfile [out] - 
 *              pointer to the array of profiles for each program counter value;
 *              the user should set the size of the array to 
 *				IX_PERFPROF_ACC_XSCALE_PMU_MAX_PROFILE_SAMPLES.  It is the 
 *				users's responsibility to allocate memory for this pointer.
 *
 * This API stops the event based sampling.  The results are stored in the 
 * pointers allocated by the user.  It can only be called once 
 * ixPerfProfAccEventSampStart has been called.
 *
 * @return 
 *      - IX_PERFPROF_ACC_STATUS_SUCCESS if event based sampling is stopped 
 *         successfully
 *      - IX_PERFPROF_ACC_STATUS_XSCALE_PMU_START_NOT_CALLED if
 *          ixPerfProfAccEventSampStart not called first.
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */
PUBLIC IxPerfProfAccStatus 
ixPerfProfAccXscalePmuEventSampStop(
    IxPerfProfAccXscalePmuSamplePcProfile *eventProfile1,
    IxPerfProfAccXscalePmuSamplePcProfile *eventProfile2,
    IxPerfProfAccXscalePmuSamplePcProfile *eventProfile3,
    IxPerfProfAccXscalePmuSamplePcProfile *eventProfile4);

/**                                                                             
 * @ingroup IxPerfProfAcc                                                      
 *                                                                              
 * @fn ixPerfProfAccXscalePmuResultsGet(IxPerfProfAccXscalePmuResults *results)     
 *                                                                              
 * @brief Reads the current value of the counters and their overflow                                        
 *                                                                              
 * @param *results @ref IxPerfProfAccXscalePmuResults [out] - pointer to the 
          results struct.  It is the user's responsibility to allocate memory
          for this pointer
 *                                                                              
 * This API reads the value of all four event counters and the clock counter, 
 * and the associated overflows.  It does not give results associated with 
 * sampling, i.e. PC and their frequencies.  This API can be called at any time
 * once a process has been started. If it is called before a process has started
 * the user should be aware that the values it contains are default values and 
 * might be meaningless.  The values of the counters are stored in the pointer 
 * allocated by the client.
 *                                                                    
 * @return - none
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */                                                                             
PUBLIC void                                                     
ixPerfProfAccXscalePmuResultsGet(IxPerfProfAccXscalePmuResults *results);    

/**
 * @ingroup IxPerfProfAcc
 * 
 * @fn ixPerfProfAccBusPmuStart(
        IxPerfProfAccBusPmuMode mode, 
        IxPerfProfAccBusPmuEventCounters1 pecEvent1,
        IxPerfProfAccBusPmuEventCounters2 pecEvent2, 
        IxPerfProfAccBusPmuEventCounters3 pecEvent3, 
        IxPerfProfAccBusPmuEventCounters4 pecEvent4,
        IxPerfProfAccBusPmuEventCounters5 pecEvent5, 
        IxPerfProfAccBusPmuEventCounters6 pecEvent6, 
        IxPerfProfAccBusPmuEventCounters7 pecEvent7)
 * @brief Initializes all the counters and selects events to be monitored.
 *
 * Function initializes all the counters and assigns the events associated 
 * with the counters. Users send in the mode and events they want to count.
 * This API verifies if the combination chosen is appropriate 
 * and sets all the registers accordingly. Selecting HALT mode will result
 * in an error. User should use ixPerfProfAccBusPmuStop() to HALT. 
 * 
 * 
 * @param mode @ref IxPerfProfAccStateBusPmuMode [in] - Mode selection.
 * @param pecEvent1 @ref IxPerfProfAccBusPmuEventCounters1 [in] - Event for PEC1.
 * @param pecEvent2 @ref IxPerfProfAccBusPmuEventCounters2 [in] - Event for PEC2.
 * @param pecEvent3 @ref IxPerfProfAccBusPmuEventCounters3 [in] - Event for PEC3.
 * @param pecEvent4 @ref IxPerfProfAccBusPmuEventCounters4 [in] - Event for PEC4.
 * @param pecEvent5 @ref IxPerfProfAccBusPmuEventCounters5 [in] - Event for PEC5.
 * @param pecEvent6 @ref IxPerfProfAccBusPmuEventCounters6 [in] - Event for PEC6.
 * @param pecEvent7 @ref IxPerfProfAccBusPmuEventCounters7 [in] - Event for PEC7.
 *
 * @return 
 *      - IX_PERFPROF_ACC_STATUS_SUCCESS - Initialization executed 
 *        successfully.  
 *      - IX_PERFPROF_ACC_STATUS_BUS_PMU_MODE_ERROR - Error in selection of 
 * 		  mode. Only NORTH, SOUTH and SDRAM modes are allowed. 
 *      - IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC1_ERROR - Error in selection of 
 *        event for PEC1
 *      - IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC2_ERROR - Error in selection of 
 *        event for PEC2 
 *      - IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC3_ERROR - Error in selection of 
 *        event for PEC3
 *      - IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC4_ERROR - Error in selection of 
 *        event for PEC4
 *      - IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC5_ERROR - Error in selection of 
 *        event for PEC5
 *      - IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC6_ERROR - Error in selection of 
 *        event for PEC6
 *      - IX_PERFPROF_ACC_STATUS_BUS_PMU_PEC7_ERROR - Error in selection of 
 *        event for PEC7
 * 		- IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS - another utility 
 * 		  is running
 * 		- IX_PERFPROF_ACC_STATUS_FAIL - Failed to start because interrupt 
 * 		  service routine fails to bind.  
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 **/
PUBLIC 
IxPerfProfAccStatus ixPerfProfAccBusPmuStart (
        IxPerfProfAccBusPmuMode mode, 
        IxPerfProfAccBusPmuEventCounters1 pecEvent1,
        IxPerfProfAccBusPmuEventCounters2 pecEvent2, 
        IxPerfProfAccBusPmuEventCounters3 pecEvent3, 
        IxPerfProfAccBusPmuEventCounters4 pecEvent4,
        IxPerfProfAccBusPmuEventCounters5 pecEvent5, 
        IxPerfProfAccBusPmuEventCounters6 pecEvent6, 
        IxPerfProfAccBusPmuEventCounters7 pecEvent7);

/**
 * @ingroup IxPerfProfAcc
 * 
 * @fn ixPerfProfAccBusPmuStop(void)
 * @brief Stops all counters. 
 * 
 * This function stops all the PECs by setting the halt bit in the ESR.
 *
 *
 * @return 
 *      - IX_PERFPROF_ACC_STATUS_SUCCESS - Counters successfully halted.
 *      - IX_PERFPROF_ACC_STATUS_FAIL - Counters could'nt be halted. 
 *		- IX_PERFPROF_ACC_STATUS_BUS_PMU_START_NOT_CALLED - the 
 * 		  ixPerfProfAccBusPmuStart() function is not called.
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 **/
PUBLIC IxPerfProfAccStatus 
ixPerfProfAccBusPmuStop (void);

/**
 * @ingroup IxPerfProfAcc
 * 
 * @fn ixPerfProfAccBusPmuResultsGet (
		IxPerfProfAccBusPmuResults *busPmuResults)
 * @brief Gets values of all counters 
 * 
 * This function is responsible for getting all the counter values from the 
 * lower API and putting it into an array for the user.
 *
 * @param *busPmuResults @ref IxPerfProfAccBusPmuResults [out]
 *           - Pointer to a structure of arrays to store all counter values.
 *
 * @return  none
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 **/
PUBLIC void
ixPerfProfAccBusPmuResultsGet (IxPerfProfAccBusPmuResults *BusPmuResults);

/**
 * @ingroup IxPerfProfAcc
 * 
 * @fn ixPerfProfAccBusPmuPMSRGet (
	UINT32 *pmsrValue)
 * @brief Get values of PMSR  
 *
 * This API gets the Previous Master Slave Register
 * value and returns it to the calling function. This value indicates
 * which master or slave accessed the north, south bus or sdram last.
 * The value returned by this function is a 32 bit value and is read
 * from location of an offset 0x0024 of the base value.
 *
 * The PMSR value returned indicate the following:
 * <pre>
 *
 * *************************************************************************************
 * *  Bit    *  Name  *       Description                                              *
 * *                                                                                   * 
 * *************************************************************************************
 * * [31:18]  *Reserved*                                                               *
 * *************************************************************************************
 * * [17:12] *  PSS   * Indicates which of the slaves on                               *
 * *         *        *  ARBS was previously                                           *
 * *         *        * accessed by the AHBS.                                          * 
 * *         *        * [000001] Expansion Bus                                         *
 * *         *        * [000010] SDRAM Controller                                      *
 * *         *        * [000100] PCI                                                   *
 * *         *        * [001000] Queue Manager                                         * 
 * *         *        * [010000] AHB-APB Bridge                                        * 
 * *         *        * [100000] Reserved                                              *
 * *************************************************************************************
 * * [11:8]  *  PSN   * Indicates which of the Slaves on                               *
 * *         *        * ARBN was previously                                            *
 * *         *        * accessed the AHBN.                                             *
 * *         *        * [0001] SDRAM Controller                                        *
 * *         *        * [0010] AHB-AHB Bridge                                          *
 * *         *        * [0100] Reserved                                                *
 * *         *        * [1000] Reserved                                                *
 * *************************************************************************************
 * *  [7:4]  *  PMS   * Indicates which of the Masters on                              *
 * *         *        * ARBS was previously                                            *
 * *         *        * accessing the AHBS.                                            *
 * *         *        * [0001] Gasket                                                  *
 * *         *        * [0010] AHB-AHB Bridge                                          *
 * *         *        * [0100] PCI                                                     *
 * *         *        * [1000] APB                                                     *
 * *************************************************************************************
 * *  [3:0]  *  PMN   * Indicates which of the Masters on                              *
 * *         *        * ARBN was previously                                            *
 * *         *        * accessing the AHBN.                                            *
 * *         *        * [0001] NPEA                                                    *
 * *         *        * [0010] NPEB                                                    *
 * *         *        * [0100] NPEC                                                    *
 * *         *        * [1000] Reserved                                                *
 * *************************************************************************************
 * </pre>
 *
 * @param *pmsrValue UINT32 [out] - Pointer to return PMSR value. Users need to
 * 							  allocate storage for psmrValue.   
 *
 * @return none 
 *
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 **/
PUBLIC void
ixPerfProfAccBusPmuPMSRGet (
UINT32 *pmsrValue);


/** 
 * The APIs below are specifically used for Xcycle module. 
 **/

/**
 * @ingroup IxPerfProfAcc
 * 
 * @fn ixPerfProfAccXcycleBaselineRun (
		UINT32 *numBaselineCycle) 
 *
 * @brief Perform baseline for Xcycle 
 *
 * @param *numBaselineCycle UINT32 [out] - pointer to baseline value after
 * 					calibration. Calling function are responsible for 
 *					allocating memory space for this pointer. 
 *
 * Global Data  : 
 *                        - None.
 *                        
 * This function MUST be run before the Xcycle tool can be used. This 
 * function must be run immediately when the OS boots up with no other 
 * addition programs running. 
 * Addition note :     This API will measure the time needed to perform
 * a fix amount of CPU instructions (~ 1 second worth of loops) as a 
 * highest priority task and with interrupt disabled. The time measured
 * is known as the baseline - interpreted as the shortest time 
 * needed to complete the amount of CPU instructions. The baseline is 
 * returned as unit of time in 66Mhz clock tick.  
 *
 * @return 
 *      - IX_PERFPROF_ACC_STATUS_SUCCESS - successful run, result is returned
 *      - IX_PERFPROF_ACC_STATUS_XCYCLE_PRIORITY_SET_FAIL - failed to change
 *         task priority
 *      - IX_PERFPROF_ACC_STATUS_XCYCLE_PRIORITY_RESTORE_FAIL - failed to
 *         restore task priority
 *      - IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS - another utility 
 *         is running 
 *      - IX_PERFPROF_ACC_STATUS_XCYCLE_MEASUREMENT_IN_PROGRESS  - Xcycle 
 *	  tool has already started
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */
PUBLIC IxPerfProfAccStatus
ixPerfProfAccXcycleBaselineRun(
	UINT32 *numBaselineCycle); 

/**
 * @ingroup IxPerfProfAcc
 * 
 * @fn ixPerfProfAccXcycleStart(
		UINT32 numMeasurementsRequested);
 *
 * @brief Start the measurement
 *
 * @param  numMeasurementsRequested UINT32 [in] - number of measurements 
 *							to perform. Value can be 0 to  
 *							IX_PERFPROF_ACC_XCYCLE_MAX_NUM_OF_MEASUREMENTS. 
 *							0 indicate continuous measurement. 
 *
 * Global Data  : 
 *                        - None.
 *                        
 * 
 * Start the measurements immediately. 
 * numMeasurementsRequested specifies number of measurements to run. 
 * If numMeasurementsRequested is set to 0, the measurement will
 * be performed continuously until IxPerfProfAccXcycleStop()
 * is called.  
 * It is estimated that 1 measurement takes approximately 1 second during 
 * low CPU utilization, therefore 128 measurement takes approximately 128 sec.
 * When CPU utilization is high, the measurement will take longer.
 * This function spawn a task the perform the measurement and returns. 
 * The measurement may continue even if this function returns. 
 *
 * IMPORTANT: Under heavy CPU utilization, the task spawn by this 
 * function may starve and fail to respond to stop command. User 
 * may need to kill the task manually in this case.  
 *
 * There are only IX_PERFPROF_ACC_XCYCLE_MAX_NUM_OF_MEASUREMENTS 
 * storage available so storing is wrapped around if measurements are  
 * more than IX_PERFPROF_ACC_XCYCLE_MAX_NUM_OF_MEASUREMENTS.
 *
 *
 * @return 
 *      - IX_PERFPROF_ACC_STATUS_SUCCESS - successful start, a thread is created 
 *	   in the background to perform measurement. 
 *      - IX_PERFPROF_ACC_STATUS_XCYCLE_PRIORITY_SET_FAIL - failed to set
 *         task priority
 *      - IX_PERFPROF_ACC_STATUS_XCYCLE_THREAD_CREATE_FAIL - failed to create
 *	   thread to perform measurement.
 *      - IX_PERFPROF_ACC_STATUS_XCYCLE_NO_BASELINE - baseline is not available 
 *      - IX_PERFPROF_ACC_STATUS_XCYCLE_MEASUREMENT_REQUEST_OUT_OF_RANGE -
 *	   value is larger than IX_PERFPROF_ACC_XCYCLE_MAX_NUM_OF_MEASUREMENTS
 *      - IX_PERFPROF_ACC_STATUS_XCYCLE_MEASUREMENT_IN_PROGRESS	- Xcycle tool 
 *	   has already started
 *      - IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS - another utility is 
 *         running
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */
PUBLIC IxPerfProfAccStatus
ixPerfProfAccXcycleStart (
	UINT32 numMeasurementsRequested);  

/**
 * @ingroup IxPerfProfAcc
 * 
 * @fn ixPerfProfAccXcycleStop(void); 
 *
 * @brief Stop the Xcycle measurement
 *
 * @param None
 *
 * Global Data  : 
 *                        - None.
 *                        
 * Stop Xcycle measurements immediately. If the measurements have stopped 
 * or not started, return IX_PERFPROF_STATUS_XCYCLE_MEASUREMENT_NOT_RUNNING. 
 * Note: This function does not stop measurement cold. The measurement thread 
 * may need a few seconds to complete the last measurement. User needs to use
 * ixPerfProfAccXcycleInProgress() to determine if measurement is indeed
 * completed. 
 *
 * @return 
 *      - IX_PERFPROF_ACC_STATUS_SUCCESS - successful measurement is stopped
 *      - IX_PERFPROF_STATUS_XCYCLE_MEASUREMENT_NOT_RUNNING - no measurement running
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */
PUBLIC IxPerfProfAccStatus
ixPerfProfAccXcycleStop(void); 

/**
 * @ingroup IxPerfProfAcc
 * 
 * @fn ixPerfProfAccXcycleResultsGet(
	IxPerfProfAccXcycleResults *xcycleResult ) 
 *
 * @brief Get the results of Xcycle measurement
 *
 * @param *xcycleResult @ref IxPerfProfAccXcycleResults [out] - Pointer to 
 * 				results of last measurements. Calling function are 
 *				responsible for allocating memory space for this pointer.
 *
 * Global Data  : 
 *                        - None.
 *                        
 * Retrieve the results of last measurement. User should use 
 * ixPerfProfAccXcycleInProgress() to check if measurement is completed 
 * before getting the results.
 *
 * @return 
 *      - IX_PERFPROF_ACC_STATUS_SUCCESS - successful 
 *      - IX_PERFPROF_ACC_STATUS_FAIL - result is not complete. 
 *      - IX_PERFPROF_ACC_STATUS_XCYCLE_NO_BASELINE - baseline is performed
 *      - IX_PERFPROF_ACC_STATUS_XCYCLE_MEASUREMENT_IN_PROGRESS  - Xcycle 
 *         tool is still running
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */
PUBLIC IxPerfProfAccStatus
ixPerfProfAccXcycleResultsGet (
    IxPerfProfAccXcycleResults *xcycleResult);  

/**
 * @ingroup IxPerfProfAcc
 * 
 * @fn ixPerfProfAccXcycleInProgress (void)
 *
 * @brief Check if Xcycle is running
 *
 * @param None
 * Global Data  : 
 *                        - None.
 *                        
 * Check if Xcycle measuring task is running. 
 *
 * @return 
 *      - true - Xcycle is running
 *      - false - Xcycle is not running
 *              
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */
PUBLIC BOOL
ixPerfProfAccXcycleInProgress(void); 

#ifdef __linux
/**
 * @ingroup IxPerfProfAcc
 *
 * @fn ixPerfProfAccXscalePmuTimeSampCreateProcFile 
 *
 * @brief Enables proc file to call module function
 *
 * @param None
 *
 * Global Data  :
 *                        - None.
 *
 * This function is declared globally to enable /proc directory system to call
 * and execute the function when the registered file is called. This function is not meant to 
 * be called by the user.
 *
 * @return
 *      - Length of data written to file.
 *
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */
int
ixPerfProfAccXscalePmuTimeSampCreateProcFile (char *buf, char **start, off_t offset,
                                      int count, int *eof, void *data);

/**
 * @ingroup IxPerfProfAcc
 *
 * @fn ixPerfProfAccXscalePmuEventSampCreateProcFile 
 *
 * @brief Enables proc file to call module function 
 *
 * @param None
 *
 * Global Data  :
 *                        - None.
 *
 * This function is declared globally to enable /proc directory system to call
 * and execute the function when the registered file is called. This function is not meant to 
 * be called by the user.
 *
 * @return
 *      - Length of data written to file.
 *
 * @li   Reentrant    : no
 * @li   ISR Callable : no
 *
 */
int
ixPerfProfAccXscalePmuEventSampCreateProcFile (char *buf, char **start, off_t offset,
                                      int count, int *eof, void *data);


#endif /* ifdef __linux */

#endif /* ndef IXPERFPROFACC_H */

/**
 *@} defgroup IxPerfProfAcc 
 */


