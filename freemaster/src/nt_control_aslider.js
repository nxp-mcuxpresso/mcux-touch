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
 
 /* NTControl_aslider class */

function NTControl_aslider(symbol, addr)
{
    // call base when not typed dynamically (see change_class_type()) 
    // otherwise, the base object is already constructed
    if(!this._dynamically_typed)
        NTControl(symbol, addr)

    this._gui_id0 = get_unique_id(); // 
    this._range = 100;

    // define RAM type
    var st = pcm.DefineSymbol(this._symbol, this._addr, "nt_control_data*").then((res) => {
      var st = pcm_read_ptr_new(this._symbol + "->data").then((res) => {
        var st = pcm.DefineSymbol(this._symbol + "->data", res.data, "nt_control_aslider_data").then((res) => {
          var st = pcm_read_ptr_new(this._symbol + "->rom->control_params").then((res) => {
            var st = pcm.DefineSymbol(this._symbol + "->rom->control_params", res.data, "nt_control_aslider*").then((res) => {
              var st = pcm_read_var_new(this._symbol + "->rom->control_params->range", 1).then((res) => {
                this._range = res.data;
                const prom1 = pcm_define_variable_bit_new(this._name + "->flags_direction", this._symbol + "->flags", 17);
                const prom2 = pcm_define_variable_bit_new(this._name + "->flags_movement", this._symbol + "->flags", 18);
                const prom3 = pcm_define_variable_bit_new(this._name + "->flags_touch", this._symbol + "->flags", 19);
                var proms = [];
                proms.push(prom1);
                proms.push(prom2);
                proms.push(prom3);
                var st = Promise.all(proms).then((vals) => {
                  debug_print("The ARotary Control has been created.", false);
                });
                this.NTControlFinish();
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
      return st;
    })
    .catch((err) => {
      this.OnError(err);
    });
  return st;
}

NTControl_aslider.prototype.GetRange = function()
{
    return this._range;
}

NTControl_aslider.prototype.GetPosition = function(callback)
{
    var st = pcm.ReadUIntVariable(this._symbol + "->data.position", 1).then((res) => {
      callback(res.data);
    })
    .catch((err) => {
      this.OnError(err);
    });

}

NTControl_aslider.prototype.GetGuiTag = function()
{
    var str = "";
    str = "<img src=\"./img/slider.svg\" class=\"perifery-icon\"/>";
    return str;
}

NTControl_aslider.prototype.GetGuiTagsLabels = function()
{
    var str = "";
    str += "<div class=\"mhl-type-sh\"><p>Type</p></div>";
    str += "<div class=\"mhl-position\"><p>Position</p></div>";
    str += "<div class=\"mhl-range\"><p>Range</p></div>";
    str += "<div class=\"mhl-touch\"><p>Touch</p></div>";
    str += "<div class=\"mhl-movement\"><p>Movement</p></div>";
    str += "<div class=\"mhl-direction\"><p>Direction</p></div>";
    str += "<div class=\"mhl-invalid-touch\"><p>Invalid touch</p></div>";
    return str;
}

NTControl_aslider.prototype.GetGuiTags = function()
{
    var str = "";
    str += "<div class=\"mtl-type-sh\"><p>"+this._type+"</p></div>";
    str += "<div class=\"mtl-position\" id=" + this._gui_id0 + "pos><p>0</p></div>";
    str += "<div class=\"mtl-range\" id=" +  this._gui_id0 + "range><p>0</p></div>";
    str += "<div class=\"mtl-touch\"><div class=\"empty-touch\" id=" + this._gui_id0 + "Touch></div></div>";
    str += "<div class=\"mtl-movement\"><p><img id=" + this._gui_id0 + "Movement class=\"movement-icon\" src=\"./img/movement.svg\"</p></div>";
    str += "<div class=\"mtl-direction\" id=" + this._gui_id0 + "Direction><p>right</p></div>";
    str += "<div class=\"mtl-invalid-touch\" ><p><img id=" + this._gui_id0 + "Invalid class=\"triangle-icon\" src=\"./img/triangle.svg\"</p></div>";
    return str;
}

NTControl_aslider.prototype.UpdateGui = function()
{
    var callbackclass = this;
    this.GetPosition(function(kst) {     
      var kobj = document.getElementById(callbackclass._gui_id0+"pos");    
      if(kobj)
      {
        kobj.innerHTML = "<p>" + kst.toString(10) + "</p>";       
      }
    });
    var rng = this.GetRange();
    kobj = document.getElementById(this._gui_id0+"range");    
    if(kobj)
        kobj.innerHTML = "<p>" + rng.toString(10) + "</p>";
    
    this.GetFlagsNew(function(flags) {     
      kobj = document.getElementById(callbackclass._gui_id0+"Touch");    
      if(kobj)
      {
        kobj.className = (flags & (1<<19)) ? "full-touch" : "empty-touch";  
      }

      kobj = document.getElementById(callbackclass._gui_id0+"Movement");    
      if(kobj)
      {
        kobj.src = (flags & (1<<18)) ? "img/movement.svg" : "img/gui_arrow_move_hidden.png"; 
      }

      kobj = document.getElementById(callbackclass._gui_id0+"Direction");    
      if(kobj)
      {
        kobj.innerHTML = (flags & (1<<17)) ? "<p>right</p>" : "<p>left</p>"; 
      }

      kobj = document.getElementById(callbackclass._gui_id0+"Invalid");    
      if(kobj)
      {
        kobj.src = (flags & (1<<16)) ? "img/triangle.svg" : "img/gui_warning_hidden.png"; 
      }
    });     
}

NTControl_aslider.prototype.OnLinkClicked = function()
{
  var pname = this._name + "_position";
  return pcm_define_variable_new(pname, this._symbol + "->data.position").then((res) => {
    var ok = res;
    var callbackclass = this;

    // scope variables
    var vars = [ 
        {"variable":pname, "visible":true, "color":3026413, "y_block":0 } 
    ]; 

    var fname = this._name + "->flags_direction";
    var fdef = { "variable":fname, "visible":true, "color":4688896, "y_block":2 };
    vars.push(fdef);

    fname = this._name + "->flags_movement";
    fdef = { "variable":fname, "visible":true, "color":11098392, "y_block":2 };
    vars.push(fdef);

    fname = this._name + "->flags_touch";
    fdef = { "variable":fname, "visible":true, "color":2260467, "y_block":2 };
    vars.push(fdef);

    // up to 4 electrode signals
    var el_count = callbackclass._electrodes.length;
    if(el_count > 4)
        el_count = 4;
    var defArr = new Array();
    for(var e=0; e<el_count; e++)
    {
        var ename = callbackclass._name + "_electrode" + e + "_signal";
        defArr[e] = callbackclass._electrodes[e].DefineSignalVariable(ename);
        
        var edef = { "variable":ename, "visible":true, "color":9055202*e, "y_block":1 }; 
        vars.push(edef);
    }
    Promise.all(defArr).then((res) => {
      // scope Y-blocks
      var yblocks = [
          { "laxis_label":"position", "join_class":0, "laxis_min":0, "laxis_max":callbackclass.GetRange(), "laxis_min_auto":false, "laxis_max_auto":false },
          { "laxis_label":"flags", "join_class":1, "laxis_min":0, "laxis_max":2, "laxis_min_auto":false, "laxis_max_auto":false },
          { "laxis_label":"signals", "join_class":2, "laxis_min_auto":true, "laxis_max_auto":true },
        
      ];

      // scope definition
      var def = {};
      def["var_info"] = vars;
      def["yblock_info"] = yblocks;
      def["scope_period"] = 0.025;
      def["auto_delete"] = true;
      def["href"] = DOC_PATH+"group__aslider.html#details";

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

