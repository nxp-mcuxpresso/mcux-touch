/*
* Copyright 2013-2016, Freescale Semiconductor, Inc.
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

#include "../source/electrodes/nt_electrodes_prv.h"
#include "../source/system/nt_system_prv.h"
#include "nt_keydetector_mbw_prv.h"
#include "nt_keydetectors.h"
#include "../source/system/nt_system_mem_prv.h"
#include "../source/modules/nt_modules_prv.h"

uint32_t baseline_accu_max_count;     /* Fix for long time constants for baseline */

static int32_t _nt_keydetector_mbw_rom_check(const struct nt_keydetector_mbw *rom);
static void _nt_keydetector_mbw_signal_track(struct nt_electrode_data *electrode,
                                               const struct nt_keydetector_mbw *rom,
                                               struct nt_keydetector_mbw_data *ram,
                                               uint16_t signal);
static void _nt_reset_keydetector_mbw_reset(struct nt_electrode_data *electrode, uint32_t signal, uint32_t touch);

static int32_t _nt_keydetector_mbw_init(struct nt_electrode_data *electrode);
static void _nt_keydetector_mbw_measure(struct nt_electrode_data *electrode, uint32_t signal);
static void _nt_keydetector_mbw_process(struct nt_electrode_data *electrode);
static void _nt_keydetector_mbw_enable(struct nt_electrode_data *electrode, uint32_t touch);
static void _nt_keydetector_mbw_reset(struct nt_electrode_data *electrode);

static void _nt_keydetector_mbw_lock_baseline(struct nt_electrode_data *electrode,
                                                const struct nt_keydetector_mbw *rom,
                                                struct nt_keydetector_mbw_data *ram,
                                                uint16_t signal);

static void _nt_keydetector_mbw_unlock_baseline(struct nt_electrode_data *electrode,
                                                  const struct nt_keydetector_mbw *rom,
                                                  struct nt_keydetector_mbw_data *ram,
                                                  uint16_t signal);

static uint16_t find_min(uint16_t * buff, uint8_t bsize);
static uint16_t find_median(uint16_t *buff, uint8_t bsize); /* Important note: this changes the input buffer */

const struct nt_keydetector_mbw nt_keydetector_mbw_default = {
    .signal_filter               = {.coef1 = 1},
    .base_avrg                   = {.n2_order = 9},
    .non_activity_avrg           = {.n2_order = (int32_t)NT_FILTER_MOVING_AVERAGE_MAX_ORDER},
    .entry_event_cnt             = 4,
    .signal_to_noise_ratio       = 8,
    .deadband_cnt                = 5,
    .min_noise_limit             = 20,
    .baseline_track_window       = 2000,
    .baseline_track_window_touch = 8000,
    .touch_limit                 = 3,
    .tau_smooth_signal           = 10,
    .tau_smooth_baseline         = 5000,
    .debounce_length             = 5,
    .baseline_debounce_length    = 0,
};


const struct nt_keydetector_interface nt_keydetector_mbw_interface = {
    .nt_keydetector_init    = _nt_keydetector_mbw_init,
    .nt_keydetector_process = _nt_keydetector_mbw_process,
    .nt_keydetector_measure = _nt_keydetector_mbw_measure,
    .nt_keydetector_enable  = _nt_keydetector_mbw_enable,
    .nt_keydetector_reset   = _nt_keydetector_mbw_reset,
    .name                   = "nt_keydetector_mbw_interface",
    .params_size            = sizeof(struct nt_keydetector_mbw),
};



static int32_t _nt_keydetector_mbw_rom_check(const struct nt_keydetector_mbw *rom)
{
    return (int32_t)NT_SUCCESS;
}

