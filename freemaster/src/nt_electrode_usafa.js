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
 
 /* NTElectrode_usafa class */
  
function NTElectrode_usafa(module, symbol, addr, type)
{
    // call base when not typed dynamically (see change_class_type()) 
    // otherwise, the base object is already constructed
    if(!this._dynamically_typed)
        NTElectrode(module, symbol, addr, null)
}

NTElectrode_usafa.prototype.OnLinkClicked = function()
{
    var vname_measured_iir     = this._name + "_measured_iir";
    var vname_predicted  = this._name + "_predicted"; 
    var vname_baseline   = this._name + "_baseline";
    var vname_noise      = this._name + "_noise";
    var vname_evnt_cnt   = this._name + "_entry_event_cnt";
    var vname_deadband   = this._name + "_deadband_cnt";
    var vname_deadband_h = this._name + "_deadband_h";
    const vname_measured_iir_prom = this.DefineSignalVariable(vname_measured_iir);
    const vname_predicted_prom = this.DefineFilteredSignalVariable(vname_predicted);
    const vname_baseline_prom = this.DefineBaselineVariable(vname_baseline);
    const vname_noise_prom = this.DefineNoiseVariable(vname_noise);
    const vname_evnt_cnt_prom = this.DefineEventCntVariable(vname_evnt_cnt);
    const vname_deadband_prom = this.DefineDeadBandVariable(vname_deadband);
    const vname_deadband_h_prom = this.DefineDeadBandHighVariable(vname_deadband_h);
    
    Promise.all([vname_measured_iir_prom, vname_predicted_prom, vname_baseline_prom, vname_noise_prom, vname_evnt_cnt_prom, vname_deadband_prom, vname_deadband_h_prom]).then((res) => {
          if(res)
          {
            // scope variables
            var vars = [ 
                {"variable":vname_measured_iir, "visible":true, "color":3026413, "y_block":0 }, 
                {"variable":vname_predicted, "visible":true, "color":4688896, "y_block":0 }, 
                {"variable":vname_baseline, "visible":true, "color":11098392, "y_block":0 },
                {"variable":vname_deadband_h, "visible":true, "color":2260467, "y_block":0 },
                {"variable":vname_noise, "visible":true, "color":9055202, "y_block":1 },
                {"variable":vname_evnt_cnt, "visible":true, "color":2104737, "y_block":2 },
                {"variable":vname_deadband, "visible":true, "color":9514086, "y_block":2 },
            ]; 
          
            // scope Y-blocks
            var yblocks = [
                { "laxis_label":"signals", "join_class":0, "laxis_min_auto":true, "laxis_max_auto":true },
                { "laxis_label":"noise", "join_class":1, "laxis_min_auto":true, "laxis_max_auto":true },
                { "laxis_label":"counters", "join_class":2, "laxis_min_auto":true, "laxis_max_auto":true },
            ];
          
            // scope definition
            var def = {};
            def["var_info"] = vars;
            def["yblock_info"] = yblocks;
            def["scope_period"] = 0.025;
            def["auto_delete"] = true;
            def["href"] = DOC_PATH+"group__electrodes.html#details";
            this.CreateScope(def);
          }
    })
    .catch((err) => {
      this.OnError(err);
    });
}
NTElectrode_usafa.prototype.GetKeydetGui = function(shieldding)
{
  var str = "";
  var baseid = this._id+shieldding+"keydet";
  str += "<div class=\"top-modal-section\">";
  str += "<p class=\"modal-headline\">keydetectortype: "+this._keydettype+"</p>";
  str += "<p class=\"modal-state-ok\"><img src=\"./img/detected.svg\" class=\"icon-detected\">detected</p>";
  str += "</div>";
  str += "<div class=\"modules-table\">";
  str += "<div class=\"modules-table-headline modules-table-headline-240\">";
  str += "<div class=\"mhl-raw-signal\"><p>filtered_signal</p></div>";
  str += "<div class=\"mhl-signal\"><p>noise</p></div>";
  str += "</div>";
  str += "<div class=\"modules-table-line modules-line-headline-240\">";
  str += "<div class=\"mtl-raw-signal\" id=\""+baseid+"keydetector_data->predicted_signal\"><p>0</p></div>";
  str += "<div class=\"mtl-signal\" id=\""+baseid+"keydetector_data->noise\"><p>0</p></div>";
  str += "<div class=\"cistic\"></div>";
  str += "</div>";
  str += "</div>";
  return str;
}
NTElectrode_usafa.prototype.UpdateGuiKeydetRaw = function(shieldding)
{
    var idbase = this._id+shieldding+"keydet";
    
    var subids = ["keydetector_data->predicted_signal", "keydetector_data->noise"];
    var proms = [];
    for( subidindex in subids)
    {
      var p = this.GetVarIdNew(shieldding+subids[subidindex], idbase+subids[subidindex]);
      proms.push(p)
    }
    
    Promise.all(proms).then((vals) => {
      for(res in vals)
      {
        var obj = document.getElementById(vals[res].objid);
        if(obj)
          obj.innerHTML = "<p>"+vals[res].value+"</p>";
      }
    })
    .catch((err) => {
      this.OnError(err);
    });
}

