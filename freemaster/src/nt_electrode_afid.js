/*
 * Copyright 2013 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021, 2024 NXP
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
 
 /* NTElectrode class */
  
function NTElectrode_afid(module, symbol, addr, type)
{
    var ascAddr, ascSymbol;

    // call base when not typed dynamically (see change_class_type()) 
    // otherwise, the base object is already constructed
    if(!this._dynamically_typed)
        NTElectrode(module, symbol, addr, null)
}

// AFID integration_value is common to all AFID electrodes, we define it here to avoid duplicating it in each
NTElectrode_afid.prototype.DefineAfidSlowSignalVariable = function(vname)
{
    debug_assert(this._keydetsymbol, "need typed keydetector");
    return pcm_define_variable2_new(vname, this._keydetsymbol + "->slow_signal", "UINT16");
}

// AFID integration_value is common to all AFID electrodes, we define it here to avoid duplicating it in each
NTElectrode_afid.prototype.DefineAfidIntegrationValueVariable = function(vname)
{
    debug_assert(this._keydetsymbol, "need typed keydetector");
    return pcm_define_variable2_new(vname, this._keydetsymbol + "->integration_value", "SINT16");
}

// AFID touch_threshold is common to all AFID electrodes, we define it here to avoid duplicating it in each
NTElectrode_afid.prototype.DefineAfidTouchThreshold = function(vname)
{
    debug_assert(this._keydetsymbol, "need typed keydetector");
    return pcm_define_variable2_new(vname, this._keydetsymbol + "->touch_threshold", "SINT16");
}

// AFID touch_reset_counter is common to all AFID electrodes, we define it here to avoid duplicating it in each
NTElectrode_afid.prototype.DefineAfidTouchResetCounter = function(vname)
{
    debug_assert(this._keydetsymbol, "need typed keydetector");
    return pcm_define_variable_new(vname, this._keydetsymbol + "->touch_reset_counter");
}

// AFID release_reset_counter is common to all AFID electrodes, we define it here to avoid duplicating it in each
NTElectrode_afid.prototype.DefineAfidReleaseResetCounter = function(vname)
{
    debug_assert(this._keydetsymbol, "need typed keydetector");
    return pcm_define_variable_new(vname, this._keydetsymbol + "->release_reset_counter");
}

// ASC threshold is common to all AFID electrodes, we define it here to avoid duplicating it in each 
NTElectrode_afid.prototype.DefineAfidAscThresholdVariable = function(vname)
{
    debug_assert(this._keydetsymbol, "need typed keydetector");
    return pcm_define_variable2_new(vname, this._keydetsymbol + "->asc.dest_threshold", "SINT16");
}

// ASC noise reset is common to all AFID electrodes, we define it here to avoid duplicating it in each 
NTElectrode_afid.prototype.DefineAfidAscNoiseResetsVariable = function(vname)
{
    debug_assert(this._keydetsymbol, "need typed keydetector");
    return pcm_define_variable2_new(vname, this._keydetsymbol + "->asc.noise_resets", "SINT16");
}

// ASC maximal reset is common to all AFID electrodes, we define it here to avoid duplicating it in each 
NTElectrode_afid.prototype.DefineAfidAscMaxResetsVariable = function(vname)
{
    debug_assert(this._keydetsymbol, "need typed keydetector");
    return pcm_define_variable2_new(vname, this._keydetsymbol + "->asc.max_resets", "SINT16");
}

// ASC threshold is common to all AFID electrodes, we define it here to avoid duplicating it in each 
NTElectrode_afid.prototype.DefineAfidThresholdVariable = function(vname)
{
    debug_assert(this._keydetsymbol, "need typed keydetector");
    return pcm_define_variable2_new(vname, this._keydetsymbol + "->touch_threshold", "UINT32");
}