static int32_t _nt_keydetector_mbw_init(struct nt_electrode_data *electrode)
{
    NT_ASSERT(electrode->rom->keydetector_interface == &nt_keydetector_mbw_interface);

    const struct nt_keydetector_mbw *rom = electrode->rom->keydetector_params.mbw;

    NT_ASSERT(rom->debounce_length < 16U)
    NT_ASSERT(rom->baseline_debounce_length < 200U)


    if (_nt_keydetector_mbw_rom_check(rom) != (int32_t)NT_SUCCESS)
    {
        return (int32_t)NT_FAILURE;
    }

    electrode->keydetector_data.mbw =
        (struct nt_keydetector_mbw_data *)_nt_mem_alloc(sizeof(struct nt_keydetector_mbw_data));

    if (electrode->keydetector_data.mbw == NULL)
    {
        return (int32_t)NT_OUT_OF_MEMORY;
    }

    electrode->keydetector_data.mbw->signal_buffer = _nt_mem_alloc((uint32_t)sizeof(uint32_t) * (uint32_t)rom->debounce_length+1);

    if (electrode->keydetector_data.mbw->signal_buffer == NULL)
    {
        return (int32_t)NT_OUT_OF_MEMORY;
    }

    electrode->keydetector_data.mbw->recovery_cnt = 0;

    electrode->keydetector_data.mbw->base_avrg_init  = rom->base_avrg;
    electrode->keydetector_data.mbw->noise_avrg_init = rom->base_avrg;
    electrode->keydetector_data.mbw->noise_avrg_init.n2_order += 2;

    if (electrode->keydetector_data.mbw->noise_avrg_init.n2_order > (int32_t)NT_FILTER_MOVING_AVERAGE_MAX_ORDER)
    {
        electrode->keydetector_data.mbw->noise_avrg_init.n2_order = (int32_t)NT_FILTER_MOVING_AVERAGE_MAX_ORDER;
    }

    /* fast noise filter */
    electrode->keydetector_data.mbw->f_noise_avrg_init = rom->base_avrg;
    electrode->keydetector_data.mbw->f_noise_avrg_init.n2_order = 4;

    if (electrode->keydetector_data.mbw->f_noise_avrg_init.n2_order < 4)
    {
        electrode->keydetector_data.mbw->f_noise_avrg_init.n2_order = 4;
    }

    electrode->keydetector_data.mbw->predicted_signal_avrg_init = rom->base_avrg;
    if (electrode->keydetector_data.mbw->predicted_signal_avrg_init.n2_order > 2)
    {
        electrode->keydetector_data.mbw->predicted_signal_avrg_init.n2_order -= 2;
    }

    /* Init baseline tracking window counter */
    electrode->keydetector_data.mbw->baseline_cnt = 0U;

    /* Fix for accuracy problem with long time constants for baseline */
    baseline_accu_max_count = rom->tau_smooth_baseline / 100; /* 100ms smoothing at normal time_period is target */

    /* Get the time periode */
    int16_t ts = (int32_t)_nt_system_get_time_period();
    /* Smooth filters initializations */
     /* 4x times faster filter used for baseline level decrease, i.e. Asymetric smooth */
    _nt_asym_smooth_init(rom->tau_smooth_baseline, rom->tau_smooth_baseline / 4, ts * baseline_accu_max_count, &electrode->keydetector_data.mbw->base_smooth);
    _nt_smooth_init(rom->tau_smooth_signal, ts, &electrode->keydetector_data.mbw->base_smooth_fix);
    
    /* Calculate baseline counter maximum value to establish next baseline window periode */
    electrode->keydetector_data.mbw->baseline_cnt_max = (rom->baseline_track_window / ts);

    uint16_t slow_down_factor_during_touch;
    /* Avoid division by zero */
    if((rom->baseline_track_window_touch) > 0U && (rom->baseline_track_window > 0U))
    {
        slow_down_factor_during_touch = rom->baseline_track_window_touch / rom->baseline_track_window;
    }
    else
    {
        slow_down_factor_during_touch = 0U;
    }
    if(slow_down_factor_during_touch >  0U)
    {
        /* Freeze baseline during touch */
        electrode->keydetector_data.mbw->baseline_add_no_touch = slow_down_factor_during_touch;
        electrode->keydetector_data.mbw->baseline_add_touch = 1U;
    }
    else
    {
        /* Slow down baseline during touch */
        electrode->keydetector_data.mbw->baseline_add_no_touch = 1U;
        electrode->keydetector_data.mbw->baseline_add_touch = 0U;
    }

    _nt_electrode_set_status(electrode, (int32_t)NT_ELECTRODE_STATE_INIT);
    return (int32_t)NT_SUCCESS;
}

static void _nt_keydetector_mbw_enable(struct nt_electrode_data *electrode, uint32_t touch)
{
    struct nt_keydetector_mbw_data *ram = electrode->keydetector_data.mbw;

    if ((bool)touch)
    {
        _nt_electrode_set_flag(electrode, (int32_t)NT_ELECTRODE_AFTER_INIT_TOUCH_FLAG);
    }
    else
    {
        _nt_electrode_clear_flag(electrode, (int32_t)NT_ELECTRODE_AFTER_INIT_TOUCH_FLAG);
    }
    ram->filter_state = NT_FILTER_STATE_INIT;
    _nt_electrode_set_status(electrode, (int32_t)NT_ELECTRODE_STATE_INIT);
}

