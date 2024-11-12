/*
* Copyright 2013-2016 Freescale Semiconductor, Inc.
* Copyright 2016-2024 NXP
*
* NXP Proprietary. This software is owned or controlled by NXP and may
* only be used strictly in accordance with the applicable license terms. 
* By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that
* you have read, and that you agree to comply with and are bound by,
* such license terms.  If you do not agree to be bound by the applicable
* license terms, then you may not retain, install, activate or otherwise
* use the software.
*/

#include "../source/system/nt_system_prv.h"
#include "../source/xtalk/nt_xtalk_prv.h"
#include "nt_xtalk.h"

#define NT_XTALK_ONE (32767)
#define MAT_IND(row,col,size)(row * size + col)

struct nt_xtalk_data xtalk_data;

/* Local functions */
int32_t nt_mult_16(int32_t x, int32_t y)
{
    return (int32_t)(((int64_t)x * (int64_t)y) >> 15);
}

/** interface system cross-talk */
const struct nt_system_xtalk_interface nt_system_xtalk_interface = {
    .init        = _nt_system_xtalk_init,
    .process     = _nt_system_xtalk_process,
    .name        = NT_SYSTEM_XTALK_NAME,
    .params_size = sizeof(struct nt_system_xtalk_params),
};

/* Init the cross-talk functions for the system. */
int32_t _nt_system_xtalk_init(struct nt_kernel *system)
{
    NT_ASSERT(system != NULL)
    NT_ASSERT(system->rom->xtalk_interface != NULL)
    NT_ASSERT(system->rom->xtalk_interface->process != NULL)
    NT_ASSERT(system->rom->xtalk_electrodes != NULL)
    NT_ASSERT(system->rom->xtalk_params != NULL)
    NT_ASSERT(system->rom->xtalk_params->actMat != NULL)     
    NT_ASSERT(system->rom->xtalk_params->nt_xtalk_neighbours <= 4)  /* Number of sensors used for cross-talk reduction */
    NT_ASSERT(system->rom->xtalk_params->nt_xtalk_neighbours >= 1)  /* Code supports only neighbours K = 1, 2, 3, 4 */
    NT_ASSERT(NT_XTALK_NSENSORS_TRACE_MAX >= NT_XTALK_NSENSORS)
   
    /* Allocation the array for cross-talk reduction depend on the cross-talk electrod number */
    uint32_t elec_xtalk_cnt = system->xtalk_electrodes_cnt;
    NT_ASSERT(elec_xtalk_cnt <= 32); /* cross-talk can be calculated for 32 electrodes as maximum */ 
    NT_ASSERT(elec_xtalk_cnt >= 2); /* cross-talk can be calculated for 2 electrodes as minimum */ 
    
    /* Initialization and clearing the dynamic arrays*/
    system->xtalk_electrodes            = _nt_mem_alloc((uint32_t)sizeof(struct nt_electrode_data *) * (uint32_t)elec_xtalk_cnt);
    system->xtalk_data->adjCol          = _nt_mem_alloc((uint32_t)sizeof(int32_t) * (uint32_t)elec_xtalk_cnt);
    system->xtalk_data->actToConfRowGain= _nt_mem_alloc((uint32_t)sizeof(int32_t) * (uint32_t)elec_xtalk_cnt);
    system->xtalk_data->confMat         = _nt_mem_alloc((uint32_t)sizeof(int32_t) * (uint32_t)system->rom->xtalk_params->nt_xtalk_neighbours * (uint32_t)system->rom->xtalk_params->nt_xtalk_neighbours);
    system->xtalk_data->transformVec    = _nt_mem_alloc((uint32_t)sizeof(int32_t) * (uint32_t)elec_xtalk_cnt * (uint32_t)system->rom->xtalk_params->nt_xtalk_neighbours);
    system->xtalk_data->delta           = _nt_mem_alloc((uint32_t)sizeof(int32_t) * (uint32_t)elec_xtalk_cnt);
    system->xtalk_data->delta_reduced   = _nt_mem_alloc((uint32_t)sizeof(int32_t) * (uint32_t)elec_xtalk_cnt);
    system->xtalk_data->sensInd         = _nt_mem_alloc((uint32_t)sizeof(uint16_t)* (uint32_t)elec_xtalk_cnt);
    system->xtalk_data->indButtons      = _nt_mem_alloc((uint32_t)sizeof(uint16_t)* (uint32_t)elec_xtalk_cnt * (uint32_t)system->rom->xtalk_params->nt_xtalk_neighbours);   
    system->xtalk_data->profile_buffer  = _nt_mem_alloc((uint32_t)sizeof(int16_t) * (uint32_t)elec_xtalk_cnt);    
    system->xtalk_data->state_prev      = _nt_mem_alloc((uint32_t)sizeof(int8_t)  * (uint32_t)elec_xtalk_cnt);
    
    if (system->xtalk_electrodes == NULL)
    {
        return (int32_t)NT_FAILURE;
    }

    xtalk_rest_transform(system);
    
    /* Full optimization at once */
    nt_xtalk_optimize_transform(system);

    return (int32_t)NT_SUCCESS;
}

