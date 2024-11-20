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
 

var pcm = 0; // the main FreeMASTER communication object
var nt_good = false;
var SIZEOF_PTR = 4;
var MODULE_TYPES = ["tsi", "gpio", "gpioint", "cs"];
var CONTROL_TYPES = ["keypad", "rotary", "slider", "arotary", "aslider", "proxi", "matrix"];
var KEYDETECTOR_TYPES = ["safa", "usafa","mbw", "afid"];
var DOC_PATH="../../doc/_OUTPUT_/html/"
var rpcs_addr = "localhost:41000";
var rpc_addr_loaded = "default"

function StartGUI()
{
  document.data = {};
  // default address
  rpcs_addr = "localhost:41000";
  rpc_addr_loaded = "default"
  // freemaster 3.1.3 and above provides the info about itself:
  if(typeof(FreeMASTER) != "undefined")
  {
    rpcs_addr = String(FreeMASTER.rpcs_addr);
    rpc_addr_loaded = "FM config"
  }
  pcm = new PCM(rpcs_addr, on_connected, on_error, on_error);
  pcm.OnServerError = on_error;
  pcm.OnSocketError = on_error;
}



/* Desktop FreeMASTER listens on port 41000 by default, unless this is
* overridden on command line using /rpcs option. FreeMASTER Lite
is configurable. */

function on_connected()
{
/* Typically, you want to enable extra features to make use of the full API
* provided by desktop application. Leave this disabled and avoid any extra
* features when creating pages compatible with FreeMASTER Lite. */
  nt_static_cfg_init();
  debug_print("JSON RPC session:"+rpcs_addr+" ("+rpc_addr_loaded+")", false);
}
function on_error(err)
{
  /* Erors are reported in the status field. */
  debug_print("failed connect to FreeMASTER:"+err.message, true);
}


function OnElectrodeInit(id)
{
  NTElectrode.All[0].CreateProperty(3);
}

var maintimer_initialised = false;
function OnModuleInit(id)
{
    var str = "";
    
    for(m in NTModule.All)
    {
      str += NTModule.All[m].GetGui();
    
    }
    if(document.getElementById("tab1"))
    {
      tab1.innerHTML = str;
    }
    GenerateVars(id);
    
    if(maintimer_initialised == false)
    {
      setInterval(main_timer, 100);
      maintimer_initialised = true;
    }
}

async function OnControlInit(id)
{
    str = "";
    for(c in NTControl.All)
      str += NTControl.All[c].GetGui();
    if(document.getElementById("tab2"))
    {
      tab2.innerHTML = str;
    }
}

async function nt_static_cfg_init()
{
  //var paramFile = pcm.LocalFileOpen("src/news.xml","r");
  var config_string = "";
  try {
    var file_st = await pcm.LocalFileOpen("FMSTR_PACKDIR_PATH/test1.xml","r");
    debug_print("load web cfg xml", true);
    if(file_st.success == false)
    {
      debug_print("Failed load web cfg xml", true);
      return;
    }
  
    var st = await pcm.LocalFileReadString(file_st.data);
    
    var file_close_st = await pcm.LocalFileClose(file_st.data);
    
    if((st.success == false)||(st.xtra.retval<=0))
    {
      debug_print("Failed load web cfg xml", true);
      return;
    }
    config_string = st.data;
    if(file_close_st.success == false)
    {
      debug_print("Failed close web cfg xml", true);
      return;
    }

  }
  catch(err) {
    debug_print("Failed access to GUI configuration: "+err.msg, true);
  }
  if(config_string == "")
    return;
  try {
    //parse the xml configuration
    document.data = XmlStrToDataObj(config_string);

    st = await pcm.GetAppVersion();
    if(st.xtra.retval<0x01040400)
    {
      if(confirm("You need FreeMASTER version 1.4.4 or later to display the TSS GUI.\n\n" + 
               "Do you want to navigate to FreeMASTER download page now?")) 
      {
        window.location.href = "http://www.freescale.com/webapp/sps/site/prod_summary.jsp?code=FREEMASTER&tab=Design_Tools_Tab";
      }
      return;
    }

    Build_WebContent();
    SetCurrenPageIndex(ActivePage);

    st = await pcm.IsBoardDetected();
    if(st.data == false)
    {
      debug_print("No Board is detected!", true);
      return;
    }
    debug_print("Board is detected.");

    // enable FMSTR function to define symbol
    pcm.EnableExtraFeatures(true);
    pcm.EnableEvents(true);
    // detect NT objects
    nt_symbols_init(OnModuleInit, OnControlInit, OnElectrodeInit);
    const n = document.data["nt_crosstalk"]["current"];
    n.fillValues();
    n.variables();
  }
  catch(err) {
    debug_print("Failed to load GUI: "+err.msg, true);
  }
}