static uint16_t find_min(uint16_t * buff, uint8_t bsize)
{
  uint16_t min = buff[0];
  for (uint8_t i = 1; i < bsize; i++)
  {
    if (buff[i] < min)
      min = buff[i];
  }
  return min;
}

static uint16_t find_median(uint16_t *buff, uint8_t bsize)
{
    size_t nEl = bsize;
    while( nEl > 2 )
    {
        /* Identify min/max */
        uint16_t min1 = 65535u; 
        uint16_t max1 = 0;
        uint16_t indMax = 0;
        uint16_t indMin = 0;
        for( size_t c2 = 0; c2 < nEl; c2++ )
        {
            if( buff[c2] > max1 )
            {
                max1 = buff[c2];
                indMax = c2;
            }
            else if( buff[c2] < min1 )
            {
                min1 = buff[c2];
                indMin = c2;
            }
        }

        /* Remove min/max from buffer */
        size_t ind1 = 0;
        for( size_t c2 = 0; c2 < bsize; c2++ )
        {
            if(( c2 != indMax ) && ( c2 != indMin ))
            {
                buff[ind1++] = buff[c2];
            }
            else
            {
                nEl--;
            }
        }
    }

    /* Averaging if there nEl is even */
    uint16_t out1 = buff[0];
    if( nEl == 2 )
    {
        out1 += buff[1] + 1u;
        out1 = out1 >> 1;
    }
    return out1;
}