/* Init the cross-talk electrode, must be assigned after module init function.*/
int32_t _nt_system_xtalk_electrode_init(struct nt_kernel *system)
{
    NT_ASSERT(system != NULL);
    uint32_t i;
    
    for (i = 0; i < system->xtalk_electrodes_cnt; i++)
    {                                                        
        system->xtalk_electrodes[i] = _nt_electrode_get_data(system->rom->xtalk_electrodes[i]);
    }   
    return (int32_t)NT_SUCCESS;
}

/* Process the cross-talk on the module. */
int32_t _nt_system_xtalk_process(struct nt_kernel *system)
{
    uint32_t elec_xtalk_cnt = system->xtalk_electrodes_cnt;
    struct nt_electrode_data *elec;   
     
    /* Get delta signal */
    int32_t delta;
    while ((bool)(elec_xtalk_cnt--))
    {
        elec = system->xtalk_electrodes[elec_xtalk_cnt];
        
        delta = _nt_electrode_get_delta( elec );    /* Delta has been normalised and smoothed in keydetector_mbw */
		
        system->xtalk_data->delta[elec_xtalk_cnt] = delta; 
        xtalk_delta[elec_xtalk_cnt] = system->xtalk_data->delta[elec_xtalk_cnt];  /* copy xtalk deltas to static array for FreeMASTER-lite */
    }

    if(xtalk_adapt_on == (bool)1)
    {
        nt_xtalk_adapt_model_process(system);
    }
    if(xtalk_request_adapt == (bool)1)
    {
        xtalk_request_adapt = 0;
        xtalk_adapt_on = 0;
        nt_xtalk_optimize_transform(system);
    }
    
    /* Cross-talk reduction */    
    if((bool)xtalk_reduction_enabled)
    {
        nt_xtalk_reduction(system);          
    }  

    /* Updated the signal for keydetector by xtalk delta reduced or by delta in case xtalk reduction is not used */ 
    elec_xtalk_cnt = system->xtalk_electrodes_cnt;
    while ((bool)(elec_xtalk_cnt--))
    {
        elec = system->xtalk_electrodes[elec_xtalk_cnt];
        if((bool)xtalk_reduction_enabled)
        {   
            delta = system->xtalk_data->delta_reduced[elec_xtalk_cnt];
            xtalk_delta_reduced[elec_xtalk_cnt] = delta; 
            if (xtalk_reduction_enabled == 2U)   /* make reduced delta visible */
            {    
              delta = system->xtalk_data->delta[elec_xtalk_cnt];
            }
        }
        else /* Reduction disabled */
        {  
            delta = system->xtalk_data->delta[elec_xtalk_cnt];
            xtalk_delta_reduced[elec_xtalk_cnt] = 0;
        }
        _nt_electrode_set_delta_red(elec, delta);
    }    
    
    return (int32_t)NT_SUCCESS;  
}

void nt_xtalk_reduction(struct nt_kernel *system)
{
    int32_t OutSensor;
    uint8_t indRaw = 0, cSensor = 0, cCol = 0;
    for (cSensor = 0; cSensor < system->xtalk_electrodes_cnt; cSensor++)
    {
       OutSensor = 0;
        /* Dot product */
        for (cCol = 0; cCol < system->rom->xtalk_params->nt_xtalk_neighbours; cCol++)
        {
            indRaw = system->xtalk_data->indButtons[ MAT_IND(cSensor, cCol, system->rom->xtalk_params->nt_xtalk_neighbours) ];
            /* multiplication Q15 * Q15.16 to Q15 */
            OutSensor += ( (int64_t) system->xtalk_data->delta[indRaw] * system->xtalk_data->transformVec[ MAT_IND(cSensor, cCol, system->rom->xtalk_params->nt_xtalk_neighbours) ] ) >> 15;
        }
        /* Set negative values to 0 */ 
        system->xtalk_data->delta_reduced[cSensor] = OutSensor > 0 ? OutSensor : 0;
    }     
}