var timer_i=0;
function main_timer()
{
    var ix;

    // update all electrode and control objects, one at each timer callback    
    if(((ix=timer_i-0) < NTElectrode.All.length) && (ActivePage == 1))
    {
      if(ix in NTElectrode.All)
      {
        if(NTElectrode.All[ix].read)
          timer_i--;
        else
          NTElectrode.All[ix].UpdateGui();
      }

    }
    else if(((ix=timer_i-0) < NTControl.All.length) && (ActivePage == 2))
    {
        if (ix>=0)
        {
          if(ix in NTControl.All)
          {
            if(NTControl.All[ix].readcontrol)
              timer_i--;
            else
              NTControl.All[ix].UpdateGui();
          }
        }
    }
    else if(ActivePage == 3)
    {
        var tmp = 0;
        sel = document.getElementById("NTElectrode_select");
        if(sel)
          tmp = sel.selectedIndex;
        if(NTElectrode.All[tmp].read == 0)
        {
          NTElectrode.All[tmp].UpdateGuiElectrode();
          NTElectrode.All[tmp].UpdateGuiShieldElectrode();
          NTElectrode.All[tmp].UpdateGuiKeydetRaw("");
        }
    }
    else
        timer_i = -1;
    timer_i++;
}

function debug_get_timestamp()
{
  var date = new Date();
  datevalues = [
   date.getFullYear(),
   date.getMonth()+1,
   date.getDate(),
   date.getHours(),
   date.getMinutes(),
   date.getSeconds(),
   date.getMilliseconds(),
  ];
  return datevalues[0]+"/"+datevalues[1]+"/"+datevalues[2]+" "+datevalues[3]+":"+datevalues[4]+":"+datevalues[5]+","+datevalues[6];
}

function debug_print(msg, err)
{
    color = err ? "red" : "black";
    if(document.getElementById("msglog") )
    {
    	msglog.innerHTML += "<p style=\"color:" + color + ";\" class=\"debug-log\">[" + debug_get_timestamp()+"] "+msg + "</p>";
    }
    return false;
}

function debug_assert(c, msg)
{
    if(!c)
        alert("Internal error: ASSERT failed " + msg ? msg : "!");
}

function debug_log(str)
{
  //console.log(str);
}

function pcm_write_var(name, size, val)
{
    if(!pcm.WriteUIntVariable(name, size, val))
    {
        debug_print("can not read variable \"" + name + "\", err=" + pcm.LastRetMsg, true);
        return false;
    }

    return true;
}


function pcm_write_var_new(name, size, val)
{
  return new Promise((resolve, reject) => {
    pcm.WriteUIntVariable(name, size, val).then((res) => {
      resolve( { name: name } );
    }).catch((err) => {
      debug_print("can not write variable \"" + name + "\", err=" + err.msg, true);
      reject(err);
    });
  });
}

function pcm_read_var(name, size)
{
    if(!pcm.ReadUIntVariable(name, size))
    {
        debug_print("can not read variable \"" + name + "\", err=" + pcm.LastRetMsg, true);
        return false;
    }

    return true;
}

function pcm_read_var_new(name, size)
{
  return new Promise((resolve, reject) => {
    pcm.ReadUIntVariable(name, size).then((res) => {
      resolve( { name: name, data: res.data} );
    }).catch((err) => {
      debug_print("can not read variable \"" + name + "\", err=" + err.msg, true);
      reject(err);
    });
  });
}

function pcm_read_ptr(name)
{
    if(!pcm.ReadUIntVariable(name, SIZEOF_PTR))
    {
        debug_print("can not read pointer \"" + name + "\", err=" + pcm.LastRetMsg, true);
        return false;
    }

    return true;
}

function pcm_read_ptr_new(name)
{
  return new Promise((resolve, reject) => {
    pcm.ReadUIntVariable(name, SIZEOF_PTR).then((res) => {
      resolve( { name: name, data: res.data} );
    }).catch((err) => {
      debug_print("can not read pointer \"" + name + "\", err=" + err.msg, true);
      reject(err);
    });
  });
}