static void _nt_keydetector_mbw_measure(struct nt_electrode_data *electrode, uint32_t signal)
{
    NT_ASSERT(electrode->rom->keydetector_interface == &nt_keydetector_mbw_interface);

    const struct nt_keydetector_mbw *rom = electrode->rom->keydetector_params.mbw;
    struct nt_keydetector_mbw_data *ram  = electrode->keydetector_data.mbw;
    uint32_t touch_states = _nt_module_get_electrodes_state(electrode->module_data);
    uint32_t signalNoSmooth;
    uint8_t touch_number = 0U;

    signal = _nt_electrode_normalization_process(electrode, signal);
    signalNoSmooth = signal;
    ram->smooth_signal = _nt_smooth_process(&electrode->keydetector_data.mbw->base_smooth_fix, (int32_t)signalNoSmooth, ram->smooth_signal);
    signal = ram->smooth_signal;

    int32_t s_delta = ((int32_t)signal - (int32_t)electrode->baseline);
	
    /* Delta limitation */
    if (electrode->rom->delta_limit && (ram->filter_state == NT_FILTER_STATE_RUN))
    {
        /* Signed delta is cut off if is higher than electrode's limitation */
        if(s_delta > electrode->rom->delta_limit)
            s_delta = electrode->rom->delta_limit;  

        else if(s_delta < -electrode->rom->delta_limit)
            s_delta = -electrode->rom->delta_limit;
    }
    
	int16_t delta   = s_delta;
    delta = _nt_electrode_delta_normalization_process(electrode, delta);

    /* Min filtering for debounce */
    /* Initialization code before filtering */
    uint32_t delta_debounced = delta;

    /* Initialization smooth filter data or processing them based on actual state */
    if (ram->filter_state == NT_FILTER_STATE_INIT)
    {
        /* Baseline initialization by raw signal value */
        ram->smooth_baseline = (int32_t)signalNoSmooth;
        ram->smooth_baseline_accu = 0u;
        ram->smooth_baseline_accu_count = 0u;
        /* Set maximum initialization value for baseline to find the minimum value in window */
        electrode->baseline = (int32_t)signalNoSmooth;
        electrode->count_init = 0u;
        electrode->init_buffer_count = 0;
        ram->smooth_baseline_min = 65535U;

        /* Recompute with initial baseline */
        delta   = 0U;

        ram->smooth_baseline_min = 65535U;
        ram->smooth_signal_min = 65535U;
        ram->smooth_signal_max = 0u;
        ram->smooth_signal = (int32_t)signal;
        for( size_t c1 = 0; c1 < rom->debounce_length; ++c1 )
        {
            ram->signal_buffer[c1] = 65535U;
        }
    }
        
    /* Baseline init with median after number of scans */   
    if(rom->baseline_debounce_length)    
    {
      if( electrode->count_init < rom->baseline_debounce_length )
      {
        electrode->init_buffer[electrode->init_buffer_count++] = signalNoSmooth;
        if( electrode->init_buffer_count >= INIT_BUFFER_N )
        {
          electrode->init_buffer_count = 0;
        }
        electrode->count_init++;
      }
      if( electrode->count_init == rom->baseline_debounce_length )
      {
        /* Compute median and replace baseline */
        electrode->count_init++;
        uint16_t med1 = find_median( electrode->init_buffer, INIT_BUFFER_N );
        
        /* Baseline re-initialization */
        ram->smooth_baseline = (int32_t)med1;
        ram->smooth_baseline_accu = 0u;
        ram->smooth_baseline_accu_count = 0u;
        electrode->baseline = (int32_t)med1;
        ram->smooth_baseline_min = 65535U;
        
        /* Recompute with initial baseline */
        s_delta = ((int32_t)signal - (int32_t)electrode->baseline);
        delta = _nt_electrode_delta_normalization_process(electrode, delta);
      }
    }
    
    /* Debouncing */
    ram->signal_buffer[ram->shist++] = delta;
    if(ram->shist > rom->debounce_length)
    {
        ram->shist = 0;
    }

    delta_debounced = find_min(ram->signal_buffer, rom->debounce_length);
    
    /* Debouncing - delta limitation and short peaks elimination */
    if(rom->debounce_length && electrode->rom->delta_limit)
    {
        /* Delta bounced higher than delta limitation */
        if(_nt_abs_int32(s_delta) >= electrode->rom->delta_limit)
        {
            /* If delta changes it's polarity, reset debouncing counter */
            if((ram->prev_delta ^ s_delta) < 0)
                ram->debounce_cnt = 1;
            else
                ++ram->debounce_cnt;
            
            if(ram->debounce_cnt <= rom->debounce_length)
            {
                /* Set last valid value for actual delta */
                delta_debounced = _nt_electrode_get_delta(electrode);
                signalNoSmooth = ram->smooth_baseline;
            }
        }
        else
        {    
            ram->debounce_cnt = 0;
        }
        ram->prev_delta = s_delta;
    }
    delta = delta_debounced;  /* Store filtered delta value for next processing */
    
    /* Find touch on other electrode or to adjust filtering */
    while (touch_states)
    {
        if (touch_states & 0x00000001)
        {
            touch_number++;
        }
        touch_states >>= 1;
    }

    /* Check touch on the selected electrode to adjust the filtering */
    if (electrode->delta > (ram->noise * rom->signal_to_noise_ratio))
    {
        touch_number++;
    }

    if (ram->filter_state != NT_FILTER_STATE_INIT)
    {
        /* Filtering the delta and baseline by own smooth filter parameters */
        if( ram -> smooth_baseline_accu_count < baseline_accu_max_count )
        {
            ram->smooth_baseline_accu += signalNoSmooth;
            ram->smooth_baseline_accu_count++;
        }
        else
        {
            /* Compute average activation over baseline accumulation to boost accuracy */
            ram->smooth_baseline_accu += ram->smooth_baseline_accu_count / 2;
            ram->smooth_baseline_accu /= ram->smooth_baseline_accu_count;
            
            ram->smooth_baseline =_nt_asym_smooth_process(&electrode->keydetector_data.mbw->base_smooth, (int32_t)ram->smooth_baseline_accu, ram->smooth_baseline);
            ram->smooth_baseline_accu = 0;
            ram->smooth_baseline_accu_count = 0;
        }
    }

    /* Reset the delta if negative value happend for other cases transfer to local variables */
    if(s_delta < 0)
    {
      delta = 0;
    }
    else
    {
      s_delta = delta;
    }
    
    _nt_electrode_set_delta(electrode, delta);

    /* Baseline tracking process */
    /* Finding the minimum in window, and transferred to baseline at end of window periode */
    /* Signals smaller than baseline are transferred by smooth filter */
    if(ram->baseline_cnt < ram->baseline_cnt_max * ram->baseline_add_no_touch)
    {
        /* Signal is in window and search for the minimum baseline value */
        if(ram->smooth_baseline < ram->smooth_baseline_min)
        {
            ram->smooth_baseline_min = ram->smooth_baseline;
        }
        
        /* Baseline follows faster the negative change */
        if( ram->smooth_baseline < electrode->baseline )
        {
            electrode->baseline = ram->smooth_baseline;           
            /* Recompute with new baseline */         
            s_delta = ((int32_t)signal - (int32_t)electrode->baseline);          
            delta = (int32_t)_nt_filter_pos(s_delta);          
            delta = _nt_electrode_delta_normalization_process(electrode, delta);          
            if(s_delta > 0)
            {
                s_delta = delta;
            }
        }

        /* Signal is in window and search for the minimum/maximum values */
        if(signal > ram->smooth_signal_max)
        {
          ram->smooth_signal_max = signal;
        }

        if(signal < ram->smooth_signal_min)
        {
          ram->smooth_signal_min = signal;
        }

        /* Adjusting counter speed: when no touch or number of touches > touch_number */
        if((touch_number == 0U) || (touch_number > rom->touch_limit))
        {
          ram->baseline_cnt += ram->baseline_add_no_touch;
        }
        else
        {
          ram->baseline_cnt += ram->baseline_add_touch;
        }
    }
    else
    {
        /* Only accept new minimum if the dynamic range is below 2*min_noise_limit */
      if( ram->smooth_signal_max - ram->smooth_signal_min < (( uint16_t )rom->min_noise_limit << 2))
      {
        electrode->baseline = ram->smooth_baseline_min;

            /* Recompute delta and s_delta */
            s_delta = ((int32_t)signal - (int32_t)electrode->baseline);
            delta = (int32_t)_nt_filter_pos(s_delta);
            delta = _nt_electrode_delta_normalization_process(electrode, delta);
            if(s_delta > 0)
            {
                s_delta = delta;
            }
        
        }

        /* Reinint tracking minimum baseline value and baseline counter */
        ram->smooth_baseline_min = 65535U;
        ram->smooth_signal_min = 65535U;
        ram->smooth_signal_max = 0u;
        ram->baseline_cnt = 0U;
    }


    if (ram->filter_state == NT_FILTER_STATE_INIT)
    {
        _nt_reset_keydetector_mbw_reset(
            electrode, signalNoSmooth, _nt_electrode_get_flag(electrode, (int32_t)NT_ELECTRODE_AFTER_INIT_TOUCH_FLAG));

        ram->filter_state = NT_FILTER_STATE_RUN;
    }
    else
    {     
        uint32_t prev_signal = (uint32_t)_nt_electrode_get_signal(electrode);
        signal = (uint32_t)(( int32_t) electrode->baseline + s_delta );   /* Signal reconstruction, using the normalized delta */

        uint32_t iir_signal  = _nt_filter_iir_process(&rom->signal_filter, signal, prev_signal);
        iir_signal  = _nt_electrode_shielding_process(electrode, iir_signal);

        _nt_electrode_set_signal(electrode, iir_signal);
        _nt_keydetector_mbw_signal_track(electrode, rom, ram, (uint16_t)iir_signal);
    }
}

