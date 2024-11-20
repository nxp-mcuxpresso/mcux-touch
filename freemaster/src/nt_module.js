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
  
function NTModule(symbol, addr, onInitialized, OnElInit)
{
    this._id = get_unique_id();
    this._valid = false;
    this._symbol = symbol;
    this._addr = addr;
    this._romaddr = null;
    this._type = "";
    this._name = "";
    this._electrodes = new Array();
    this._struct_type = "nt_module_data";
    this.onInitialized = onInitialized;
    this.onElInit = OnElInit;
    this._electrodes_cnt = 0;
    this._hw_config_addr = 0

    if(NTModule.ByAddr[addr])
      alert("NTModule already exists");
      var st = pcm_define_symbol_new(symbol, addr, this._struct_type + "*").then((res) => {
      debug_print("Reads pointer: " + symbol + "->rom", false);
      var st = pcm_read_ptr_new(symbol + "->rom").then((res) => {
        // remember our ROM pointer
        this._romaddr = res.data
        var st = pcm_define_symbol_new(symbol + "->rom", this._romaddr, "nt_module*").then((res) => {
          //load electrodes
          this.NTModuleContinueInit2(symbol);
          // try to determine control type dynamically
          var st = pcm_read_ptr_new(symbol + "->rom->interface").then((res) => {
            var itf = res.data;
            // by comparing interface pointers with known ones
            return this.CheckModuleType(symbol, itf, 0);
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
NTModule.prototype.NTModuleContinueInit = function(symbol)
{
  var st = pcm_read_ptr_new(symbol + "->rom->config").then((res) => {
    var config = res.data;
    this._hw_config_addr = res.data;
    if(this.GetType() == "tsi")
    {
      var st = pcm_define_symbol_new(symbol + "->rom->config", config, "tsi_config_t*").then((res) => {
        var st = pcm_read_ptr_new(symbol + "->rom->recalib_config").then((res) => {
          var recalib_config = res.data;
          var st = pcm_define_symbol_new(symbol + "->rom->recalib_config", recalib_config, "nt_tsi_recalib_config_t*").then((res) => {
            return this.NTModuleContinueInit3(symbol);
          })
          return st;
        })
        .catch((err) => {
          //this exception is called, 
          //a)when reading symbol + "->rom->recalib_config" fails, (this._name  not defined)
          //b) when somepromisses fail (this._name is defined)
          if(this._name == "")
          {
            debug_print("variable: "+symbol + "->rom->recalib_config, not available("+err+")", 0);
            return this.NTModuleContinueInit3(symbol);
          }
          else
          {
            this.OnError(err);
          }
        });
        return st;
      });
    }
    if(this.GetType() == "cs")
    {
      var st = pcm_define_symbol_new(symbol + "->rom->config", config, "cs_config_t*").then((res) => {
        return this.NTModuleContinueInit3(symbol);
      });
    }
    return st;
  })
  .catch((err) => {
    this.OnError(err);
  });

  return st;
}
NTModule.prototype.NTModuleContinueInit2 = function(symbol)
{
    // read x->electrodes array
    var st = pcm_read_var_new(symbol + "->electrodes_cnt", 1).then((res) => {
      var el_cnt = res.data;
      this._electrodes_cnt = el_cnt;
      debug_print("Read " + this._symbol + "->electrodes_cnt" + "..." + el_cnt);
      var elarrname = symbol + "->electrodes";
         var st = read_ptr_array_cnt_new(elarrname, el_cnt).then((res) => {
          var elarr = res.data;
          var eix = 0;
          for(eix=0; eix<elarr.length; eix++)
          {
            // pointer to the nt_electrode object
            var eptr = elarr[eix];

            // electrode already loaded?
            var el = 0;
            var elInit = 0;
            {
              el = NTElectrode.ByAddr[eptr];
              elInit = NTElectrode.InitByAddr[eptr];
              debug_print("NTModule uses electrode #" + eix + " at 0x" + eptr.toString(16) + ", " + (el ? el.GetName() : "creating..."), false);            
            }

            // no, create nt_electrode type
            if(!elInit)
              el = new NTElectrode(this, elarrname + "["+eix+"]", eptr, this._type, this.OnElectrodeFinish, this);
            else if(el)
              this.OnElectrodeFinish(el, eptr);
            else 
              elInit.RegisterCallback(this, this.OnElectrodeFinish);
          }
          return st;
        })
        return st;
    })
    return st;
}

NTModule.prototype.NTModuleContinueInit3 = function(symbol)
{
          // success!
          this._valid = true;

          NTModule.All.push(this);
          NTModule.ById[this._id] = this;
          NTModule.ByName[this._name] = this;
          NTModule.ByAddr[this._addr] = this;
          NTModule.BySymbol[this._symbol] = this;

          // create the module name
          this._name = "module_" + this._type + "_"+ NTModule.All.length;
          debug_print("NTModule defined at " + this._symbol + ", name=" + this._name + ", electrodes=" + this._electrodes.length, false);

          if(typeof this.onInitialized === 'function')
          {
            args = []
            args.push(this._id)
            this.onInitialized.apply(this, args);
          }
}

NTModule.prototype.CheckModuleType = function(symbol, itf, mt)
{
  if(mt<MODULE_TYPES.length)
  {
    var st = pcm.GetSymbolInfo("nt_module_" + MODULE_TYPES[mt] + "_interface").then((res) => {
      if(itf == res.xtra.addr)
      {
          debug_print("Module at " + this._symbol + " is " + MODULE_TYPES[mt]);
          this._type = MODULE_TYPES[mt];
          return this.NTModuleContinueInit(symbol);
      }
      else if(mt<MODULE_TYPES.length)
      {
        mt++;
        return this.CheckModuleType(symbol, itf, mt);
      }
      return st;
    })
    .catch((err) => {
      if(mt<MODULE_TYPES.length)
      {
        //this code is executed, when
        //a) fails reading the symbol of list of types   ...   continue listing
        //b) fails any promise in NTModuleContinueInit();...   print error
        //when initialized  this._type, the type was detected, print the error.
        mt++;
        if(this._type == "")
          this.CheckModuleType(symbol, itf, mt);
        else
          this.OnError(err);
      }
    });
    return st;
  }
  else if(this._type == "")
  {
    debug_print("Module type was not detected");
    return this.NTModuleContinueInit(symbol);
  }
  return 0;
}

NTModule.prototype.OnElectrodeFinish = function(callback_class, el, eptr)
{
  if(el.IsValid())
  {
    var pushelindex = callback_class._electrodes.length
    for(elindex in callback_class._electrodes)
    {
      if(callback_class._electrodes[elindex]._romaddr>el._romaddr)
      {
        pushelindex = elindex;
        break;
      }
    }
    callback_class._electrodes.splice(pushelindex, 0, el);

  }
  else
    debug_print("NTModule uses invalid electrode? 0x" + eptr.toString(16), true);

  if(typeof callback_class.onElInit === 'function')
  {
    args = []
    args.push(el)
    callback_class.onElInit.apply(this, args);
  }
  if(typeof callback_class.onInitialized === 'function')
  {
    args = []
    args.push(callback_class._id)
    callback_class.onInitialized.apply(this, args);
  }
}

NTModule.prototype.OnElectrodeLinkClicked = function()
{
    // give scope with basic electrode variables
    var i = 0;
    var defArr = new Array();
    var vars =  new Array();
    for(i = 0; i<this._electrodes.length; i++)
    {
      let el = this._electrodes[i];
      if((el._keydettype == "safa") || (el._keydettype == "usafa") || (el._keydettype == "mbw"))
      {
        var vname_signal  = el._name + "_signal"; 
        // scope variables
        vars[i] = {"variable":vname_signal, "visible":true, "y_block":0 };
        defArr[i] = el.DefineFilteredSignalVariable(vname_signal);
      }
      else if (el._keydettype == "afid")
      {
        var vname_rawsignal  = el._name + "_raw_signal"; 
        // scope variables
        vars[i] = {"variable":vname_rawsignal, "visible":true, "y_block":0};
        defArr[i] = el.DefineRawSignalVariable(vname_rawsignal);
      }
    }
    Promise.all(defArr).then((res) => {
                if(res)
                {
                  // scope Y-blocks
                  var yblocks = [
                      { "laxis_label":"raw signal", "join_class":0, "laxis_min_auto":true, "laxis_max_auto":true },
                  ];
                  // scope definition
                  var def = {};
                  def["var_info"] = vars;
                  def["yblock_info"] = yblocks;
                  def["scope_period"] = 0.025;
                  def["href"] = DOC_PATH+"group__electrodes.html#details";

                  this._electrodes[0].CreateScope(def);
                }
    })
    .catch((err) => {
        this.OnError(err);
    });
}


NTModule.prototype.OnError = function(msg)
{
  if(msg.hasOwnProperty("msg"))
    debug_print("Failed init NTModule:"+ msg.msg, true);
  else
    debug_print("Failed init NTModule:"+ msg, true);
}

NTModule.All = new Array();
NTModule.ById = new Array();
NTModule.ByName = new Array();
NTModule.ByAddr = new Array();
NTModule.BySymbol = new Array();
document.NTModule = NTModule;

NTModule.prototype.IsValid = function()
{
    return this._valid;
}

NTModule.prototype.GetName = function()
{
    return this._name;
}

NTModule.prototype.GetSymbol = function()
{
    return this._symbol;
}

NTModule.prototype.GetType = function()
{
	return this._type;
}

NTModule.prototype.GetElLink = function()
{
    var code = "NTModule.ById[\"" + this._id + "\"].OnElectrodeLinkClicked()";
    return "<a href='pcmaster:void' onclick='" + code + "'>scope</a>";
}


NTModule.prototype.GetGui = function()
{
    var pEl = this._electrodes;

    var str="<div class=\"modules\">";
    str += "<div class=\"top-modal-section\">";
    str += "<p class=\"modal-headline\"> "+ this._type +": "+this._name+"</p>";
    str += "<p class=\"modal-state-ok\"><img src=\"./img/detected.svg\" class=\"icon-detected\">detected </p><p class=\"modal-el-scope\">"+this.GetElLink()+"</p>";
    str += "</div>";

    str += "<div class=\"modules-table\">";
    str += "<div class=\"modules-table-headline\">";
    str += "<div class=\"mhl-name\"><p>Electrode name</p></div>";
    str += "<div class=\"mhl-keydetector\"><p>Keydetector</p></div>";
    str += "<div class=\"mhl-raw-cnt\"><p>Raw Cnt</p></div>";
    str += "<div class=\"mhl-baseline-cnt\"><p>Baseline Cnt</p></div>";
    str += "<div class=\"mhl-signal\"><p>Signal</p></div>";
    str += "<div class=\"mhl-touch\"><p>Touch</p></div>";
    str += "<div class=\"mhl-flags\"><p>Flags</p></div>";
    str += "<div class=\"mhl-maxtouch\"><p>Max Touch</p></div>";
    str += "</div>";
    for(el in pEl)
    {
        str += "<div class=\"modules-table-line\">";
        str += "<div class=\"mtl-name\">"+pEl[el].GetLink()+"</div>";
        str += "<div class=\"mtl-keydetector\"><p>"+pEl[el].GetKeydetType()+"</p></div>";
        str += "<div class=\"mtl-raw-cnt\"><p>"+pEl[el].GetGuiVar("raw_signal")+"</p></div>";
        str += "<div class=\"mtl-baseline-cnt\"><p>"+pEl[el].GetGuiVar("baseline")+"</p></div>";
        str += "<div class=\"mtl-signal\"><p>"+pEl[el].GetGuiVar("signal")+"</p></div>";
        str += "<div class=\"mtl-touch\">"+pEl[el].GetGuiTouchSt()+"</div>";
        str += "<div class=\"mtl-flags\"><p>"+pEl[el].GetGuiVar("flags")+"</p></div>";
        str += "<div class=\"mtl-maxtouch\"><p>"+pEl[el].GetGuiVar("maxtouch")+"</p></div>";
        str += "</div>";
    }

    str += "</div>";
    str += "</div>";

    return str;
}