function pcm_define_symbol(name, addr, type, size)
{
    if(!pcm.DefineSymbol(name, addr, type, size))
    {        
        debug_print("can not find structure type \"" + type + "\"", true);
        return false;        
    }

    debug_print("Script symbol defined: " + name + " = 0x" + addr.toString(16) + "&nbsp;&nbsp;&nbsp;type:" + type, false);
    return true;
}

function pcm_define_symbol_new(name, addr, type, size)
{
  return new Promise((resolve, reject) => {
    pcm.DefineSymbol(name, addr, type, size).then((res) => {
      debug_print("Script symbol defined: " + name + " = 0x" + addr.toString(16) + "&nbsp;&nbsp;&nbsp;type:" + type, false);
      resolve( true );
    }).catch((err) => {
      debug_print("can not find structure type \"" + type + "\"", true);
      reject(err);
    });
  });
}


function pcm_duplicate_symbol(new_sym, existing_sym)
{
    var ok = false;
    var addr, size;
    
    if(pcm_get_symbol_info(existing_sym))
    {
        ok = pcm_define_symbol(new_sym, pcm.LastSymbolInfo_addr, null, pcm.LastSymbolInfo_size);
    }
    
    return ok;
}

function pcm_define_variable(name, symbol, symbol_info)
{
    var ok = false;
    
    if(pcm_get_symbol_info(symbol_info ? symbol_info : symbol))
    {
        var def = { 
            "address" : symbol,
            "byte_size" : pcm.LastSymbolInfo_size
        };
         
        ok = pcm.DefineVariable(name, JSON.stringify(def));
    }
    
    return ok;
}

function pcm_define_variable_new(name, symbol, symbol_info)
{
  return new Promise((resolve, reject) => {
    var st = pcm.GetSymbolInfo(symbol_info ? symbol_info : symbol).then((res) => {
        var def = { 
            "address" : symbol,
            "byte_size" : res.xtra.size,
            "name" : name
        };
        var st = pcm.DefineVariable(JSON.stringify(def)).then((res) => {
          resolve( true );
        });
    }).catch((err) => {
      if(err.hasOwnProperty("msg"))
        debug_print("Failed define var:"+ err.msg, true);
      else
        debug_print("Failed define var:"+ err, true);
      reject(err);
    });
  });
}

function pcm_define_variable_bit(name, symbol, bit_shift, symbol_info)
{
    var ok = false;
    
    if(pcm_get_symbol_info(symbol_info ? symbol_info : symbol))
    {
        var def = { 
            "address" : symbol,
            "byte_size" : pcm.LastSymbolInfo_size,
            "bit_shift" : bit_shift,
            "bit_mask" : "one bit (0x1)"
        };
         
        ok = pcm.DefineVariable(name, JSON.stringify(def));
    }
    
    return ok;
}

function pcm_define_variable_bit_new(name, symbol, bit_shift, symbol_info)
{
  return new Promise((resolve, reject) => {
    var st = pcm.GetSymbolInfo(symbol_info ? symbol_info : symbol).then((res) => {
        var def = { 
            "address" : symbol,
            "byte_size" : res.xtra.size,
            "bit_shift" : bit_shift,
            "bit_mask" : "one bit (0x1)",
            "name" : name
        };
        var st = pcm.DefineVariable(JSON.stringify(def)).then((res) => {
          resolve( true );
        });
    }).catch((err) => {
      if(err.hasOwnProperty("msg"))
        debug_print("Failed define bit var:"+ err.msg, true);
      else
        debug_print("Failed define bit var:"+ err, true);
      reject(err);
    });
  });
}

function pcm_define_variable2(name, symbol, varType)
{
    var ok = false;
    
    var treatAsType = 0; //uint type
    if((varType == "SINT8") || (varType == "SINT16") || (varType == "SINT32") || (varType == "SINT64"))
      treatAsType = 1;
    else if((varType == "FLOAT") || (varType == "DOUBLE"))
      treatAsType = 2;
    else if((varType == "SFRAC16") || (varType == "SFRAC32"))
      treatAsType = 3;
    else if((varType == "UFRAC16") || (varType == "UFRAC32"))
      treatAsType = 4;
    
    var size = GetSizeFromType(varType);
    var def = { 
            "address" : symbol,
            "byte_size" : size,
            "treat_as" : treatAsType
    };
         
    ok = pcm.DefineVariable(name, JSON.stringify(def));
    return ok;
}