static void _nt_reset_keydetector_mbw_reset(struct nt_electrode_data *electrode, uint32_t signal, uint32_t touch)
{
    NT_ASSERT(electrode->rom->keydetector_interface == &nt_keydetector_mbw_interface);

    const struct nt_keydetector_mbw *rom = electrode->rom->keydetector_params.mbw;
    struct nt_keydetector_mbw_data *ram  = electrode->keydetector_data.mbw;

    _nt_electrode_clear_flag(
        electrode, (uint32_t)NT_ELECTRODE_LOCK_BASELINE_REQ_FLAG | (uint32_t)NT_ELECTRODE_LOCK_BASELINE_FLAG |
                       (uint32_t)NT_ELECTRODE_DIGITAL_RESULT_ONLY_FLAG | (uint32_t)NT_ELECTRODE_AFTER_INIT_TOUCH_FLAG);

    if (!(bool)touch)
    {
        _nt_electrode_set_signal(electrode, signal);
        ram->base_avrg_init.n2_order = rom->base_avrg.n2_order;
        (void)(int32_t) _nt_filter_moving_average_init(&ram->base_avrg_init, &ram->base_avrg, (uint16_t)signal);

        electrode->baseline = electrode->signal;

        (void)(int32_t)
            _nt_filter_moving_average_init(&ram->noise_avrg_init, &ram->noise_avrg, (uint16_t)rom->min_noise_limit);
        /* fast noise filter */
        (void)(int32_t) _nt_filter_moving_average_init(&ram->f_noise_avrg_init, &ram->f_noise_avrg,
                                                       (uint16_t)(rom->min_noise_limit));

        ram->predicted_signal = signal + rom->min_noise_limit * rom->signal_to_noise_ratio;
        (void)(int32_t) _nt_filter_moving_average_init(&rom->non_activity_avrg, &ram->predicted_signal_avrg,
                                                       (uint16_t)ram->predicted_signal);

        ram->noise   = rom->min_noise_limit;
        ram->f_noise = rom->min_noise_limit;

        _nt_electrode_clear_flag(electrode, (int32_t)NT_ELECTRODE_LOCK_BASELINE_FLAG);
    }
    else
    {
        _nt_electrode_set_signal(electrode, signal);
        ram->base_avrg_init = rom->non_activity_avrg;

        (void)(int32_t)
            _nt_filter_moving_average_init(&ram->noise_avrg_init, &ram->noise_avrg, (uint16_t)rom->min_noise_limit);
        /* fast noise filter */
        (void)(int32_t)
            _nt_filter_moving_average_init(&ram->f_noise_avrg_init, &ram->f_noise_avrg, (uint16_t)rom->min_noise_limit);

        ram->predicted_signal = signal;
        (void)(int32_t) _nt_filter_moving_average_init(&ram->predicted_signal_avrg_init, &ram->predicted_signal_avrg,
                                                       (uint16_t)ram->predicted_signal);

        ram->noise   = rom->min_noise_limit;
        ram->f_noise = rom->min_noise_limit;

        _nt_electrode_set_flag(electrode, (int32_t)NT_ELECTRODE_AFTER_INIT_TOUCH_FLAG);
    }
}

