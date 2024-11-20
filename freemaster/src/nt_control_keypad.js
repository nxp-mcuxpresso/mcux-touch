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
 
 /* NTControl_keypad class */

function NTControl_keypad(symbol, addr)
{
    // call base when not typed dynamically (see change_class_type()) 
    // otherwise, the base object is already constructed
    this.groupsize = 0;
    if(!this._dynamically_typed)
        NTControl(symbol, addr)

    this._gui_id0 = get_unique_id(); // span
    // define RAM type
    var st = pcm.DefineSymbol(this._symbol, this._addr, "nt_control_data*").then((res) => {
      var st = pcm_read_ptr_new(this._symbol + "->data").then((res) => {
        var st = pcm.DefineSymbol(this._symbol + "->data", res.data, "nt_control_keypad_data").then((res) => {
          var st = pcm_read_ptr_new(this._symbol + "->rom->control_params").then((res) => {
            var st = pcm.DefineSymbol(this._symbol + "->rom->control_params", res.data, "nt_control_keypad*").then((res) => {
              var st = pcm_read_var_new(this._symbol + "->rom->control_params->groups_size", 1).then((res) => {
                debug_print("The Keypad Control has been created.", false);
                this.groupsize = res.data;
                
                this.NTControlFinish();
              })
              return st;
            })
            return st;
          })
          return st;
        })
        return st;
      })
      return st;
    })
    .catch((err) => {
      this.OnError(err);
    });
  return st;
}

NTControl_keypad.prototype.GetKeyState = function(callback)
{
    var st = pcm.ReadUIntVariable(this._symbol + "->data.last_state", 4).then((res) => {
      callback(res.data);
    })
    .catch((err) => {
      this.OnError(err);
    });
}

NTControl_keypad.prototype.GetGuiTag = function()
{
    var str = "";
    str = "<img src=\"./img/keypad.svg\" class=\"perifery-icon\"/>";
    return str;
}

NTControl_keypad.prototype.GetGuiTags = function()
{
    var str_grouped_electrodes = "img/gui_yes_hidden.png";
    var grp_els = "no";

    // get maximum range
    if(pcm_read_var(this._symbol + "->rom->control_params->groups_size", 1) && pcm.LastVariable_vValue)
    {
        str_grouped_electrodes = "img/gui_yes.png";
        grp_els = "yes";
    }

    var str = "";
    str += "<div class=\"mtl-type\"><p>"+this._type+"</p></div>";
    str += "<div class=\"mtl-state\" id=\"" + this._gui_id0 + "st\"><div class=\"empty-touch\"></div></div>";
    str += "<div class=\"mtl-rate\" id=\"" +  this._gui_id0 + "autorep_rate\"><p>?</p></div>";
    str += "<div class=\"mtl-touch\"><div class=\"empty-touch\" id=\"" + this._gui_id0 + "TouchObj\"></div></div>";
    str += "<div class=\"mtl-group\"><p>"+grp_els+"</p><img id=" + this._gui_id0 + "GroupE src=\""+str_grouped_electrodes+"\" style=\"height:24px;width:24px;\"></div>";
    return str;  
}

NTControl_keypad.prototype.GetGuiTagsLabels = function()
{
    var str = "";
        str += "<div class=\"mhl-type\"><p>Type</p></div>";
        str += "<div class=\"mhl-state\"><p>State</p></div>";
        str += "<div class=\"mhl-rate\"><p>Autorepear rate</p></div>";
        str += "<div class=\"mhl-touch\"><p>Touch</p></div>";
        str += "<div class=\"mhl-group\"><p>Grouped electrodes</p></div>";
    return str;
}

NTControl_keypad.prototype.UpdateGui = function()
{
    var callbackclass = this;
    this.GetKeyState(function(kst) {
      var kobj = document.getElementById(callbackclass._gui_id0+"TouchObj");
      if(kobj)
      {
        kobj.className = (kst != 0) ? "full-touch" : "empty-touch";
      }
    });
    
    var st = pcm.ReadUIntVariable(this._symbol + "->data.last_state", 4).then((res) => {
      var kobj = document.getElementById(callbackclass._gui_id0+"st");
    
      if(kobj)
        kobj.innerHTML = "<p>0x"+ res.data.toString(16)+"</p>";
    })
    .catch((err) => {
      this.OnError(err);
    });
    
    var st = pcm.ReadUIntVariable(this._symbol + "->data.autorepeat_rate", 2).then((res) => {
      var kobj = document.getElementById(callbackclass._gui_id0+"autorep_rate");
      if(kobj)
        kobj.innerHTML = "<p>"+res.data.toString(10)+"</p>";
    })
    .catch((err) => {
      this.OnError(err);
    });
}

NTControl_keypad.prototype.DefineKeyStateVariable = function(vname) 
{
    
}

NTControl_keypad.prototype.OnLinkClicked = function()
{
  var pname = this._name + "_keys_state";
  return pcm_define_variable_new(pname, this._symbol + "->data.last_state").then((res) => {
    var ok = res;
    var callbackclass = this;

    // scope variables
    var vars = [ 
        {"variable":pname, "visible":true, "color":3026413, "y_block":0 } 
    ]; 

    // up to 7 electrodes, keypad may have more
    var ec = callbackclass._electrodes.length < 7 ? callbackclass._electrodes.length : 7;  
    var defArr = new Array();
    
    for(var e=0; e<callbackclass._electrodes.length; e++)
    {
        var ename = callbackclass._name + "_electrode" + e + "_signal";
        defArr[e] = callbackclass._electrodes[e].DefineSignalVariable(ename);
        
        var edef = { "variable":ename, "visible":true, "color":4688896*e, "y_block":1 }; 
        vars.push(edef);
    }
    Promise.all(defArr).then((res) => {
      // scope Y-blocks
      var yblocks = [
        { "laxis_label":"key state", "join_class":0, "laxis_min":0, "laxis_min_auto":false, "laxis_max_auto":true },
        { "laxis_label":"signal", "join_class":1, "laxis_min_auto":true, "laxis_max_auto":true },
      ];

      // scope definition
      var def = {};
      def["var_info"] = vars;
      def["yblock_info"] = yblocks;
      def["scope_period"] = 0.025;
      def["auto_delete"] = true;
      def["href"] = DOC_PATH+"group__keypad.html#details";

      var item = callbackclass._name + " Details";
      ok = ok && pcm.DefineOscilloscope(item, JSON.stringify(def));
    
      if(ok)
        pcm.SelectItem(item, "scope");
      else
        alert("Could not create graph with selected signals");
    }) 
    .catch((err) => {
      this.OnError(err);
    });
  });
}