/* Calculate gain vector than can be used to convert activation matrix into confusion matrix*/
void nt_xtalk_get_act_to_conf_gain(struct nt_kernel *system)
{
    for (uint8_t cRow = 0; cRow < system->xtalk_electrodes_cnt; ++cRow)
    {
        /* Q31 */
        system->xtalk_data->actToConfRowGain[cRow] = NT_INV_32(system->rom->xtalk_params->actMat[MAT_IND(cRow,cRow,system->xtalk_electrodes_cnt)]);
    }
}

/* Find K max indices in specified column of the activation matrix */
void nt_xtalk_find_max_ind(uint8_t indCol, struct nt_kernel *system)
{
    /* Temporary vector needed to calculate K maximum values in confusion matrix */
    int32_t cpyRow[NT_XTALK_NSENSORS] = {0};
    uint8_t cRow = 0, cMax = 0;
    int32_t tmpMax = 0;

    /* Copy activation matrix column */
    for (cRow = 0; cRow < system->xtalk_electrodes_cnt; ++cRow)
    {
        cpyRow[cRow] = ((int64_t)system->rom->xtalk_params->actMat[MAT_IND(cRow,indCol,system->xtalk_electrodes_cnt)] * system->xtalk_data->actToConfRowGain[cRow]) >> 16;
    }

    /* Keep target index as max */
    system->xtalk_data->sensInd[0] = indCol;
    /* Remove max for new search */
    cpyRow[system->xtalk_data->sensInd[0]] = 0;

    /* Loop to find M max elements */
    for(cMax = 1; cMax < system->rom->xtalk_params->nt_xtalk_neighbours; ++cMax)
    {
        /* Loop over N sensors */
        for(cRow = 0; cRow < system->xtalk_electrodes_cnt; ++cRow)
        {
            /* Get new max */
            if (cpyRow[cRow] > tmpMax)
            {
                tmpMax = cpyRow[cRow];
                system->xtalk_data->sensInd[cMax] = cRow; 
            }
        }
        /* Remove max for new search */
        cpyRow[system->xtalk_data->sensInd[cMax]] = 0;
        tmpMax = 0;
    }
}