NTElectrode_afid.prototype.OnLinkClicked = function()
{
    var vname_rawsignal  = this._name + "_raw_signal";
    var vname_fast_signal  = this._name + "_fast_signal";
    var vname_slow_signal  = this._name + "_slow_signal";
    var vname_baseline = this._name + "_baseline";
    var vname_afid_trc = this._name + "_touch_rcount";
    var vname_afid_rrc = this._name + "_release_rcount";
    var vname_afid_i = this._name + "_afid_integration";
    var vname_reset_threshold = this._name + "_reset_threshold";
    
    this.DefineRawSignalVariable(vname_rawsignal).then((res) => {
      if(!res)
        return;
      var st = this.DefineSignalVariable(vname_fast_signal).then((res) => {
        if(!res)
          return;
        var st = this.DefineAfidSlowSignalVariable(vname_slow_signal).then((res) => {
          if(!res)
            return;
          var st = this.DefineBaselineVariable(vname_baseline).then((res) => {
            if(!res)
              return;
            var st = this.DefineAfidTouchResetCounter(vname_afid_trc).then((res) => {
              if(!res)
                return;
              var st = this.DefineAfidReleaseResetCounter(vname_afid_rrc).then((res) => {
                if(!res)
                  return;
                var st = this.DefineAfidIntegrationValueVariable(vname_afid_i).then((res) => {
                  if(!res)
                    return;
                  var st = this.DefineAfidThresholdVariable(vname_reset_threshold).then((res) => {
                                  if(res)
                                  {
                                      // scope variables
                                      var vars = [ 
                                          {"variable":vname_rawsignal, "visible":true, "color":3026413, "y_block":0},
                                          {"variable":vname_fast_signal, "visible":true, "color":4688896, "y_block":0 }, 
                                          {"variable":vname_slow_signal, "visible":true, "color":11098392, "y_block":0 }, 
                                          {"variable":vname_baseline, "visible":true, "color":2260467, "y_block":0 },
                                          {"variable":vname_afid_i, "visible":true, "color":9055202, "y_block":1 },
                                          {"variable":vname_afid_trc, "visible":true, "color":2104737, "y_block":2 },
                                          {"variable":vname_afid_rrc, "visible":true, "color":9514086, "y_block":2 },
                                          {"variable":vname_reset_threshold, "visible":true, "color":16766720, "y_block":3 }
                                      ]; 
                                  
                                      // scope traces color
                                      var trace_colors = [ 
                                          0xcfacdb,
                                          0x00ff00, 
                                          0x000000, 
                                          0x8B008B, 
                                          0xFF8C00, 
                                          0xff0000,
                                      ];
                                  
                                      // scope Y-blocks
                                      var yblocks = [
                                          { "laxis_label":"signals", "join_class":0, "laxis_min_auto":true, "laxis_max_auto":true },
                                          { "laxis_label":"AFID Integration", "join_class":1, "laxis_min_auto":true, "laxis_max_auto":true },
                                          { "laxis_label":"AFID Resets", "join_class":2, "laxis_min_auto":true, "laxis_max_auto":true },
                                          { "laxis_label":"AFID Dynamic Threshold", "join_class":3, "laxis_min_auto":true, "laxis_max_auto":true }
                                      ];
                                      // scope definition
                                      var def = {};
                                      def["var_info"] = vars;
                                      def["yblock_info"] = yblocks;
                                      def["var_color"] = trace_colors;
                                      def["scope_period"] = 0.025;
                                      def["auto_delete"] = true;
                                      def["href"] = DOC_PATH+"group__electrodes.html#details";
                                  
                                      this.CreateScope(def);
                                  }
                                  return st;
                  });
                  return st;
                });
                return st;
              });
              return st;
            });
            return st;
          });
          return st;
        });
        return st;
      });
      return st;
    })
    .catch((err) => {
      this.OnError(err);
    });
}
NTElectrode_afid.prototype.GetKeydetGui = function(shieldding)
{
  var str = "";
  var baseid = this._id+shieldding+"keydet";
  str += "<div class=\"top-modal-section\">";
  str += "<p class=\"modal-headline\">keydetectortype: "+this._keydettype+"</p>";
  str += "<p class=\"modal-state-ok\"><img src=\"./img/detected.svg\" class=\"icon-detected\">detected</p>";
  str += "</div>";
  str += "<div class=\"modules-table\">";
  str += "<div class=\"modules-table-headline modules-table-headline-240\">";
  str += "<div class=\"mhl-raw-signal\"><p>filt_st</p></div>";
  str += "<div class=\"mhl-signal\"><p>integr_val</p></div>";
  str += "<div class=\"mhl-signal\"><p>touch_trshold</p></div>";
  str += "<div class=\"mhl-signal\"><p>touch_res_cnt</p></div>";
  str += "<div class=\"mhl-signal\"><p>rel_res_cnt</p></div>";
  str += "<div class=\"mhl-signal\"><p>ASC_max_resets</p></div>";
  str += "</div>";
  str += "<div class=\"modules-table-line modules-line-headline-240\">";
  str += "<div class=\"mtl-raw-signal\" id=\""+baseid+"keydetector_data->filter_state\"><p>0</p></div>";
  str += "<div class=\"mtl-signal\" id=\""+baseid+"keydetector_data->integration_value\"><p>0</p></div>";
  str += "<div class=\"mtl-signal\" id=\""+baseid+"keydetector_data->touch_threshold\"><p>0</p></div>";
  str += "<div class=\"mtl-signal\" id=\""+baseid+"keydetector_data->touch_reset_counter\"><p>0</p></div>";
  str += "<div class=\"mtl-signal\" id=\""+baseid+"keydetector_data->release_reset_counter\"><p>0</p></div>";
  str += "<div class=\"mtl-signal\" id=\""+baseid+"keydetector_data->asc.max_resets\"><p>0</p></div>";
  str += "<div class=\"cistic\"></div>";
  str += "</div>";
  str += "</div>";
  return str;
}

NTElectrode_afid.prototype.UpdateGuiKeydetRaw = function(shieldding)
{
    var idbase = this._id+shieldding+"keydet";
    
    var subids = ["keydetector_data->filter_state", "keydetector_data->integration_value", "keydetector_data->touch_reset_counter", "keydetector_data->release_reset_counter", "keydetector_data->asc.max_resets"]; 
    var proms = [];
    for( subidindex in subids)
    {
      var p = this.GetVarIdNew(shieldding+subids[subidindex], idbase+subids[subidindex]);
      proms.push(p)
    }
    var p = this.GetVarIdNew(shieldding + "keydetector_data->touch_threshold", idbase+subids[subidindex]);
    proms.push(p);
    
    Promise.all(proms).then((vals) => {
      for(res in vals)
      {
        var obj = document.getElementById(vals[res].objid);
        if(obj)
	{
          if(vals[res].objid.indexOf("touch_threshold")>0)//"touch_threshold" item has different format
            obj.innerHTML = "<p>"+vals[res].value+"</p>";
	  else
	    obj.innerHTML = "<p>"+ToSign16(vals[res].value)+"</p>";
	}
      }
    })
    .catch((err) => {
      this.OnError(err);
    });

}