function pcm_define_variable2_new(name, symbol, varType)
{
  return new Promise((resolve, reject) => {

    var ok = true;

    var treatAsType = 0; //uint type
    if((varType == "SINT8") || (varType == "SINT16") || (varType == "SINT32") || (varType == "SINT64"))
      treatAsType = 1;
    else if((varType == "FLOAT") || (varType == "DOUBLE"))
      treatAsType = 2;
    else if((varType == "SFRAC16") || (varType == "SFRAC32"))
      treatAsType = 3;
    else if((varType == "UFRAC16") || (varType == "UFRAC32"))
      treatAsType = 4;
    
    var size = GetSizeFromType(varType);
    var def = { 
            "address" : symbol,
            "byte_size" : size,
            "treat_as" : treatAsType,
            "name" : name
    };

    var st = pcm.DefineVariable(JSON.stringify(def)).then((res) => {
      resolve(true);
    })  
    .catch((err) => {
      if(err.hasOwnProperty("msg"))
        debug_print("Failed define_variable2_new:"+ err.msg, true);
      else
        debug_print("Failed define_variable2_new:"+ err, true);
      reject(err);
    });
  });
}

function pcm_get_address_info(addr, size)
{
    if(!pcm.GetAddressInfo(addr, size))
    {
        debug_print("can not find address info for 0x" + addr.toString(16) + " and size " + size, true);
        return false;
    }

    debug_print("Address info: 0x" + addr.toString(16) + " is " + pcm.LastAddressInfo_name, false);
    return true;
}

function pcm_get_symbol_info(name)
{
    if(!pcm.GetSymbolInfo(name))
    {
        debug_print("can not find symbol info \"" + name + "\"", true);
        return false;
    }
    
    return true;
}


function pcm_get_symbol_info_new(name)
{
  return new Promise((resolve, reject) => {
    pcm.GetSymbolInfo(name).then((res) => {
      resolve( { name: name, data: res.data, xtra: res.xtra} );
    }).catch((err) => {
      debug_print("can not read pointer \"" + name + "\", err=" + err.msg, true);
      reject(err);
    });
  });
}

// read NULL-terminated array of pointers, argument is name of variable which 
// stores the pointer to the first element 

function read_ptr_array(ptrvar, maxsize)
{
    if(!maxsize)
        maxsize = 100;
        
    if(!pcm_read_ptr(ptrvar, SIZEOF_PTR))
    {
        debug_print("can not read_ptr_array address info \"" + ptrvar + "\"", true);
        return new Array(); // empty array
    }

    if(!pcm.LastVariable_vValue)
    {
        debug_print("The read_ptr_array can not read NULL array from \"" + ptrvar + "\"", false);
        return new Array(); // empty array
    }

    // there is an array of pointers there
    return read_ptr_array_at(pcm.LastVariable_vValue, maxsize);
}

// read NULL-terminated array of pointers, argument is pointer to first element 

function read_ptr_array_at(addr, maxsize)
{
    var ix, ptr;
    var arr = new Array();
    
    for(ix=0; true; ix++, addr+=SIZEOF_PTR)
    {
        // read array[ix]
        if(!pcm_read_ptr(addr))
            break;

        // pointer to the nt_control object itself
        ptr = pcm.LastVariable_vValue;
        // NULL terminates the controls list
        if(!ptr)
            break;
            
        arr.push(ptr);
        
        if(ix > maxsize)
        {
            debug_print("The read_ptr_array_at \"" + addr.toString(16) + "\" failed on sanity check, more than " + maxsize + " entries found", false);
            arr = [];
            return arr;
        }
    }

    debug_print("The read_ptr_array_at \"" + addr.toString(16) + "\" returns " + arr.length + " entries", false);
    return arr;
}

// read array of pointers, argument is name of variable which 
// stores the pointer to the first element 

function read_ptr_array_cnt(ptrvar, size)
{
    if(!pcm_read_ptr(ptrvar, SIZEOF_PTR))
    {
        debug_print("can not read_ptr_array address info \"" + ptrvar + "\"", true);
        return new Array(); // empty array
    }

    if(!pcm.LastVariable_vValue)
    {
        debug_print("The read_ptr_array can not read NULL array from \"" + ptrvar + "\"", false);
        return new Array(); // empty array
    }

    // there is an array of pointers there
    return read_ptr_array_cnt_at(pcm.LastVariable_vValue, size);
}

