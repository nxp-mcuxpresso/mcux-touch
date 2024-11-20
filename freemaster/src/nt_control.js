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
 
 /* NTControl class  */
  
function NTControl(symbol, addr, control_index, onInitialized, OnElInit)
{
    this._id = get_unique_id();
    this._valid = false;
    this._symbol = symbol;
    this._addr = addr;
    this._romaddr = null;
    this._type = "";
    this._name = "";
    this._electrodes = new Array();
    this.onInitialized = onInitialized;
    this.OnElInit = OnElInit;
    this._electrodes_cnt = 0;
    this._control_index = control_index;
    this.readcontrol = false;

    // sanity check
    if(NTControl.ByAddr[addr])
        alert("NTControl already exists");

    var st = pcm_define_symbol_new(symbol, addr, "nt_control_data*").then((res) => {
      debug_print("Reads pointer: " + symbol + "->rom", false);
      var st = pcm_read_ptr_new(symbol + "->rom").then((res) => {
        // remember our ROM pointer
        this._romaddr = res.data
        var st = pcm_define_symbol_new(symbol + "->rom", this._romaddr, "nt_control*").then((res) => {
          // read x->electrodes array
          var elarrname = symbol + "->electrodes";
          var st = pcm_read_var_new(symbol + "->electrodes_size", 1).then((res) => {
            var el_cnt = res.data;
            this._electrodes_cnt = el_cnt;
              var st = read_ptr_array_cnt_new(elarrname, el_cnt, SIZEOF_PTR).then((res) => {
                var elarr = res.data;
                var eix;
                for(eix=0; eix<elarr.length; eix++)
                {
                   // pointer to the nt_electrode object
                  var eptr = elarr[eix];

                  // electrode already loaded?
                  var el = NTElectrode.ByAddr[eptr];
                  var elInit = NTElectrode.InitByAddr[eptr];

                  debug_print("NTControl uses electrode #" + eix + " at 0x" + eptr.toString(16) + ", " + (el ? el.GetName() : "creating..."), false);

                  // no, create basic nt_electrode type without parent module
                  if(!elInit)
                    el = new NTElectrode(null, elarrname + "["+eix+"]", eptr, this.OnElectrodeFinish, this);
                  else if(el)
                    this.OnElectrodeFinish(this, el, eptr);
                  else 
                    elInit.RegisterCallback(this, this.OnElectrodeFinish);
                }
                // success!
                this._valid = true;
                
                var st = pcm.ReadUIntVariable(symbol + "->rom", SIZEOF_PTR).then((res) => {
                  var itf = res.data;

                  debug_print("Pointer of interface: 0x" + itf.toString(16), false);
                  // by comparing interface pointers with known ones
                  return this.CheckControlType(symbol, itf, 0);

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
}

NTControl.prototype.CheckControlType = function(symbol, itf, ct)
{
  if(ct<CONTROL_TYPES.length)
  {
      var st = pcm.GetSymbolInfo("nt_control_" + CONTROL_TYPES[ct] + "_interface").then((res) => {
      debug_print("Found interface: " + CONTROL_TYPES[ct], false);
      if(itf == res.xtra.addr)
      {
        this._type = CONTROL_TYPES[ct];
        debug_print("This interface is good one: " + CONTROL_TYPES[ct], false);

        this._controlsstatsymbol = symbol + "->rom->control_params";
        // define symbol for control parameters
        var st = pcm.GetSymbolInfo(this._controlsstatsymbol).then((res) => {
          var control_params_addr = res.xtra.addr;
          var st = pcm_read_ptr_new(control_params_addr).then((res) => {
            var st = pcm_define_symbol_new(this._controlsstatsymbol+"."+this._type, res.data, "nt_control_"+this._type).then((res) => {
              //done, continue
              return this.NTControlContinueInit1();
            })
            return st;
          })
          return st;
        })
        return st;
      }
      else if(ct<CONTROL_TYPES.length)
      {
        ct++;
        return this.CheckControlType(symbol, itf, ct);
      }
      return st;
    })
    .catch((err) => {
      if(ct<CONTROL_TYPES.length)
      {
        ct++;
        //this code is executed, when
        //a) fails reading the symbol of list of types   ...   continue listing
        //b) fails any promise in NTModuleContinueInit();...   print error
        //when initialized  this._type, the type was detected, print the error.
        if(this._type == "")
          this.CheckControlType(symbol, itf, ct);
        else
          this.OnError(err);
      }
    });
  }
  else if(this._type == "")
  {
    debug_print("Control type was not detected");
    return this.NTControlContinueInit1();
  }
  return 0;
}

NTControl.prototype.NTControlContinueInit1 = function()
{
    // change our class to the real one wrapping the functionality specific for the control type
    if(this._type)
    {
      var classname = "NTControl_" + this._type;
      return change_class_type(this, classname);
    }
    else
    {
      return this.NTControlFinish();
    }
}

NTControl.prototype.NTControlFinish = function()
{
    NTControl.All[this._control_index] = this;
    NTControl.ById[this._id] = this;
    NTControl.ByAddr[this._addr] = this;
    NTControl.BySymbol[this._symbol] = this;

    // create the control name
    this._name = "control_" + this._type + "_" + (this._control_index+Number(1));

    NTControl.ByName[this._name] = this;

    debug_print("NTControl defined at " + this._symbol + ", name=" + this._name + ", electrodes=" + this._electrodes.length, false);

    if(typeof this.onInitialized === 'function')
    {
      args = []
      args.push(this._id)
      this.onInitialized.apply(this, args);
    }
    return 0;
}

NTControl.prototype.OnElectrodeFinish = function(callback_class, el, eptr)
{
  if(el.IsValid())
    callback_class._electrodes.push(el);
  else
    debug_print("NTModule uses invalid electrode? 0x" + eptr.toString(16), true);

  if(typeof callback_class.OnElInit === 'function')
  {
    args = []
    args.push(el)
    callback_class.onInitialized.apply(this, args);
  }
  if((typeof callback_class.onInitialized === 'function')/* && (this._electrodes_cnt == this._electrodes.length)*/)
  {
    args = []
    args.push(callback_class._id)
    callback_class.onInitialized.apply(this, args);
  }
  
}

NTControl.prototype.OnError = function(msg)
{
  if(msg.hasOwnProperty("msg"))
    debug_print("Failed init NTControl:"+ msg.msg, true);
  else
    debug_print("Failed init NTControl:"+ msg, true);
}


NTControl.UpdateGuiAll = function()
{
    for(c in NTControl.All)
        NTControl.All[c].UpdateGui();
}

NTControl.All = new Array();
NTControl.ById = new Array();
NTControl.ByName = new Array();
NTControl.ByAddr = new Array();
NTControl.BySymbol = new Array();
document.NTControl = NTControl;


NTControl.prototype._className = "NTControl";

NTControl.prototype.IsValid = function()
{
    return this._valid;
}

NTControl.prototype.GetName = function()
{
    return this._name;
}

NTControl.prototype.GetSymbol = function()
{
    return this._symbol;
}

NTControl.prototype.GetElectrodes = function()
{
    return this._electrodes;
}

NTControl.prototype.GetFlags = function() 
{
   if(!pcm_read_var(this._symbol + "->flags", 4)) 
        return 0;
        
    return pcm.LastVariable_vValue;
}

NTControl.prototype.GetFlagsNew = function(callback)
{
    this.readcontrol = true;
    var st = pcm.ReadUIntVariable(this._symbol + "->flags", 4).then((res) => {
      this.readcontrol = false;
      callback(res.data);
    })
    .catch((err) => {
      this.OnError(err);
    });
}

NTControl.prototype.GetLink = function()
{
    var code = "NTControl.ById[\"" + this._id + "\"].OnLinkClicked()";
    return "<a href='pcmaster:void' onclick='" + code + "'><p>" + this._name + "</p></a>";
}

NTControl.prototype.GetType = function()
{
    return this._type;
}

NTControl.prototype.UpdateGui = function()
{
    // this function is typically overriden
}

NTControl.prototype.GetGuiTag = function()
{
    // this function is typically overriden
    return "void";
}
NTControl.prototype.GetGuiTags = function()
{
    // this function is typically overriden
    return "";
}
NTControl.prototype.GetGuiTagsLabels = function()
{
    // this function is typically overriden
    return "";
}

NTControl.prototype.OnLinkClicked = function()
{
    // this function is typically overriden
    alert("no graph avaliable for generic control type");
}

NTControl.prototype.GetGui = function()
{
    var str = "";
    var ctr = this;
    str += "<div class=\"top-modal-section\">";
    str += "<div class=\"top-modal-section-wrap\">";
    str += "<p class=\"modal-headline\">"+ctr._type+"</p>";
    str += "<p class=\"modal-state-ok\"><img src=\"./img/detected.svg\" class=\"icon-detected\">detected</p>";
    str += "</div>";
    str += ctr.GetGuiTag();
    str += "</div>";
    str += "<div class=\"modules-table\">";
    str += "<div class=\"modules-table-headline\">";
    str += "<div class=\"mhl-name\"><p>Name</p></div>";
    str += "<div class=\"mhl-electrodes\"><p>Electrodes</p></div>";
    str += ctr.GetGuiTagsLabels();
    str += "</div>";
    str += "<div class=\"modules-table-line\">";
    str += "<div class=\"mtl-name\">"+ctr.GetLink()+"</div>";
    var electrodes = "";
    for(e=0; e<ctr._electrodes.length; e++)
        electrodes += ctr._electrodes[e].GetLink();
    str += "<div class=\"mtl-electrodes\">";
    str += electrodes;
    str += "</div>";
    str += ctr.GetGuiTags();
    str += "<div class=\"cistic\"></div>";
    str += "</div>";
    str += "</div>";
  return str;
}