static void _nt_keydetector_mbw_process(struct nt_electrode_data *electrode)
{
    NT_ASSERT(electrode->rom->keydetector_interface == &nt_keydetector_mbw_interface);

    const struct nt_keydetector_mbw *rom = electrode->rom->keydetector_params.mbw;
    struct nt_keydetector_mbw_data *ram  = electrode->keydetector_data.mbw;
    struct nt_kernel *system               = _nt_system_get();

    int32_t delta;
    if( electrode->delta_red_on )
    {
        delta = (int32_t)_nt_electrode_get_delta_red( electrode );
    }
    else
    {
        delta = (int32_t)_nt_electrode_get_delta(electrode);
    }

    uint16_t signal = delta + electrode->baseline;
    uint16_t sig_filter = signal;

    switch (_nt_electrode_get_last_status(electrode))
    {
        case (int32_t)NT_ELECTRODE_STATE_INIT:
            /* manage switch from electrode init to run phase */
            if (_nt_electrode_get_time_offset(electrode) >= system->rom->init_time)
            {
                ram->entry_event_cnt = 0;
                ram->deadband_cnt    = (int32_t)rom->deadband_cnt;

                if ((bool)_nt_electrode_get_flag(electrode, (int32_t)NT_ELECTRODE_AFTER_INIT_TOUCH_FLAG))
                {
                    if (sig_filter < (ram->noise * rom->signal_to_noise_ratio))
                    {
                        sig_filter = (uint16_t)(ram->noise) * (uint16_t)(rom->signal_to_noise_ratio);
                    }
                    (void)(int32_t) _nt_filter_moving_average_init(&ram->predicted_signal_avrg_init,
                                                                   &ram->predicted_signal_avrg, sig_filter);
                    _nt_electrode_set_status(electrode, (int32_t)NT_ELECTRODE_STATE_TOUCH);
                    _nt_keydetector_mbw_lock_baseline(electrode, rom, ram, signal);
                }
                else
                {
                    _nt_electrode_set_status(electrode, (int32_t)NT_ELECTRODE_STATE_RELEASE);
                    (void)(int32_t) _nt_filter_moving_average_init(&rom->non_activity_avrg, &ram->predicted_signal_avrg,
                                                                   (uint16_t)(ram->predicted_signal));
                    _nt_keydetector_mbw_unlock_baseline(electrode, rom, ram, signal);

                    _nt_reset_keydetector_mbw_reset(
                        electrode, (uint32_t)signal,
                        _nt_electrode_get_flag(
                            electrode,
                            (int32_t)
                                NT_ELECTRODE_AFTER_INIT_TOUCH_FLAG)); /* Initial signal warm-up (settling) Completed */
                }
            }
            break;
        case (int32_t)NT_ELECTRODE_STATE_TOUCH:
            if ((delta < (int32_t)_nt_filter_pos(
                             (int32_t)((int32_t)ram->predicted_signal - (int32_t)electrode->baseline) * 4 / 5) &&
                 (ram->deadband_cnt == 0))) /* 80% release thresh */
            {
                ram->entry_event_cnt = 0;
                ram->deadband_cnt    = (int16_t)(rom->deadband_cnt);
                _nt_electrode_set_status(electrode, (int32_t)NT_ELECTRODE_STATE_RELEASE);
                (void)(int32_t) _nt_filter_moving_average_init(&rom->non_activity_avrg, &ram->predicted_signal_avrg,
                                                               (uint16_t)(ram->predicted_signal));
                 /* is init touch */
                 if ((bool)_nt_electrode_get_flag(electrode, (int32_t)NT_ELECTRODE_AFTER_INIT_TOUCH_FLAG))
                 {
                     if (!(bool)_nt_electrode_get_flag(electrode, (int32_t)NT_ELECTRODE_LOCK_BASELINE_REQ_FLAG))
                     {
                         _nt_keydetector_mbw_unlock_baseline(electrode, rom, ram, signal);
                     }
                 }
            }
            else
            {
                ram->entry_event_cnt = 0;
                if ((bool)(ram->deadband_cnt))
                {
                    ram->deadband_cnt--;
                }
            }

            break;
        case (int32_t)NT_ELECTRODE_STATE_RELEASE:
            if ((ram->deadband_cnt == 0) &&
                ( delta > (uint16_t)(ram->noise * rom->signal_to_noise_ratio )))
            {
                ram->entry_event_cnt++;

                if (ram->entry_event_cnt > (int32_t)(rom->entry_event_cnt)) /* debouncing */
                {
                    ram->entry_event_cnt = 0;
                    ram->deadband_cnt    = (int32_t)(rom->deadband_cnt);
                if (!(bool)_nt_electrode_get_flag(electrode, (int32_t)NT_ELECTRODE_LOCK_BASELINE_FLAG))
                {
                    _nt_keydetector_mbw_lock_baseline(electrode, rom, ram, signal);
                }

                    (void)(int32_t) _nt_filter_moving_average_init(
                        &ram->predicted_signal_avrg_init, &ram->predicted_signal_avrg,
                        _nt_filter_deadrange_p(signal, electrode->baseline,
                                               (uint16_t)(ram->noise * rom->signal_to_noise_ratio)));
                    _nt_electrode_set_status(electrode, (int32_t)NT_ELECTRODE_STATE_TOUCH);
                }
            }

            else
            {
                ram->entry_event_cnt = 0;

                if ((bool)_nt_electrode_get_flag(electrode, (int32_t)NT_ELECTRODE_LOCK_BASELINE_FLAG))
                {
                    if ((bool)ram->deadband_cnt)
                    {
                        ram->deadband_cnt--;
                        break;
                    }
                }
                else
                {
                    if ((bool)(ram->recovery_cnt))
                    {
                        ram->recovery_cnt--;
                        if (ram->recovery_cnt == 0)
                        {
                            ram->base_avrg_init = rom->base_avrg;
                            (void)(int32_t) _nt_filter_moving_average_init(&ram->base_avrg_init, &ram->base_avrg,
                                                                           electrode->baseline);
                        }
                        break;
                    }
                }

                ram->deadband_cnt = 0;
                ram->recovery_cnt = 0;

                if (_nt_electrode_get_flag(electrode, (int32_t)NT_ELECTRODE_LOCK_BASELINE_REQ_FLAG))
                {
                    _nt_keydetector_mbw_lock_baseline(electrode, rom, ram, signal);
                }
                if ((_nt_electrode_get_flag(electrode, (uint32_t)NT_ELECTRODE_LOCK_BASELINE_REQ_FLAG | (uint32_t)NT_ELECTRODE_LOCK_BASELINE_FLAG) == (uint32_t)NT_ELECTRODE_LOCK_BASELINE_FLAG))
                {
                    _nt_keydetector_mbw_unlock_baseline(electrode, rom, ram, signal);
                }
            }
            break;
        default:
            /*MISRA rule 16.4*/
            break;
    }
}