function read_ptr_array_cnt_new(ptrvar, size)
{
  return new Promise((resolve, reject) => {
    var st = pcm_read_ptr_new(ptrvar).then((res) => {
      var ptr = res.data;
      var st = pcm.ReadUIntArray(ptr, size, SIZEOF_PTR).then((res) => {
        resolve( { name: ptrvar, data: res.data} );
      })
    })
    .catch((err) => {
      debug_print("can not read pointer \"" + ptrvar + "\", err=" + err.msg, true);
      reject(err);
    });
  });
}

// read array of pointers, argument is pointer to first element 

function read_ptr_array_cnt_at(addr, size)
{
    if(!size)
    {
        debug_print("Invalid size of array \"" + size + "\"", true);
        return new Array(); // empty array
    }

    var ix, ptr;
    var arr = new Array();
    
    for(ix=0; ix<size; ix++, addr+=SIZEOF_PTR)
    {
        // read array[ix]
        if(!pcm_read_ptr(addr))
            break;
        // pointer to the nt_control object itself
        ptr = pcm.LastVariable_vValue;
        arr.push(ptr);
    }

    debug_print("The read_ptr_array_at \"" + addr.toString(16) + "\" returns " + arr.length + " entries", false);
    return arr;
}


function read_electrode(name, objid)
{
  return new Promise((resolve, reject) => {
    let name2 = JSON.parse(JSON.stringify(name));
    let objid2 = JSON.parse(JSON.stringify(objid));
    pcm.ReadUIntVariable(name2, 4, objid2).then((res) => {
      resolve( { name: name2, value: res.data, objid: objid2 } );
    }).catch((err) => {
      reject(err);
    });
  });
}

async function nt_symbols_init(onModuleInit, onControlInit, onElectrodeInit)
{
  var ix;
    
  nt_good = false;

  debug_print("parsing symbols...");
  try {
    var st = await pcm.DeleteAllScriptSymbols();
    if(st.success == false)
    {
      debug_print("failed delete script symbos!", true);
      return false;
    }
    modulesName = "nt_kernel_data.modules";

    st = await pcm.ReadUIntVariable(modulesName, SIZEOF_PTR)
    if(st.success == false)
    {
      debug_print("failed read pointer to " + modulesName, true);
      return false;
    }
    var nt_modules_ptr = st.data;
    debug_print("NT modules address: 0x" + nt_modules_ptr.toString(16), false);
      
    // map nt_symbol type to the obtained pointer
    var tssvar = "_nt_module_data";
    st = await pcm.DefineSymbol(tssvar, nt_modules_ptr, "nt_module_data")
    if(st.success == false)
    {
      debug_print("failed define symbol _nt_module_data at" + nt_modules_ptr, true);
      return false;
    }  
    debug_print("Script symbol defined: " + tssvar + " = 0x" + nt_modules_ptr.toString(16) + "&nbsp;&nbsp;&nbsp;type:" + "nt_module_data", false);
    
    st = await pcm.ReadUIntVariable("nt_kernel_data.modules_cnt", 1);
    if(st.success == false)
    {
      debug_print("failed read count of modules", true);
      return false;
    }  

    var modules_cnt = st.data;
    debug_print("Read nt_kernel_data.modules..." + modules_cnt);
    st = await pcm.ReadUIntVariable(modulesName, SIZEOF_PTR);
    if(st.success == false)
    {
      debug_print("failed pointer of first module", true);
      return false;
    }  

    var arrptr = st.data;
    st = await pcm.ReadUIntArray(arrptr, modules_cnt, SIZEOF_PTR);
    if(st.success == false)
    {
      debug_print("failed pointers of modules", true);
      return false;
    }  

    var arr_modules_ptrs = st.data;
    var ix=0;
    // any module?
    debug_print("Create variables for all modules");
    for(ix=0; ix<arr_modules_ptrs.length; ix++)
      new NTModule(modulesName + "["+ix+"]", arr_modules_ptrs[ix], onModuleInit, onElectrodeInit);
      
    st = await pcm.ReadUIntVariable("nt_kernel_data.controls_cnt", 1);
    if(st.success == false)
    {
      debug_print("failed read count of controls", true);
      return false;
    }  

    var controls_cnt = st.data;
    debug_print("read nt_kernel_data.controls..." + controls_cnt);
    var cptrvar = "nt_kernel_data.controls";
    st = await pcm.ReadUIntVariable(cptrvar, SIZEOF_PTR);
    if(st.success == false)
    {
      debug_print("failed read pointer of controls", true);
      return false;
    }
    
    var arrptr = st.data;
    st = await pcm.ReadUIntArray(arrptr, controls_cnt, SIZEOF_PTR)
    if(st.success == false)
    {
      debug_print("failed read pointers of controls", true);
      return false;
    }


    var arr_controls_ptrs = st.data;
    var ix=0;
    debug_print("Create variables for all controls");
    for(ix=0; ix<arr_controls_ptrs.length; ix++)
      new NTControl(cptrvar + "["+ix+"]", arr_controls_ptrs[ix], ix, onControlInit);
    nt_good = arr_modules_ptrs.length > 0;
  }
  catch(err) {
    debug_print("Failed freemaster request: "+err.msg, true);
  }
    return nt_good;
}

