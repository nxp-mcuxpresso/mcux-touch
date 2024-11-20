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
  
function NTElectrode(module, symbol, addr, moduleType,onInitialized, baseOnInit)
{
    this._id = get_unique_id();
    this._valid = false;
    this._module = module;
    this._symbol = symbol;
    this._addr = addr;
    this._romaddr = null;
    this._moduleType = moduleType;
    this._keydettype = null;
    this._keydetcfg_addr = null;
    this._keydetsymbol = null;
    this._shielding_symbol = null;
    this._shielding_addr = null;
    this._name = "";
    this._ramstruct = "nt_electrode_data";
    this._romstruct = "nt_electrode";
    this._gui_id1 = get_unique_id();
    this._gui_id2 = get_unique_id();
    this._statusCnt = 1;
    this._electrode_colors_table = ["color-ball bg-green", "color-ball bg-blue", "color-ball bg-lblue", "color-ball bg-violet", "color-ball bg-pink", "color-ball bg-yellow", "color-ball bg-orange"];
    this.baseOnInit = baseOnInit;
    this._electrode_index = NTElectrode.InitByAddrIndex.length;
    this.maxtouch = 0;
    this.lasttouchstate = 0;
    this.read = false;

    var el_cnt = 1;
    if(NTElectrode.InitByAddr)
      el_cnt = NTElectrode.InitByAddrIndex.length + Number(1);
    
    this.onInitialized = onInitialized;
    this.onFinishClasCallback = new Array();
    this._electrode_color = this._electrode_colors_table[el_cnt%this._electrode_colors_table.length];

    if(this._moduleType)
        this._symbol = symbol;

    if(NTElectrode.InitByAddr[addr])
      alert("Internal error: NTElectrode already exists");
    NTElectrode.InitByAddr[this._addr] = this;
    var item = {};
    item[this._addr] = this;
    NTElectrode.InitByAddrIndex.push(item)
    
    // generic structure at symbol
    var st = pcm_define_symbol_new(this._symbol, this._addr, this._ramstruct+"*").then((res) => {
      // generic ROM address 
      var st = pcm_read_ptr_new(this._symbol + "->rom").then((res) => {
        this._romaddr = res.data;
        var st = pcm_define_symbol_new(this._symbol + "->rom", this._romaddr, this._romstruct+"*").then((res) => {
          // try to determine keydetector type dynamically
          var st = pcm_read_ptr_new(symbol + "->rom->keydetector_interface").then((res) => {
            var itf = res.data;
            return this.CheckElectrodeType(symbol, itf, 0);
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
}

NTElectrode.prototype.NTElectrodeContinue = function(symbol)
{
    var st = pcm.GetSymbolInfo(symbol + "->status").then((res) => {
      var baseAddr = res.xtra.addr;
      var tmpSymbolSize = res.xtra.size;
      this._statusCnt = 1;
      var st = pcm.GetSymbolInfo(symbol + "->status[0]").then((res) => {
        this._statusCnt = tmpSymbolSize/res.xtra.size;
        for(var i=0; i<this._statusCnt; i++)
        {
          var st = pcm_define_symbol_new(symbol + "->status["+i+"]", baseAddr+(res.xtra.size*i), "nt_electrode_status").then((res) => {
            ;
          })
        }
        return this.NTElectrodeContinue2();
      })
      return st;
    })
    return st;
}

NTElectrode.prototype.NTElectrodeContinue2 = function()
{
  debug_print("   ->status: " +this._statusCnt);
  this._shielding_symbol = this._symbol + "->shielding_electrode";
  var st = pcm_read_ptr_new(this._shielding_symbol).then((res) => {
    this._shielding_addr = res.data;
    var st = pcm_define_symbol_new(this._shielding_symbol, this._shielding_addr, this._ramstruct+"*").then((res) => {
      var st = pcm_read_ptr_new(this._symbol + "->rom->keydetector_params").then((res) => {
      this._keydetcfg_addr = res.data;
      // success!
      this._valid = true;

      // create the electrode name
      this._name = "electrode_" + this._keydettype + "_"+ (this._electrode_index+Number(1));
      debug_print("NTElectrode defined at 0x" + this._addr.toString(16) + ", symbol=" + this._symbol + ", name=" + this._name, false);

      var pushelindex = NTElectrode.All.length
      for(elindex in NTElectrode.All)
      {
        if(NTElectrode.All[elindex]._romaddr>this._romaddr)
        {
          pushelindex = elindex;
          break;
        }
      }
      NTElectrode.All.splice(pushelindex, 0, this);
      NTElectrode.ById[this._id] = this;
      NTElectrode.ByName[this._name] = this;
      NTElectrode.ByAddr[this._addr] = this;
      NTElectrode.BySymbol[this._symbol] = this;
      
      for(i in this.onFinishClasCallback)
        this.onFinishClasCallback[i].callback(this.onFinishClasCallback[i].callback_class, this, this._addr);

      if(typeof this.onInitialized === 'function')
      {
        args = []
        args.push(this.baseOnInit);
        args.push(this)
        args.push(this._addr)
        this.onInitialized.apply(this.baseOnInit, args);
      }
      })
      return st;
    })
    return st;
  })
  return st;
}

NTElectrode.prototype.CheckElectrodeType = function(symbol, itf, mt)
{
  if(mt<KEYDETECTOR_TYPES.length)
  {
    var st = pcm.GetSymbolInfo("nt_keydetector_" + KEYDETECTOR_TYPES[mt] + "_interface").then((res) => {
      debug_print("Found interface: " + KEYDETECTOR_TYPES[mt], false);
      if(itf == res.xtra.addr)
      {
        this._keydettype = KEYDETECTOR_TYPES[mt];
        debug_print("Keydetector at " + this._symbol + " is " + KEYDETECTOR_TYPES[mt], false);

        var keydetAddr = null;
        var st = pcm_read_ptr_new(symbol + "->keydetector_data").then((res) => {
          keydetAddr = res.data;
          this._keydetsymbol = this._symbol + "->keydetector_data";
          var st = pcm_define_symbol_new(this._keydetsymbol, keydetAddr, "nt_keydetector_"+KEYDETECTOR_TYPES[mt]+"_data*").then((res) => {
            // define symbol for control parameters
            this._keydetstatsymbol = this._symbol + "->rom->keydetector_params";
            var st = pcm.GetSymbolInfo(this._keydetstatsymbol).then((res) => {
              var electrode_params_addr = res.xtra.addr;
              var st = pcm_read_ptr_new(electrode_params_addr).then((res) => {
                var st = pcm_define_symbol_new(this._keydetstatsymbol+"."+this._keydettype, res.data, "nt_keydetector_"+this._keydettype).then((res) => {
                  // change our class to the real one wrapping the functionality specific for the electrode type
                  var classname = "NTElectrode_" + this._keydettype;
                  change_class_type(this, classname);
                  return this.NTElectrodeContinue(symbol);
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
      }
      else if(mt<KEYDETECTOR_TYPES.length)
      {
        mt++;
        st = this.CheckElectrodeType(symbol, itf, mt);
      }
      return st;
    })
    .catch((err) => {
      if(mt<KEYDETECTOR_TYPES.length)
      {
        mt++;
        
        //this code is executed, when
        //a) fails reading the symbol of list of types   ...   continue listing
        //b) fails any promise in NTModuleContinueInit();...   print error
        //when initialized  this._type, the type was detected, print the error.
        if(this._keydettype == null)
          this.CheckElectrodeType(symbol, itf, mt);
        else
          this.OnError(err);
      }
    });
    return st;
  }
  else if(this._keydettype == null)
  {
    debug_print("Control type was not detected");
    return this.NTElectrodeContinue(symbol);
  }
  return 0;
}

NTElectrode.prototype.OnError = function(msg)
{
  if(msg.hasOwnProperty("msg"))
    debug_print("Failed init NTElectrode:"+ msg.msg, true);
  else
    debug_print("Failed init NTElectrode:"+ msg, true);
}




NTElectrode.All = new Array();
NTElectrode.ById = new Array();
NTElectrode.ByName = new Array();
NTElectrode.ByAddr = new Array();
NTElectrode.BySymbol = new Array();
NTElectrode.InitByAddr = new Array();
NTElectrode.InitByAddrIndex = new Array();
document.NTElectrode = NTElectrode;


NTElectrode.UpdateGuiAll = function ()
{
    for(e in NTElectrode.All)
        NTElectrode.All[e].UpdateGui();
}

NTElectrode.prototype.RegisterCallback = function (callback_class, callback)
{
    var callback_str =  { "callback_class":callback_class, "callback":callback};
    
    this.onFinishClasCallback.push(callback_str);
}

NTElectrode.prototype.IsValid = function()
{
    return this._valid;
}

NTElectrode.prototype.GetName = function()
{
    return this._name;
}

NTElectrode.prototype.GetSymbol = function()
{
    return this._symbol;
}

NTElectrode.prototype.GetLink = function()
{
    var code1 = "NTElectrode.ById[\"" + this._id + "\"].CreateProperty(3);SetCurrenPage(3)";
    var code2 = "NTElectrode.ById[\"" + this._id + "\"].OnLinkClicked()";
    return "<p><a href='pcmaster:void' onclick='"+code1+"'>" + this._name + "</a>, <a href='pcmaster:void' onclick='" + code2 + "'>scope</a></p>";
}
NTElectrode.prototype.GetKeydetType = function()
{
    return this._keydettype;
}

NTElectrode.prototype.GetGuiVar = function(id)
{
    return "<span id=" + this._gui_id1 + id + ">?</span>";
}


NTElectrode.prototype.GetGuiColor = function(id)
{
    return "<div id=" + this._gui_id1 + id + " class=\""+this._electrode_color+"\"></div>";
}

NTElectrode.prototype.GetGuiTouchSt = function()
{
    return "<div id=" + this._gui_id1 + "TouchSt class=\"empty-touch\"></div>";
}


NTElectrode.prototype.CreateScope = function(def_object)
{
    var item = this._name + " Details";
    var st = pcm.DefineOscilloscope(item, JSON.stringify(def_object)).then((res) => {
      if(res.xtra.retval)
        pcm.SelectItem(item, "scope");
      else
        alert("Could not create electrode graph item.");
    })
    .catch((err) => {
      this.OnError(err);
    });
}

NTElectrode.prototype.GetTouchState = function() 
{
   if(!pcm_read_var(this._symbol + "->status_index", 1))
        return false;
        
   var status_index = pcm.LastVariable_vValue; 

   if(!pcm_read_var(this._symbol + "->status[" + status_index + "].state", 4))
        return false;
        
    return pcm.LastVariable_vValue == 2;
}

NTElectrode.prototype.GetTouchStateNew = function(objid)
{
  return new Promise((resolve, reject) => {
    var st = pcm_read_var_new(this._symbol + "->status_index", 1).then((res) => {
      var st = pcm_read_var_new(this._symbol + "->status[" + res.data + "].state", 4).then((res) => {
        resolve( { objid: objid, data: res.data == 2 } );
        return st;
      })
      return st;
    })
    return st;
  });
}

NTElectrode.prototype.GetTouchStateNew2 = function(callback) 
{
    var callbackclass = this;
    var st = pcm.ReadUIntVariable(this._symbol + "->status_index", 1).then((res) => {
      var st = pcm.ReadUIntVariable(callbackclass._symbol + "->status[" + res.data + "].state", 4).then((res) => {
        if(callback)
          callback(res.data == 2);
        return st;
      })
      return st;
    })
    .catch((err) => {
      this.OnError(err);
    });
}

NTElectrode.prototype.GetSignal = function() 
{
   if(!pcm_read_var(this._symbol + "->signal"))
        return 0;
        
    return pcm.LastVariable_vValue;
}
NTElectrode.prototype.GetVar = function(param) 
{
   if(!pcm_read_var(this._symbol + "->"+param))
        return 0;
        
    return pcm.LastVariable_vValue;
}

NTElectrode.prototype.GetVarNew = function(param)
{
  return new Promise((resolve, reject) => {
    var st = pcm_read_var_new(this._symbol + "->"+param).then((res) => {
      resolve( res.data );
      return st;
    })
    .catch((err) => {
      this.OnError(err);
    });
  });
}

NTElectrode.prototype.GetVarNew2 = function(symbol, objid, callback)
{
      var st = pcm.ReadUIntVariable(this._symbol + "->"+symbol, 4).then((res) => {
        var obj = document.getElementById(objid);
        if(callback)
          callback(obj, res.data);
      })
      .catch((err) => {
        this.OnError(err);
      });
}

NTElectrode.prototype.Getval = function(param)
{
  if(param in NTElectrode.BySymbol[this._symbol])
    return NTElectrode.BySymbol[this._symbol][param];
  return 0;
}

NTElectrode.prototype.GetVarIdNew = function(param, objid)
{
  return new Promise((resolve, reject) => {
    var st = pcm_read_var_new(this._symbol + "->"+param).then((res) => {
      resolve( { name: param, value: res.data, objid: objid } );
    }).catch((err) => {
      reject(err);
    });
  });
}

// signal is common to all electrodes
NTElectrode.prototype.DefineSignalVariable = function(vname)
{
    return pcm_define_variable_new(vname, this._symbol + "->signal");
}

// status_index is common to all electrodes
NTElectrode.prototype.DefineStatusVariable = function(vname)
{
    return pcm_define_variable_new(vname, this._symbol + "->status_index");
}

// baseline is common to all electrodes
NTElectrode.prototype.DefineBaselineVariable = function(vname)
{
    return pcm_define_variable_new(vname, this._symbol + "->baseline");
}

// flags is common to all electrodes
NTElectrode.prototype.DefineFlagsVariable = function(vname)
{
    return pcm_define_variable_new(vname, this._symbol + "->flags");
}

// raw_signal is common to all electrodes
NTElectrode.prototype.DefineRawSignalVariable = function(vname)
{
    return pcm_define_variable_new(vname, this._symbol + "->raw_signal");
}

// keydetector_data is common to all electrodes
NTElectrode.prototype.DefineKeydetectorDataVariable = function(vname)
{
    return pcm_define_variable_new(vname, this._symbol + "->keydetector_data");
}

// filtered signal is common to safa electrodes
NTElectrode.prototype.DefineFilteredSignalVariable = function(vname)
{
    debug_assert(this._keydetsymbol, "need typed keydetector");
    return pcm_define_variable_new(vname, this._keydetsymbol + "->predicted_signal");
}

// filtered signal is common to safa electrodes
NTElectrode.prototype.DefineNoiseVariable = function(vname)
{
    debug_assert(this._keydetsymbol, "need typed keydetector");
    return pcm_define_variable_new(vname, this._keydetsymbol + "->noise");
}

// filtered signal is common to safa electrodes
NTElectrode.prototype.DefineEventCntVariable = function(vname)
{
    debug_assert(this._keydetsymbol, "need typed keydetector");
    return pcm_define_variable_new(vname, this._keydetsymbol + "->entry_event_cnt");
}

// filtered signal is common to safa electrodes
NTElectrode.prototype.DefineDeadBandVariable = function(vname)
{
    debug_assert(this._keydetsymbol, "need typed keydetector");
    return pcm_define_variable_new(vname, this._keydetsymbol + "->deadband_cnt");
}

// DeadbandHigh signal is common to safa electrodes
NTElectrode.prototype.DefineDeadBandHighVariable = function(vname)
{
    debug_assert(this._keydetsymbol, "need typed keydetector");
    return pcm_define_variable_new(vname, this._keydetsymbol + "->deadband_h");
}

// DeadbandLow signal is common to safa electrodes
NTElectrode.prototype.DefineDeadBandLowVariable = function(vname)
{
    debug_assert(this._keydetsymbol, "need typed keydetector");
    return pcm_define_variable_new(vname, this._keydetsymbol + "->deadband_l");
}

// Smooth Baseline for MBW keydetector
NTElectrode.prototype.DefineSmoothBaselineVariable = function(vname)
{
    debug_assert(this._keydetsymbol, "need typed keydetector");
    return pcm_define_variable_new(vname, this._keydetsymbol + "->smooth_baseline");
}

// update common electrode GUI showing just a touch-release state and signal value
NTElectrode.prototype.UpdateGui = function()
{
  var vars = ["signal", "baseline", "raw_signal", "flags"];
  var proms = [];
  this.read = true;
  for( varname in vars)
  {
    const prom = this.GetVarIdNew(vars[varname], this._gui_id1+vars[varname]);
    proms.push(prom);
  }
  const prom = this.GetTouchStateNew(this._gui_id1+"TouchSt");
  proms.push(prom);

  Promise.all(proms).then((vals) => {
    this.read = false;
    var templates = [function(val){   var obj = document.getElementById(val.objid); if(obj) obj.innerHTML = "<p>"+val.value+"</p>"; }, 
                     function(val){   var obj = document.getElementById(val.objid); if(obj) obj.innerHTML = "<p>"+val.value+"</p>"; }, 
                     function(val){   var obj = document.getElementById(val.objid); if(obj) obj.innerHTML = "<p>"+val.value+"</p>"; }, 
                     function(val){   var obj = document.getElementById(val.objid); if(obj) obj.innerHTML = val.value; },
                     function(val){   var img = document.getElementById(val.objid); if(img) img.className = val.data ? "full-touch" : "empty-touch"; },
                     ];
    for(res in vals)
    {
      templates[res](vals[res]);
    }
    if(vals[4].data != this.lasttouchstate) 
    {
        this.lasttouchstate = vals[4].data;
        if(this.lasttouchstate)
          this.maxtouch = vals[0].value;
    }
    var val = this.Getval("maxtouch");
    var obj = document.getElementById(this._gui_id1+"maxtouch");
    if(obj) 
      obj.innerHTML = val;
  })
  .catch((err) => {
    this.read = false;
    this.OnError(err);
  });
}

// define basic scope. this function is typically overriden by NTElectrode_xxx 
NTElectrode.prototype.OnLinkClicked = function()
{
    // give scope with basic electrode variables
    var vname_status  = this._name + "_status_index"; 
    var vname_signal  = this._name + "_signal"; 
    var vname_baseline = this._name + "_position";
    this.DefineSignalVariable(vname_status).then((res) => {
      if(res)
        var st = this.DefineFilteredSignalVariable(vname_signal).then((res) => {
          if(res)
            var st = this.DefineFilteredSignalVariable(vname_baseline).then((res) => {
              if(res)
              {
                  // scope variables
                  var vars = [ 
                      {"variable":vname_signal, "visible":true, "color":3026413, "y_block":0 }, 
                      {"variable":vname_baseline, "visible":true, "color":3026413, "y_block":0 },
                      {"variable":vname_status, "visible":true, "color":3026413, "y_block":1 },
                  ]; 
                  // scope Y-blocks
                  var yblocks = [
                      { "laxis_label":"raw signal", "join_class":0, "laxis_min_auto":true, "laxis_max_auto":true },
                      { "laxis_label":"touch status", "join_class":1, "laxis_min_auto":true, "laxis_max_auto":true },
                  ];
                  // scope definition
                  var def = {};
                  def["var_info"] = vars;
                  def["yblock_info"] = yblocks;
                  def["scope_period"] = 0.025;
                  def["href"] = DOC_PATH+"group__electrodes.html#details";

                  this.CreateScope(def);
              }
            });
          return st;
        });
      return st;
    })
    .catch((err) => {
      this.OnError(err);
    });
}

function ElectrodeButtonHandler(name)
{
  var pObj =NTElectrode.ByName[name];
  pObj.UpdateGuiProperty();
  
}

NTElectrode.prototype.CreateProperty = function(tab)
{
    var index = 0;
    var x = document.createElement("SELECT");

    //get selected electrode
    for (e in NTElectrode.All)
    {
      var opt;
      opt = document.createElement("option");
      opt.setAttribute("value", "1");
      opt.innerHTML = NTElectrode.All[e]._name;
      opt.className = "selection-name";
      if(NTElectrode.All[e]._name == this._name)
      {
        index = e;
      }
      x.appendChild(opt);
    }
    x.id = "NTElectrode_select";
    x.selectedIndex = Number(index);
    x.className = "choose unselected";
    x.onchange = changeHandler;
    function changeHandler(){
        var tmp = 0;
        sel = document.getElementById("NTElectrode_select");
        if(sel)
          tmp = sel.selectedIndex;
        NTElectrode.All[tmp].CreateProperty(tab);
    }
    
    //create content of the electrode property
    var str;
    str = "<div class=\"top-modal-section\">";
    str += "<p class=\"describing\">Selected electrode: </p>";
    var code1 = "NTElectrode.ById[\"" + this._id + "\"].OnLinkClicked()";
    str += "<div class=\"select-wrap\" id=selectDiv>";
    str += "</div>";
    str += "<p class=\"modal-headline\"><a href='pcmaster:void' onclick='" + code1 + "'>Scope</a> of " + this._name+ " <span class=\"grey-text\">| measured data</span></p>"
    str += "<p class=\"modal-state-ok\"><img src=\"./img/detected.svg\" class=\"icon-detected\">detected</p>";
    str += "</div>";
    str += this.GetElectrodeGui("");
    str += this.GetKeydetGui("");
    if(this._shielding_addr)
    {
        str += "</br>Shielding electrode:";
        str += this.GetElectrodeGui("shielding_electrode->");
    }
    str += "</p></center>";

    // move property of the electrode to page
    var div = document.getElementById("tab"+String(tab));
    if(div)
        div.innerHTML=str;
    var div2 = document.getElementById("selectDiv");

    //update select object
    div2.appendChild(x);
}

NTElectrode.prototype.GetElectrodeGui = function(shielding)
{
    var str = "";
    var idbase = this._id+shielding+"elData";
    str += "<div class=\"modules-table\">";
    str += "<div class=\"modules-table-headline\">";
    str += "<div class=\"mhl-raw-signal\"><p>Raw_Signal</p></div>";
    str += "<div class=\"mhl-signal\"><p>Signal</p></div>";
    str += "<div class=\"mhl-baseline\"><p>Baseline</p></div>";
    str += "<div class=\"mhl-flags\"><p>Flags</p></div>";
    for(i=0; i<this._statusCnt; i++)
    {
	    str += "<div class=\"mhl-statex\"><p>State "+i+"</p></div>";
	    str += "<div class=\"mhl-tstamp\"><p>T_stamp "+i+"</p></div>";
    }
    str += "</div>";
    str += "<div class=\"modules-table-line\">";
    str += "<div class=\"mtl-raw-signal\" id=\""+idbase+"raw_signal\"><p>0</p></div>";
    str += "<div class=\"mtl-signal\" id=\""+idbase+"signal\"><p>0</p></div>";
    str += "<div class=\"mtl-baseline\" id=\""+idbase+"baseline\"><p>0</p></div>";
    str += "<div class=\"mtl-flags\" id=\""+idbase+"flags\"><p>0</p></div>";
    for(i=0; i<this._statusCnt; i++)
    {
	    str += "<div class=\"mtl-statex\" id=\""+idbase+"status["+i+"].state\"><p>0</p></div>";
	    str += "<div class=\"mtl-tstamp\" id=\""+idbase+"status["+i+"].time_stamp\"><p>0</p></div>";
    }
    str += "<div class=\"cistic\"></div>";
    str += "</div>";
    str += "</div>";
    
    return str;
}

NTElectrode.prototype.UpdateGuiElectrode = function()
{
    this.UpdateGuiElectrodeRaw("");
}

NTElectrode.prototype.UpdateGuiShieldElectrode = function()
{
    if(this._shielding_addr)
      this.UpdateGuiElectrodeRaw("shielding_electrode->");
}


NTElectrode.prototype.UpdateGuiElectrodeRaw = function(shieldding)
{
  var idbase = this._id+shieldding+"elData";
  var subids = ["raw_signal", "signal", "baseline", "flags"]; 
  var proms = [];
  this.read = true;
  for( subidindex in subids)
  {
    const prom = this.GetVarIdNew(shieldding+subids[subidindex], idbase+subids[subidindex]);
    proms.push(prom);
  }
  for(i=0; i<this._statusCnt; i++)
  {
    const prom = this.GetVarIdNew(shieldding+"status["+i+"].state", idbase+"status["+i+"].state");
    proms.push(prom);
    const prom2 = this.GetVarIdNew(shieldding+"status["+i+"].time_stamp", idbase+"status["+i+"].time_stamp");
    proms.push(prom2);
  }
  const prom = this.GetVarNew(shieldding+"status_index");
  proms.push(prom);
  this.read = true;
  
  Promise.all(proms).then((vals) => {
    this.read = false;
    var templates = [function(val){   var obj = document.getElementById(val.objid); if(obj) obj.innerHTML = "<p>"+val.value+"</p>"; }, 
                     function(val){   var obj = document.getElementById(val.objid); if(obj) obj.innerHTML = "<p>"+val.value+"</p>"; }, 
                     function(val){   var obj = document.getElementById(val.objid); if(obj) obj.innerHTML = "<p>"+val.value+"</p>"; }, 
                     function(val){   var obj = document.getElementById(val.objid); if(obj) obj.innerHTML = val.value; },
                     function(val){   var obj = document.getElementById(val.objid); if(obj) {obj.innerHTML = "<p>"+val.value+"</p>";obj.style.backgroundColor = "azure";} },
                     function(val){   var obj = document.getElementById(val.objid); if(obj) {
                        var t = val.value;
                        tms = t%1000;
                        ts = Math.floor(t/1000);
                        tm = Math.floor(ts/60);
                        th = Math.floor(tm/60);
                        tstr = String(th) + ":" + String(tm%60) + ":" + String(ts%60) + "." + String(tms);
                        obj.innerHTML = "<p>"+tstr+"</p>";
                        obj.style.backgroundColor = "white";
                      } },
                     function(val){   var obj = document.getElementById(val.objid); if(obj) {obj.innerHTML = "<p>"+val.value+"</p>";obj.style.backgroundColor = "azure";} },
                     function(val){   var obj = document.getElementById(val.objid); if(obj) {
                        var t = val.value;
                        tms = t%1000;
                        ts = Math.floor(t/1000);
                        tm = Math.floor(ts/60);
                        th = Math.floor(tm/60);
                        tstr = String(th) + ":" + String(tm%60) + ":" + String(ts%60) + "." + String(tms);
                        obj.innerHTML = "<p>"+tstr+"</p>";
                        obj.style.backgroundColor = "white";
                      } },
                     function(val){   var obj = document.getElementById(val.objid); if(obj) {obj.innerHTML = "<p>"+val.value+"</p>";obj.style.backgroundColor = "azure";} },
                     function(val){   var obj = document.getElementById(val.objid); if(obj) {
                        var t = val.value;
                        tms = t%1000;
                        ts = Math.floor(t/1000);
                        tm = Math.floor(ts/60);
                        th = Math.floor(tm/60);
                        tstr = String(th) + ":" + String(tm%60) + ":" + String(ts%60) + "." + String(tms);
                        obj.innerHTML = "<p>"+tstr+"</p>";
                        obj.style.backgroundColor = "white";
                      } },
                     function(val){   var obj = document.getElementById(val.objid); if(obj) {obj.innerHTML = "<p>"+val.value+"</p>";obj.style.backgroundColor = "azure";} },
                     function(val){   var obj = document.getElementById(val.objid); if(obj) {
                        var t = val.value;
                        tms = t%1000;
                        ts = Math.floor(t/1000);
                        tm = Math.floor(ts/60);
                        th = Math.floor(tm/60);
                        tstr = String(th) + ":" + String(tm%60) + ":" + String(ts%60) + "." + String(tms);
                        obj.innerHTML = "<p>"+tstr+"</p>";
                        obj.style.backgroundColor = "white";
                      } },
                     function(val){ 
                           var stIndex =val;
                           var obj = document.getElementById(idbase+"status["+stIndex+"].state");
                           if(obj)
                              obj.style.backgroundColor = "DarkGoldenRod"
                           obj = document.getElementById(idbase+"status["+stIndex+"].time_stamp");
                           if(obj)
                             obj.style.backgroundColor = "yellow";
                     },
]
    for(res in vals)
    {
      templates[res](vals[res]);
    }
  })
  .catch((err) => {
    this.read = false;
    this.OnError(err);
  });
}

function GetTimeFromMs(rawTime)
{
	var secs;
	var min;
	var houts;

	if(rawTime < 1000)
	{
		return rawTime.toString(10) + "ms";  
	}
	else if(rawTime < (1000 * 60))
	{
		secs = rawTime / 1000;
		return secs.toString(10)+"s";
	}
	else if(rawTime < (1000 * 60 * 60))
	{
		min = rawTime / (1000 * 60);
		min = Math.floor(min);
		rawTime -= min * (1000 * 60);
		secs = rawTime / 1000;
		return min.toString(10)+"m:"+secs.toString(10)+"s";
	}else
	{
		hours = rawTime / (1000 * 60 *60);
		hours = Math.floor(hours);
		rawTime -= hours * (1000 * 60 * 60);
		min = rawTime / (1000 * 60);
		min = Math.floor(min);
		rawTime -= min * (1000 * 60);
		secs = rawTime / 1000;
		return hours.toString(10)+"h:"+min.toString(10)+"m:"+secs.toString(10)+"s";
	}
}

function GetElectrodeState(state)
{
  switch(state)
  {
    case 0:
        return "Init";
    case 1:
        return "Release";
    case 2:
        return "Touch";
    default:
        return "Unknown";
  }
}