/* Calculate first column of adjoint matrix. */
void nt_xtalk_get_adjoint_col_0(struct nt_kernel *system)
{
    switch (system->rom->xtalk_params->nt_xtalk_neighbours) 
    {   case 1U:
        {     /* one */
            system->xtalk_data->adjCol[0] = (int32_t)(NT_XTALK_ONE);
        }
        break;        
        case 2U:
        { 
            system->xtalk_data->adjCol[0] =  system->xtalk_data->confMat[MAT_IND( 1, 1, 2 )];
            system->xtalk_data->adjCol[1] = -system->xtalk_data->confMat[MAT_IND( 1, 0, 2 )];
        }
        break;        
        case 3U:
        { 
        
            system->xtalk_data->adjCol[0] = NT_MUL_16( system->xtalk_data->confMat[MAT_IND( 1, 1, 3)], system->xtalk_data->confMat[MAT_IND( 2, 2, 3 )])
                              - NT_MUL_16( system->xtalk_data->confMat[MAT_IND( 1, 2, 3)], system->xtalk_data->confMat[MAT_IND( 2, 1, 3 )]);

            system->xtalk_data->adjCol[1] = NT_MUL_16( system->xtalk_data->confMat[MAT_IND( 1, 2, 3)], system->xtalk_data->confMat[MAT_IND( 2, 0, 3 )])
                              - NT_MUL_16( system->xtalk_data->confMat[MAT_IND( 1, 0, 3)], system->xtalk_data->confMat[MAT_IND( 2, 2, 3 )]);

            system->xtalk_data->adjCol[2] = NT_MUL_16( system->xtalk_data->confMat[MAT_IND( 1, 0, 3)], system->xtalk_data->confMat[MAT_IND( 2, 1, 3 )])
                              - NT_MUL_16( system->xtalk_data->confMat[MAT_IND( 1, 1, 3)], system->xtalk_data->confMat[MAT_IND( 2, 0, 3 )]);
        }
        break;        
        case 4U:
        { 
            int64_t acc = 0;
        
            /* Q45 -- Q15*3 */
            acc += (int64_t)system->xtalk_data->confMat[MAT_IND( 1, 1, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 2, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 3, 4 )];
            acc -= (int64_t)system->xtalk_data->confMat[MAT_IND( 1, 1, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 3, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 2, 4 )];
            acc -= (int64_t)system->xtalk_data->confMat[MAT_IND( 1, 2, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 1, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 3, 4 )];
            acc += (int64_t)system->xtalk_data->confMat[MAT_IND( 1, 2, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 3, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 1, 4 )];
            acc += (int64_t)system->xtalk_data->confMat[MAT_IND( 1, 3, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 1, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 2, 4 )];
            acc -= (int64_t)system->xtalk_data->confMat[MAT_IND( 1, 3, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 2, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 1, 4 )];
            /* Q15 result */                  
            system->xtalk_data->adjCol[0] = acc >> 30;      
                                     
            acc = 0;                 
            acc += (int64_t)system->xtalk_data->confMat[ MAT_IND( 1, 0, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 3, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 2, 4)];
            acc -= (int64_t)system->xtalk_data->confMat[ MAT_IND( 1, 0, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 2, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 3, 4)];
            acc += (int64_t)system->xtalk_data->confMat[ MAT_IND( 1, 2, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 0, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 3, 4)];
            acc -= (int64_t)system->xtalk_data->confMat[ MAT_IND( 1, 2, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 3, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 0, 4)];
            acc -= (int64_t)system->xtalk_data->confMat[ MAT_IND( 1, 3, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 0, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 2, 4)];
            acc += (int64_t)system->xtalk_data->confMat[ MAT_IND( 1, 3, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 2, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 0, 4)];
            system->xtalk_data->adjCol[1] = acc >> 30;                                                                                           
                                                                                                                                     
            acc = 0;                                                                                                                 
            acc += (int64_t)system->xtalk_data->confMat[ MAT_IND( 1, 0, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 1, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 3, 4)];
            acc -= (int64_t)system->xtalk_data->confMat[ MAT_IND( 1, 0, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 3, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 1, 4)];
            acc -= (int64_t)system->xtalk_data->confMat[ MAT_IND( 1, 1, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 0, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 3, 4)];
            acc += (int64_t)system->xtalk_data->confMat[ MAT_IND( 1, 1, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 3, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 0, 4)];
            acc += (int64_t)system->xtalk_data->confMat[ MAT_IND( 1, 3, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 0, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 1, 4)];
            acc -= (int64_t)system->xtalk_data->confMat[ MAT_IND( 1, 3, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 1, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 0, 4)];
            system->xtalk_data->adjCol[2] = acc >> 30;
                                          
            acc = 0;                      
            acc += (int64_t)system->xtalk_data->confMat[ MAT_IND( 1, 0, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 2, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 1, 4)];
            acc -= (int64_t)system->xtalk_data->confMat[ MAT_IND( 1, 0, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 1, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 2, 4)];
            acc += (int64_t)system->xtalk_data->confMat[ MAT_IND( 1, 1, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 0, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 2, 4)];
            acc -= (int64_t)system->xtalk_data->confMat[ MAT_IND( 1, 1, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 2, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 0, 4)];
            acc -= (int64_t)system->xtalk_data->confMat[ MAT_IND( 1, 2, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 0, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 1, 4)];
            acc += (int64_t)system->xtalk_data->confMat[ MAT_IND( 1, 2, 4)] * system->xtalk_data->confMat[MAT_IND( 2, 1, 4)] * system->xtalk_data->confMat[MAT_IND( 3, 0, 4)];
            system->xtalk_data->adjCol[3] = acc >> 30;       
        }    
        break;
        default:
        /*MISRA rule 16.4*/
        break;
    }
}

/* Calculate KxK confusion sub-matrix from activation matrix */
void nt_xtalk_get_confusion_submat(struct nt_kernel *system)
{
    uint8_t cRow = 0, cCol = 0;
    for (cRow = 0; cRow < system->rom->xtalk_params->nt_xtalk_neighbours; cRow++)
    {
        for (cCol = 0; cCol < system->rom->xtalk_params->nt_xtalk_neighbours; cCol++)
        {
            system->xtalk_data->confMat[MAT_IND(cRow,cCol,system->rom->xtalk_params->nt_xtalk_neighbours)] = ((int64_t)system->rom->xtalk_params->actMat[MAT_IND(system->xtalk_data->sensInd[cRow],system->xtalk_data->sensInd[cCol],system->xtalk_electrodes_cnt)] * system->xtalk_data->actToConfRowGain[system->xtalk_data->sensInd[cRow]]) >> 16;
        }
    }
}