function fill_electrodes_table(tbl_id)
{
    var tbl = document.getElementById(tbl_id);
    if(!tbl)
        return;
        
    for(e in NTElectrode.All)
    {
        var el = NTElectrode.All[e];
        var row = tbl.insertRow(-1);
    
        for(col=0; col<4; col++)
        {
            var cell = row.insertCell(col);
            
            switch(col)
            {
            case 0: // name 
                cell.innerHTML = el.GetLink();
                break;
            case 1: // symbol
                cell.innerHTML = el._symbol;
                break;
            case 2: // module
                debug_print("TEST2", true);
                cell.innerHTML = el._module ? (el._module._name + ",<br>type=" + el._module._type) : "<i>none</i>";
                break;
            case 3:
                cell.innerHTML = el.GetGuiTag();
                break;
            }
        }    
    }
}

function fill_controls_table(tbl_id)
{
    var tbl = document.getElementById(tbl_id);
    if(!tbl)
        return;
        
    for(c in NTControl.All)
    {
        var ctl = NTControl.All[c];
        var row = tbl.insertRow(-1);
        var code = "on_select_control(NTControl.All[" + c + "])";
        
        for(col=0; col<4; col++)
        {
            var cell = row.insertCell(col);
            
            switch(col)
            {
            case 0: // name 
                cell.innerHTML = ctl.GetLink();
                break;
            case 1: // symbol
                cell.innerHTML = ctl._symbol;
                break;
            case 2: // electrodes
                cell.innerHTML = "";
                for(e=0; e<ctl._electrodes.length; e++)
                    cell.innerHTML += ctl._electrodes[e].GetLink() + ", ";
                break;
            case 3:
                cell.innerHTML = ctl.GetGuiTag();
                break;
            }
        }    
    }
}

var id_counter = 0;
function get_unique_id()
{
    id_counter++;
    return "uid" + id_counter;
}

function change_class_type(obj, classname)
{
    // remember that the object is typed dynamically
    obj._dynamically_typed = true;
    
    // take all prototype methods and memebrs
    for(var p in eval(classname + ".prototype"))
        obj[p] = eval(classname + ".prototype[p]")
    
    // call new class constructor passing it an existing pointer as "this"
    eval(classname + ".call(obj)");
}

function load_script_file(filename)
{
    // Adding the script tag to the head as suggested before
    var head = document.getElementsByTagName('head')[0];
    var script = document.createElement('script');
    script.type = 'text/javascript';
    script.src = filename;

    // Fire the loading
    head.appendChild(script);
}

function ToSign16(tmp)
{
  return tmp >= Math.pow(2,15)?(tmp-Math.pow(2,16)):tmp;
}

function ToSign32(tmp)
{
  return tmp >= Math.pow(2,31)?(tmp-Math.pow(2,32)):tmp;
}

function GetSizeFromType(varType)
{
  //check if parameter is valid number
  if(!isNaN(varType))
    return varType;
  //decode size from string
  if ((varType == "UINT8") || (varType == "SINT8"))
    return 1;
  else if ((varType == "UINT16") || (varType == "SINT16") || (varType == "UFRAC16") || (varType == "SFRAC16"))
    return 2;
  else if ((varType == "UINT32") || (varType == "SINT32") || (varType == "FLOAT") || (varType == "UFRAC32") || (varType == "SFRAC32"))
    return 4;
  else if ((varType == "UINT64") || (varType == "SINT64") || (varType == "DOUBLE"))
    return 8;
  else
    alert("GetSizeFromType(varType):Unsupported type of variable:"+varType);
};
