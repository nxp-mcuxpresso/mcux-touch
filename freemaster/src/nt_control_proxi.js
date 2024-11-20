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
 
 /* NTControl_proxi class */

function NTControl_proxi(symbol, addr)
{
    // call base when not typed dynamically (see change_class_type()) 
    // otherwise, the base object is already constructed
    if(!this._dynamically_typed)
        NTControl(symbol, addr)

    this._gui_id0 = get_unique_id(); // proxi image
    this._range = 100;

    // define RAM type
    var st = pcm.DefineSymbol(this._symbol, this._addr, "nt_control_data*").then((res) => {
      var st = pcm_read_ptr_new(this._symbol + "->data").then((res) => {
        var st = pcm.DefineSymbol(this._symbol + "->data", res.data, "nt_control_proxi_data").then((res) => {
          var st = pcm_read_ptr_new(this._symbol + "->rom->control_params").then((res) => {
            var st = pcm.DefineSymbol(this._symbol + "->rom->control_params", res.data, "nt_control_proxi*").then((res) => {
              var st = pcm_read_var_new(this._symbol + "->rom->control_params->range", 1).then((res) => {
                this._range = res.data;
                const prom1 = pcm_define_variable_bit_new(this._name + "->flags_direction", this._symbol + "->flags", 16);
                const prom2 = pcm_define_variable_bit_new(this._name + "->flags_movement", this._symbol + "->flags", 17);
                const prom3 = pcm_define_variable_bit_new(this._name + "->flags_touch", this._symbol + "->flags", 18);
                var proms = [];
                proms.push(prom1);
                proms.push(prom2);
                proms.push(prom3);
                var st = Promise.all(proms).then((vals) => {
                  debug_print("The Proxi Control has been created.", false);
                });
                this.NTControlFinish(); //here is defined this._name
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

NTControl_proxi.prototype.GetRange = function()
{
    return this._range;
}

NTControl_proxi.prototype.GetProximity = function(callback)
{
    var st = pcm.ReadUIntVariable(this._symbol + "->data.proximity", 1).then((res) => {
      callback(res.data);
    })
    .catch((err) => {
      this.OnError(err);
    });
}

NTControl_proxi.prototype.GetIndex = function(callback)
{
    var st = pcm.ReadUIntVariable(this._symbol + "->data.index", 1).then((res) => {
      callback(res.data);
    })
    .catch((err) => {
      this.OnError(err);
    });
}

NTControl_proxi.prototype.GetGuiTag = function()
{
    var str = "";
    str = "<img src=\"./img/proxi.svg\" class=\"perifery-icon\"/>";
    return str;
}

NTControl_proxi.prototype.GetGuiTagsLabels = function()
{
    var str = "";
  str += "<div class=\"mhl-type-sh\"><p>Type</p></div>";
  str += "<div class=\"mhl-position\"><p>Proximity</p></div>";
  str += "<div class=\"mhl-position\"><p>Index</p></div>";
  str += "<div class=\"mhl-touch\"><p>Touch</p></div>";
  str += "<div class=\"mhl-movement\"><p>Movement</p></div>";
  str += "<div class=\"mhl-direction\"><p>Direction</p></div>";
    return str;
}

NTControl_proxi.prototype.GetGuiTags = function()
{
  var str = "";
  str += "<div class=\"mtl-type-sh\"><p>"+this._type+"</p></div>";
  str += "<div class=\"mtl-position\" id=" + this._gui_id0 + "pos><p>0</p></div>";
  str += "<div class=\"mtl-position\" id=" +  this._gui_id0 + "ind><p>0</p></div>";
  str += "<div class=\"mtl-touch\"><div class=\"empty-touch\" id=" + this._gui_id0 + "Touch></div></div>";
  str += "<div class=\"mtl-movement\"><p><img id=" + this._gui_id0 + "Movement class=\"movement-icon\" src=\"./img/movement.svg\"</p></div>";
  str += "<div class=\"mtl-direction\" id=" + this._gui_id0 + "Direction><p>down</p></div>";
  return str;
}

NTControl_proxi.prototype.UpdateGui = function()
{
    var callbackclass = this;
    this.GetProximity(function(kst) {     
      var kobj = document.getElementById(callbackclass._gui_id0+"pos");    
      if(kobj)
      {
        kobj.innerHTML = "<p>" + kst.toString(10) + "</p>";       
      }
    });

    var flags = this.GetFlags();

    var rng = this.GetRange();
    kobj = document.getElementById(this._gui_id0+"range");    
    if(kobj)
        kobj.innerHTML = "<p>" + rng.toString(10) + "</p>";

    this.GetIndex(function(kst) {     
      var kobj = document.getElementById(callbackclass._gui_id0+"ind");    
      if(kobj)
      {
        kobj.innerHTML = "<p>" + kst.toString(10) + "</p>";       
      }
    });

    this.GetFlagsNew(function(flags) {     
      kobj = document.getElementById(callbackclass._gui_id0+"Touch");    
      if(kobj)
      {
        kobj.className = (flags & (1<<18)) ? "full-touch" : "empty-touch";  
      }

      kobj = document.getElementById(callbackclass._gui_id0+"Movement");    
      if(kobj)
      {
        kobj.src = (flags & (1<<17)) ? "img/movement.svg" : "img/gui_arrow_move_hidden.png"; 
      }

      kobj = document.getElementById(callbackclass._gui_id0+"Direction");    
      if(kobj)
      {
        kobj.innerHTML = (flags & (1<<16)) ? "<p>down</p>" : "<p>up</p>"; 
      }
    });     
        
}

NTControl_proxi.prototype.DefineFlagsVariable = function(vname, nn)
{
    return pcm_define_variable_new(vname, this._symbol + "->data.proxi_flags["+ nn +"]");
}

NTControl_proxi.prototype.DefineElVariable = function(vname, nn)
{
    return pcm_define_variable_new(vname, this._symbol + "->data.proxi_curr["+ nn +"]");
}

NTControl_proxi.prototype.OnLinkClicked = function()
{
    var callbackclass = this;
    var pname = "current proximity [0]";
    this.GetIndex(function(index) {
    return callbackclass.DefineElVariable(pname,0).then((res) => {
    var ok = res;

    // scope variables
    var vars = new Array();

    // up to 1 electrode signals
    var defArr = new Array();
    var ename = "current proximity [0]";
    defArr[0] = callbackclass.DefineElVariable(ename, 0);
    var edef = { "variable":ename, "visible":true, "color":4688896, "y_block":0 }; 
    vars.push(edef);

    var ename = "flags [0]";
    defArr[1] = callbackclass.DefineFlagsVariable(ename, 0);
    var edef = { "variable":ename, "visible":true, "color":2260467, "y_block":1 }; 
    vars.push(edef);
    
    Promise.all(defArr).then((res) => {
      // scope Y-blocks

      var yblocks = [
        { "laxis_label":"proximity", "join_class":0, "laxis_min":0, "laxis_max":index, "laxis_min_auto":true, "laxis_max_auto":true },
        { "laxis_label":"flags", "join_class":1, "laxis_min":0, "laxis_max":index, "laxis_min_auto":true, "laxis_max_auto":true },        
      ];

      // scope definition
      var def = {};
      def["var_info"] = vars;
      def["yblock_info"] = yblocks;
      def["scope_period"] = 0.025;
      def["auto_delete"] = true;
      def["href"] = DOC_PATH+"group__proxi.html#details";

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
 });
}

