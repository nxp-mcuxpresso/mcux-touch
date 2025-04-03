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

function OnError(msg, descr)
{
      if(msg.hasOwnProperty("msg"))
        debug_print(descr+":"+ msg.msg, true);
      else
        debug_print(descr+":"+ msg, true);
}

function GenerateVars(moduleid)
{
    var label = "nt_system";
    romSystemName = "nt_kernel_data.rom";
    var st = pcm_read_ptr_new(romSystemName).then((res) => {
      var nt_system_rom_ptr = res.data;
      // map nt_symbol type to the obtained pointer
      var tssvar = "_nt_system";
      var st = pcm.DefineSymbol(tssvar, nt_system_rom_ptr, "nt_system").then((res) => {
        
      })
      return st;
    })
    .catch((err) => {
	OnError(err, "Failed GenerateVars");

    });
    
    var i=0;
}
