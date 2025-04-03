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

 
 /* NTControl_matrix class */

function NTControl_matrix(symbol, addr)
{
    // call base when not typed dynamically (see change_class_type()) 
    // otherwise, the base object is already constructed
    if(!this._dynamically_typed)
        NTControl(symbol, addr)

    this._gui_id0 = get_unique_id(); // span
    // define RAM type
    var st = pcm.DefineSymbol(this._symbol, this._addr, "nt_control_data*").then((res) => {
      var st = pcm_read_ptr_new(this._symbol + "->data").then((res) => {
        var st = pcm.DefineSymbol(this._symbol + "->data", res.data, "nt_control_matrix_data").then((res) => {
          var st = pcm_read_ptr_new(this._symbol + "->rom->control_params").then((res) => {
            var st = pcm.DefineSymbol(this._symbol + "->rom->control_params", res.data, "nt_control_matrix*").then((res) => {
              debug_print("The matrix Control has been created.", false);
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
    .catch((err) => {
      this.OnError(err);
    });
  return st;
}
/**********************************************************************************************************************/
NTControl_matrix.prototype.GetSizeHor= function(callback)
{                                                 /**/
    var st = pcm.ReadUIntVariable(this._symbol + "->rom->control_params->touchpad_size_horizontal", 1).then((res) => {
      callback(res.data);
    })
    .catch((err) => {
      this.OnError(err);
    });
}
NTControl_matrix.prototype.GetSizeVer= function(callback)
{                                                 /**/
    var st = pcm.ReadUIntVariable(this._symbol + "->rom->control_params->touchpad_size_vertical", 1).then((res) => {
      callback(res.data);
    })
    .catch((err) => {
      this.OnError(err);
    });
}

NTControl_matrix.prototype.GetPosX= function(callback)
{
    var st = pcm.ReadUIntVariable(this._symbol + "->data.actual_position[0].x", 1).then((res) => {
      callback(res.data);
    })
    .catch((err) => {
      this.OnError(err);
    });
}

NTControl_matrix.prototype.GetPosY= function(callback)
{
    var st = pcm.ReadUIntVariable(this._symbol + "->data.actual_position[0].y", 1).then((res) => {
      callback(res.data);
    })
    .catch((err) => {
      this.OnError(err);
    });
}

NTControl_matrix.prototype.GetPosX_2= function(callback)
{
    var st = pcm.ReadUIntVariable(this._symbol + "->data.actual_position[1].x", 1).then((res) => {
      callback(res.data);
    })
    .catch((err) => {
      this.OnError(err);
    });
}

NTControl_matrix.prototype.GetPosY_2= function(callback)
{
    var st = pcm.ReadUIntVariable(this._symbol + "->data.actual_position[1].y", 1).then((res) => {
      callback(res.data);
    })
    .catch((err) => {
      this.OnError(err);
    });
}

NTControl_matrix.prototype.GetElState= function(callback)
{
    var st = pcm.ReadUIntVariable(this._symbol + "->data.last_electode_states", 4).then((res) => {
      callback(res.data);
    })
    .catch((err) => {
      this.OnError(err);
    });
}


NTControl_matrix.prototype.GetRecognized_gesture = function(callback)
{
    var st = pcm.ReadUIntVariable(this._symbol + "->data.recognized_gesture", 1).then((res) => {
      callback(res.data);
    })
    .catch((err) => {
      this.OnError(err);
    });
}

NTControl_matrix.prototype.GetGuiTag = function()
{
    var str = "";
    str = "<img src=\"./img/matrix.svg\" class=\"perifery-icon\"/>";
    return str;
}

NTControl.prototype.GetGuiTags = function()
{
  var str = "";
  str += "<div class=\"mtl-range\"><p>"+this._type+"</p></div>";
  str += "<div class=\"mtl-range\"><div class=\"empty-touch\" id=" + this._gui_id0 + "Touch></div></div>";
  str += "<div class=\"mtl-range\" id=" +  this._gui_id0 + "PosX><p>?</p></div>";
  str += "<div class=\"mtl-range\" id=" +  this._gui_id0 + "PosY><p>?</p></div>";
  str += "<div class=\"mtl-range\" id=" +  this._gui_id0 + "PosX_2><p>?</p></div>";
  str += "<div class=\"mtl-range\" id=" +  this._gui_id0 + "PosY_2><p>?</p></div>";
  str += "<div class=\"mtl-range\" id=" +  this._gui_id0 + "Gestcure><p>?</p></div>";
  return str;  
}

NTControl_matrix.prototype.GetGuiTagsLabels = function()
{
  var str = "";
  str += "<div class=\"mhl-range\"><p>Type</p></div>";
  str += "<div class=\"mhl-range\"><p>Touch</p></div>";
  str += "<div class=\"mhl-range\"><p>Position X1</p></div>";
  str += "<div class=\"mhl-range\"><p>Position Y1</p></div>";
  str += "<div class=\"mhl-range\"><p>Position X2</p></div>";
  str += "<div class=\"mhl-range\"><p>Position Y2</p></div>";
  str += "<div class=\"mhl-range\"><p>Gesture</p></div>";
  return str;
}

NTControl_matrix.prototype.UpdateGui = function()
{
    var callbackclass = this;
    this.GetElState(function(Elstate) {
    	kobj = document.getElementById(callbackclass._gui_id0+"Touch");    
    	if(kobj)
    	{
     		kobj.className = Elstate ? "full-touch" : "empty-touch";  
    	}
    	callbackclass.GetPosX(function(posX) {
    	  kobj = document.getElementById(callbackclass._gui_id0+"PosX");    
    		if(kobj)
    		{
        		if (Elstate)
        		{
         			kobj.innerHTML = "<p>"+posX.toString(10)+"</p>";
       			}   else
         			kobj.innerHTML = '<p>?</p>';
    		}
    	});
    	callbackclass.GetPosY(function(posY) {     
    	  kobj = document.getElementById(callbackclass._gui_id0+"PosY");    
    		if(kobj)
    		{
        		if (Elstate)
        		{
         			kobj.innerHTML = "<p>"+posY.toString(10)+"</p>";
       			}   else
         			kobj.innerHTML = '<p>?</p>';
    		}
    	});
    	callbackclass.GetPosX_2(function(posX_2) {     
    	  kobj = document.getElementById(callbackclass._gui_id0+"PosX_2");    
    		if(kobj)
    		{
        		if (Elstate)
        		{
         			kobj.innerHTML = "<p>"+posX_2.toString(10)+"</p>";
       			}   else
             			kobj.innerHTML = '<p>?</p>';
    		}
    	});
    	callbackclass.GetPosY_2(function(posY_2) {     
    	  kobj = document.getElementById(callbackclass._gui_id0+"PosY_2");    
    		if(kobj)
    		{
        		if (Elstate)
        		{
         			kobj.innerHTML = "<p>"+posY_2.toString(10)+"</p>";
       			}   else
         			kobj.innerHTML = '<p>?</p>';
    		}
    	});
    	callbackclass.GetRecognized_gesture(function(gest) {     
    	  kobj = document.getElementById(callbackclass._gui_id0+"Gestcure");    
    		if(kobj)
    		{
        		if (Elstate)
        		{
         			kobj.innerHTML = "<p>"+gest.toString(10)+"</p>";
       			}   else
         			kobj.innerHTML = '<p>?</p>';
    		}
    	});
    });
}

NTControl_matrix.prototype.DefinePosX1Variable = function(vname)
{
    return pcm_define_variable_new(vname, this._symbol + "->data.actual_position[0].x");
}

NTControl_matrix.prototype.DefinePosY1Variable = function(vname)
{
    return pcm_define_variable_new(vname, this._symbol + "->data.actual_position[0].y");
}

NTControl_matrix.prototype.OnLinkClicked = function()
{
  var callbackclass = this;
  var PosX1 = this._name + "->data.actual_position[0].x";
  var PosY1 = this._name + "->data.actual_position[0].y";
    // scope variables
    var vars = [ 
        {"variable":PosY1, "visible":true},,,,,,,
        {"variable":PosX1, "visible":false},
    ]; 

    var defArr = new Array();
    defArr[0] = this.DefinePosY1Variable(PosY1);
    defArr[1] = this.DefinePosX1Variable(PosX1);
    
    Promise.all(defArr).then((res) => {
      // scope Y-blocks
      var yblocks = [
       { "laxis_label":"Actual finger position Y", "join_class":0, "laxis_min":0, "laxis_max":11, "laxis_min_auto":false, "laxis_max_auto":false },
      ];

      // scope definition
      var def = {};
      def["graph_type"] = 1;
      def["var_info"] = vars;
      def["yblock_info"] = yblocks;

      def["scope_period"] = 0.025;
      def["graph_buffer"] = 50;    
      def["auto_delete"] = true;
      def["href"] = DOC_PATH+"group__matrix.html#details";

      var item = callbackclass._name + " Details";
      ok = res && pcm.DefineOscilloscope(item, JSON.stringify(def));//ok && 
    
      if(ok)
        pcm.SelectItem(item, "scope");
      else
        alert("Could not create graph with selected signals"); 
    }) 
    .catch((err) => {
      this.OnError(err);
    });
}