/* This needs to be looked at in the context of the delta_normalization */
static void _nt_keydetector_mbw_signal_track(struct nt_electrode_data *electrode,
                                               const struct nt_keydetector_mbw *rom,
                                               struct nt_keydetector_mbw_data *ram,
                                               uint16_t signal)
{
    if (!(bool)_nt_electrode_get_flag(electrode, (int32_t)NT_ELECTRODE_LOCK_BASELINE_FLAG))
    {
        if (_nt_filter_pos(((int32_t)signal - (int32_t)electrode->baseline)) <
            (ram->noise * rom->signal_to_noise_ratio))
        {
            /* Noise increased only when positive signal increase */
            ram->noise = _nt_filter_moving_average_process(
                &ram->noise_avrg_init, &ram->noise_avrg,
                _nt_filter_limit_u((int32_t)_nt_filter_pos(((int32_t)signal - (int32_t)electrode->baseline)),
                                   (uint16_t)rom->min_noise_limit,
                                   (uint16_t)(rom->min_noise_limit * rom->signal_to_noise_ratio * 4U)));

            ram->f_noise = _nt_filter_moving_average_process(
                &ram->f_noise_avrg_init, &ram->f_noise_avrg,
                _nt_filter_limit_u((int32_t)_nt_filter_pos(((int32_t)signal - (int32_t)electrode->baseline)),
                                   (uint16_t)rom->min_noise_limit,
                                   (uint16_t)(rom->min_noise_limit * (uint32_t)rom->signal_to_noise_ratio * 4U)));

            /* Noise level recovery */
            if ((ram->noise > (rom->min_noise_limit)) && (ram->noise > (ram->f_noise)))
            {
                /* if fast noise drops 2x time faster than slowly accumulated value, then recovery slow noise level */
                if ((ram->noise - rom->min_noise_limit) > 2U * (ram->f_noise - rom->min_noise_limit))

                {
                    (void)(int32_t)
                        _nt_filter_moving_average_init(&ram->noise_avrg_init, &ram->noise_avrg, (uint16_t)ram->f_noise);
                }
            }
        }
        
        /* Deadband aux. calculation */
        uint32_t deadband = (electrode->baseline + (ram->noise * rom->signal_to_noise_ratio));
        if(deadband > 65000U)
        {
          deadband = 65000U; // Ceil the deadband
        }
        ram->deadband_h = (uint16_t)deadband;   
    }

    /* Predicted Signal tracking in released state */
    if (_nt_electrode_get_last_status(electrode) != (int32_t)NT_ELECTRODE_STATE_TOUCH)
    {
        uint16_t sig_filter = signal;

        sig_filter = _nt_filter_abs((int16_t)signal - (int16_t)electrode->baseline);

        if (sig_filter < (ram->noise * rom->signal_to_noise_ratio))
        {
            sig_filter = (uint16_t)(ram->noise * rom->signal_to_noise_ratio);
        }

        sig_filter += electrode->baseline;

        sig_filter = (uint16_t)_nt_filter_limit_u((int32_t)sig_filter, 0, 65535);
        ram->predicted_signal =
            _nt_filter_moving_average_process(&rom->non_activity_avrg, &ram->predicted_signal_avrg, sig_filter);
    }
    else /* In touched state */
    {
        ram->predicted_signal =
            _nt_filter_deadrange_p((uint16_t)_nt_filter_moving_average_process(&ram->predicted_signal_avrg_init,
                                                                               &ram->predicted_signal_avrg, signal),
                                   electrode->baseline, (uint16_t)(ram->noise * rom->signal_to_noise_ratio));
    }
}