/* Calculate transformation vector for given sensor */
void nt_xtalk_get_transform(uint8_t cRow, struct nt_kernel *system)
{
    int32_t actRow[4];
    uint8_t c1 = 0;
    int32_t dotVal = 0;
    int32_t scale = 0;

    /* Copy activation row */
    for (c1 = 0; c1 < system->rom->xtalk_params->nt_xtalk_neighbours; c1++)
    {
        actRow[c1] = system->rom->xtalk_params->actMat[MAT_IND(cRow,system->xtalk_data->sensInd[c1],system->xtalk_electrodes_cnt)];
    }

    /* Dot product */
    for (c1 = 0; c1 < system->rom->xtalk_params->nt_xtalk_neighbours; c1++)
    {
        dotVal += NT_MUL_16(actRow[c1], system->xtalk_data->adjCol[c1]);
    }

    /* Transformation matrix check. */
    if(dotVal > 0)
    {
        /* Matrix is valid. Apply transformation gain. */
        /* Multiplication Q15 * Q31 to Q15.16 */
        scale = ((int64_t)actRow[0] * NT_INV_32(dotVal)) >> 16;
        for (c1 = 0; c1 < system->rom->xtalk_params->nt_xtalk_neighbours; c1++)
        {
            /* Multiplication Q15.16 * Q15.16 to Q15.16 */
            /* 32767 relates 1.0 as real value */
            system->xtalk_data->adjCol[c1] = ((int64_t)system->xtalk_data->adjCol[c1] * scale) >> 15;
        }
    }else
    {
        /* Transformation is singular.
         * Set transform to [1, 0, ... , 0]
         */
        uint32_t cler_counter;
        for (cler_counter = 1; cler_counter < system->rom->xtalk_params->nt_xtalk_neighbours; cler_counter++) 
        {   system->xtalk_data->adjCol[cler_counter] = 0;
        }
        system->xtalk_data->adjCol[0] = NT_XTALK_ONE ;
    }
}

/* Example how to apply transformation to the input activation vector. */
int32_t nt_xtalk_get_activation(const int32_t* actVec, uint16_t indSensor, struct nt_kernel *system)
{
    int32_t outVal = 0;
    /* Dot product */
    for (uint8_t c1 = 0; c1 < system->rom->xtalk_params->nt_xtalk_neighbours; c1++)
    {
        /* multiplication Q15 * Q15.16 to Q15 */
        outVal += ((int64_t) actVec[c1*system->xtalk_data->indButtons[indSensor]+c1] * system->xtalk_data->transformVec[c1*indSensor+c1]) >> 15;
    }
    return (outVal);
}

void xtalk_rest_transform(struct nt_kernel *system)
{
    for (uint8_t cSensors = 0; cSensors < system->xtalk_electrodes_cnt; cSensors++)
    {    
         system->xtalk_data->indButtons  [MAT_IND(cSensors, 0, system->rom->xtalk_params->nt_xtalk_neighbours)]= cSensors;
         system->xtalk_data->transformVec[MAT_IND(cSensors, 0, system->rom->xtalk_params->nt_xtalk_neighbours)]= NT_XTALK_ONE;
    }
}

void nt_xtalk_optimize_transform(struct nt_kernel *system)
{
    /* Calculate activation to confusion matrix gain vector */
    nt_xtalk_get_act_to_conf_gain(system);

    /* Optimize sensor transformation matrix */
    for (uint8_t cSensors = 0; cSensors < system->xtalk_electrodes_cnt; cSensors++)
    {
        /* Find K max values in sensor column */
        nt_xtalk_find_max_ind(cSensors, system);
        /* Get KxK confusion sub-matrix */
        nt_xtalk_get_confusion_submat(system);
        /* Calculate column 0 of adjoint matrix */
        nt_xtalk_get_adjoint_col_0(system);
        /* Get transformation matrix */
        nt_xtalk_get_transform(cSensors, system);
        /* Copy transformation and indices for given sensor */
        for(uint8_t c1 = 0; c1 < system->rom->xtalk_params->nt_xtalk_neighbours; c1++)
        {
            system->xtalk_data->indButtons  [MAT_IND(cSensors, c1, system->rom->xtalk_params->nt_xtalk_neighbours)] = system->xtalk_data->sensInd[c1];
            system->xtalk_data->transformVec[MAT_IND(cSensors, c1, system->rom->xtalk_params->nt_xtalk_neighbours)] = system->xtalk_data->adjCol[c1];
        }
    }    
}

