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

 
 /* NTControl_rotary class */ 

function NTControl_rotary(symbol, addr)
{
    // call base when not typed dynamically (see change_class_type()) 
    // otherwise, the base object is already constructed
    if(!this._dynamically_typed)
        NTControl(symbol, addr)

    this._gui_id0 = get_unique_id(); // position text box
    this._range = this._electrodes.length * 2;

    // define RAM type
    var st = pcm.DefineSymbol(this._symbol, this._addr, "nt_control_data*").then((res) => {
      var st = pcm_read_ptr_new(this._symbol + "->data").then((res) => {
        var st = pcm.DefineSymbol(this._symbol + "->data", res.data, "nt_control_rotary_data").then((res) => {
    		  debug_print("The Rotary Control has been created.", false);
          this.NTControlFinish();
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

NTControl_rotary.prototype.GetRange = function()
{
    return this._range;
}

NTControl_rotary.prototype.GetPosition = function(callback)
{
    var st = pcm.ReadUIntVariable(this._symbol + "->data.position", 4).then((res) => {
      callback(res.data);
    })
    .catch((err) => {
      this.OnError(err);
    });
}


// The main information in top of control table
NTControl_rotary.prototype.GetGuiTag = function()
{
    var str = "";
    str = "<img src=\"./img/rotate.svg\" class=\"perifery-icon\"/>";
    return str;
}

NTControl_rotary.prototype.GetGuiTagsLabels = function()
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

NTControl_rotary.prototype.GetGuiTags = function()
{
    var str = "";
    str += "<div class=\"mtl-type-sh\"><p>"+this._type+"</p></div>";
    str += "<div class=\"mtl-position\" id=" + this._gui_id0 + "pos><p>0</p></div>";
    str += "<div class=\"mtl-rang\" id=" +  this._gui_id0 + "range><p>0</p></div>";
    str += "<div class=\"mtl-touch\"><div class=\"empty-touch\" id=" + this._gui_id0 + "Touch></div></div>";
    str += "<div class=\"mtl-movement\"><p><img id=" + this._gui_id0 + "Movement class=\"movement-icon\" src=\"./img/movement.svg\"</p></div>";
    str += "<div class=\"mtl-direction\" id=" + this._gui_id0 + "Direction><p>right</p></div>";
    str += "<div class=\"mtl-invalid-touch\" ><p><img id=" + this._gui_id0 + "Invalid class=\"triangle-icon\" src=\"./img/triangle.svg\"</p></div>";
    return str;
}

NTControl_rotary.prototype.UpdateGui = function()
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

NTControl_rotary.prototype.OnLinkClicked = function()
{
  var pname = this._name + "_position";
  return pcm_define_variable_new(pname, this._symbol + "->data.position").then((res) => {
    var ok = res;
    var callbackclass = this;

    // scope variables
    var vars = [ 
        {"variable":pname, "visible":true, "color":3026413, "y_block":0 } 
    ]; 

    // up to 7 electrode signals
    var el_count = callbackclass._electrodes.length;
    if(el_count > 7)
        el_count = 7;
    var defArr = new Array();
    for(var e=0; e<el_count; e++)
    {
        var ename = callbackclass._name + "_electrode" + e + "_signal";
        defArr[e] = callbackclass._electrodes[e].DefineSignalVariable(ename);
        
        var edef = { "variable":ename, "visible":true, "color":4688896*e, "y_block":1 }; 
        vars.push(edef);
    }
    Promise.all(defArr).then((res) => {
      // scope Y-blocks
      var yblocks = [
        { "laxis_label":"position", "join_class":0, "laxis_min":0, "laxis_max":callbackclass.GetRange(), "laxis_min_auto":false, "laxis_max_auto":false },
        { "laxis_label":"signal", "join_class":1, "laxis_min_auto":true, "laxis_max_auto":true },
      ];

      // scope definition
      var def = {};
      def["var_info"] = vars;
      def["yblock_info"] = yblocks;
      def["scope_period"] = 0.025;
      def["auto_delete"] = true;
      def["href"] = DOC_PATH+"group__rotary.html#details";

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