static void _nt_keydetector_mbw_reset(struct nt_electrode_data *electrode)
{
    uint32_t signal = _nt_electrode_get_raw_signal(electrode);
    signal          = _nt_electrode_normalization_process(electrode, signal);

    _nt_reset_keydetector_mbw_reset(electrode, signal, 0);
}

static void _nt_keydetector_mbw_lock_baseline(struct nt_electrode_data *electrode,
                                                const struct nt_keydetector_mbw *rom,
                                                struct nt_keydetector_mbw_data *ram,
                                                uint16_t signal)
{
  _nt_electrode_set_flag(electrode, (int32_t)NT_ELECTRODE_LOCK_BASELINE_FLAG);
}

static void _nt_keydetector_mbw_unlock_baseline(struct nt_electrode_data *electrode,
                                                  const struct nt_keydetector_mbw *rom,
                                                  struct nt_keydetector_mbw_data *ram,
                                                  uint16_t signal)
{
  if ((bool)_nt_electrode_get_flag(electrode, (uint32_t)NT_ELECTRODE_AFTER_INIT_TOUCH_FLAG))
  {
    _nt_electrode_clear_flag(electrode, (int32_t)NT_ELECTRODE_AFTER_INIT_TOUCH_FLAG);
  }


  _nt_electrode_clear_flag(electrode, (int32_t)NT_ELECTRODE_LOCK_BASELINE_FLAG);
}

/*public*/
void nt_reset_keydetector_mbw_reset(const struct nt_electrode *electrode, uint32_t signal, uint32_t touch)
{
  NT_ASSERT(electrode != NULL);

  struct nt_electrode_data * electrode_data = _nt_electrode_get_data(electrode);
  NT_ASSERT(electrode_data != NULL);

  _nt_reset_keydetector_mbw_reset(electrode_data, signal, touch);
}