void nt_xtalk_optimize_transform_step(struct nt_kernel *system)
{
    if (!system->xtalk_data->optReady)
    {
        if (!system->xtalk_data->optGainReady)
        {
            nt_xtalk_get_act_to_conf_gain(system);
            system->xtalk_data->optGainReady = 1;
        }
        else if (system->xtalk_data->optCounter < system->xtalk_electrodes_cnt)
        {
            /* Find K max values in sensor column */
            nt_xtalk_find_max_ind(system->xtalk_data->optCounter, system);
            /* Get KxK confusion sub-matrix */
            nt_xtalk_get_confusion_submat(system);
            /* Calculate column 0 of adjoint matrix */
            nt_xtalk_get_adjoint_col_0(system);
            /* Get transformation matrix */
            nt_xtalk_get_transform(system->xtalk_data->optCounter, system);

            /* Copy transformation and indices for given sensor */
            for (uint8_t c1 = 0; c1 < system->rom->xtalk_params->nt_xtalk_neighbours; c1++)
            {
                system->xtalk_data->indButtons  [c1*system->xtalk_data->optCounter+c1] = system->xtalk_data->sensInd[c1];
                system->xtalk_data->transformVec[c1*system->xtalk_data->optCounter+c1] = system->xtalk_data->adjCol[c1];
            }
            system->xtalk_data->optCounter += 1;
        }
        else
        {
            system->xtalk_data->optReady = 1;
        }
    }
}

uint16_t nt_xtalk_adapt_model_process(struct nt_kernel *system)
{
    struct nt_electrode_data *elec;     
    uint16_t ind_touch;
    uint16_t ind_release = 129;
    uint16_t n_touch = 0;
    uint16_t flag_update = 0;
    uint8_t xtalk_electrodes_cnt = system->xtalk_electrodes_cnt;
    for(uint8_t c1 = 0; c1 < xtalk_electrodes_cnt; ++c1)
    {
        uint8_t state1;

        elec = system->xtalk_electrodes[c1];
        state1 = _nt_electrode_get_last_status(elec);

        if(state1 == (int32_t)NT_ELECTRODE_STATE_TOUCH)
        {
            n_touch++;
            ind_touch = c1;
        }
        else
        {
            if(system->xtalk_data->state_prev[c1] == (int32_t)NT_ELECTRODE_STATE_TOUCH)  /* Release */
            {
                ind_release = c1;
                flag_update = 1;
            }
        }
        system->xtalk_data->state_prev[c1] = state1;
    }

    if(n_touch == 1)
    {
        elec = system->xtalk_electrodes[ind_touch];

        profile_accu_count++;
        int32_t act1 = _nt_electrode_get_delta(elec);

        /* Replace buffer if self activation is higher than that in buffer */
        if(act1 > system->xtalk_data->profile_buffer[ind_touch])
        {
            for(uint8_t c1 = 0; c1 < xtalk_electrodes_cnt; ++c1)
            {
                elec = system->xtalk_electrodes[c1];
                system->xtalk_data->profile_buffer[c1] = _nt_electrode_get_delta(elec);
            }
        }
    }
    if(flag_update)
    {
        if((n_touch == 0) && (profile_accu_count > system->rom->xtalk_params->nt_xtalk_adapt_touch_time))
        {
            int32_t norm_inv = NT_INV_32(system->xtalk_data->profile_buffer[ind_release]);
            for(uint8_t c1 = 0; c1 < xtalk_electrodes_cnt; ++c1)
            {
                system->rom->xtalk_params->actMat[MAT_IND(ind_release,c1,xtalk_electrodes_cnt)] = ((int32_t)system->xtalk_data->profile_buffer[c1] * norm_inv) >> 16;
            }
        }
        for(uint8_t c1 = 0; c1 < xtalk_electrodes_cnt; ++c1)
        {
            system->xtalk_data->profile_buffer[c1] = 0;
        }
        profile_accu_count = 0;        
    }
    return profile_accu_count;
}