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


function OnError(msg, descr)
{
      if(msg.hasOwnProperty("msg"))
        debug_print(descr+":"+ msg.msg, true);
      else
        debug_print(descr+":"+ msg, true);
}

function GenerateVars(moduleid)
{
    var label = "nt_system";
    romSystemName = "nt_kernel_data.rom";
    var st = pcm_read_ptr_new(romSystemName).then((res) => {
      var nt_system_rom_ptr = res.data;
      // map nt_symbol type to the obtained pointer
      var tssvar = "_nt_system";
      var st = pcm.DefineSymbol(tssvar, nt_system_rom_ptr, "nt_system").then((res) => {
        
      })
      return st;
    })
    .catch((err) => {
	OnError(err, "Failed GenerateVars");

    });
    
    var i=0;
}

function GetModeFromWeb(formid, rbid)
{
	var i = -1;
	var formhtmlobj1 = document.getElementById(formid);
	if(formhtmlobj1)
  {
  	//get basic expert mode from both HW configurations
    var opt1 = formhtmlobj1.elements[rbid];
    if(opt1)
    {
      for(i=0; i<opt1.length; i++)
      {
        if(opt1[i].checked)
          break;
      }
    }
  }
  return i;
}

function onchangemode(obj, id, val)
{
  var path = GetPathFromPathId(id);
	var rndid = GetIdFromPathId(id);

	var selected1 = GetModeFromWeb(path+"-1_form", path+"-1rb");
  var selected2 = GetModeFromWeb(path+"-2_form", path+"-2rb");
  var opt1 = 0;
  var formhtmlobj1 = document.getElementById(path+"-1_form");
  if(formhtmlobj1)
    opt1 = formhtmlobj1.elements[path+"-1rb"];
  var opt2 = 0;
  var formhtmlobj2 = document.getElementById(path+"-2_form");
  if(formhtmlobj2)
    opt2 = formhtmlobj2.elements[path+"-2rb"];

  //when equel do nothing
  if(selected2 == selected1)
    return;
  //sync second
  if((rndid == "1") && opt2) 
  {
    for(i=0; i<opt2.length; i++)
    {
      if(selected1 == i)
        opt2[i].checked = true;
      else
        opt2[i].checked = false;
    }
  }
  //sync first
  if((rndid == "2") && opt1)
  {
    for(i=0; i<opt1.length; i++)
    {
      if(selected2 == i)
        opt1[i].checked = true;
      else
        opt1[i].checked = false;
    }
  }
  
  
  //list all xml, disable/enable ids
  var mode = val;
  var label = "nt_system";
  SetModeToWeb(document.data[label], label, label, document.data[label]["rndid"], mode, 'mode');
  label = "nt_module";
  SetModeToWeb(document.data[label], label, label, document.data[label]["rndid"], mode, 'mode');
  label = "nt_control";
  SetModeToWeb(document.data[label], label, label, document.data[label]["rndid"], mode, 'mode');
  label = "nt_electrode";
  SetModeToWeb(document.data[label], label, label, document.data[label]["rndid"], mode, 'mode');
  if(Number(mode))
    EnableAllElCheckboxes("nt_control.electrodes");
  var module_links = 0
  if("links" in document.data["nt_module"])
  {
    module_links = document.data["nt_module"]["links"];
  }
  if(module_links)
  {
    for(id in module_links)
    {
      for(modulerndid in module_links[id])
        UpdateSafetyVisibility(modulerndid);
    }
  }
  
}

function UpdateSafetyVisibility(rndid)
{
  var selected1 = GetModeFromWeb("modeid-1_form", "modeid-1rb");
  var mode = selected1;

  var tsi_type = true;
  if(GetValFromPage(rndid, "nt_module.interface") != "&nt_module_tsi_interface")
    tsi_type = false;

  var safetyhtmllabelobj = document.getElementById("nt_module.safety_params.gpio-"+rndid);
  if(safetyhtmllabelobj)
    safetyhtmllabelobj.style.display = !(tsi_type && mode) ? 'None' : 'block';  
  var safetyhtmlobj = document.getElementById("nt_module_safety_gpio_params.tablestart"+rndid+"div");
  if(safetyhtmlobj)
    safetyhtmlobj.style.display = !(tsi_type && mode) ? 'None' : 'block';

  safetyhtmllabelobj = document.getElementById("nt_system.safety_period_multiple-"+document.data["nt_system"]["rndid"]+"tr");
  if(safetyhtmllabelobj)
    safetyhtmllabelobj.style.display = !(tsi_type && mode) ? 'None' : 'block';  
  safetyhtmllabelobj = document.getElementById("nt_system.safety_crc_hw-"+document.data["nt_system"]["rndid"]+"tr");
  if(safetyhtmllabelobj)
    safetyhtmllabelobj.style.display = !(tsi_type && mode) ? 'None' : 'block';    
    
  var label = "nt_electrode";
  SetModeToWeb(document.data[label], label, label, document.data[label]["rndid"], !(tsi_type && mode) ? 0 : 1, 'showtsi');    
}

function PutStaricCfgToPage()
{
    //put line with name of config file
	var id = "cfgfile";
    var obj = {};
    obj['nodepath'] = id;
    obj['rndid'] = "";
    var stricon = Mustache.to_html(templates["sticon"], obj);
    obj = {};
    obj['nodepath'] = "modeid";
    obj['rndid'] = "";
    obj['onchange'] = "onchangemode";
    var modestr = Mustache.to_html(templates["rb_basic_exp"], obj);
    document.write(modestr);
    document.write("<table>");
    document.write("<tr><td>touch tool configuration (frdm_ke15z.xml):</td><td><input type=\"text\" name=\"fname\" id=\""+id+"-\"  value=\"frdm_ke15z.xml\"></td><td> <input type=\"button\" onclick=\"LoadCfgData('"+id+"-')\" value=\"Load\"><input type=\"button\" onclick=\"SaveCfgData('"+id+"-')\" value=\"Save\">"+stricon+"</td></tr>");

    obj = document.getElementById(id);
    if((document.cookie)&&(obj))
    	obj.value = document.cookie;

    
    var label = "paths";
    document.data[label] = {};
    document.data[label]["rndid"] = GetRandObjId();
    var paths = GetWebContentFromObj2(document.data[label], label, "template", document.data[label]["rndid"]);
    document.write(paths);
    document.write("</table>");
    id = "boardcfg";
    obj = {};
    obj['nodepath'] = id;
    obj['rndid'] = "";
    stricon = Mustache.to_html(templates["sticon"], obj)
    	
    document.write( "<input type=\"button\" onclick=\"LoadCfgBoard('"+id+"-')\" title=\"The actual configuration will be revriten by the configuration from connected Board (requires enabled FreeMASTER communication)\" value=\"Read Configuration from Board\">"+stricon+"<br>");

         
    label = "nt_system";
    document.data[label] = {};
    document.data[label]["rndid"] = GetRandObjId();
    var h = GetWebContentFromObj2(document.data[label], label, "template", document.data[label]["rndid"]);
    document.write(h);

    label = "nt_module";
    ddocument.ata[label] = {};
    document.data[label]["rndid"] = GetRandObjId();
    var nt_module = GetWebContentFromObj2(document.data[label], label, "template", document.data[label]["rndid"]);
    label = "tsi_config_t";

    label = "nt_control";
    document.data[label] = {};
    document.data[label]["rndid"] = GetRandObjId();
    label = "nt_controls";
    document.data[label] = {};
    document.data[label]["rndid"] = GetRandObjId();
    var h2 = GetWebContentFromObj2(document.data[label], label, "template", document.data[label]["rndid"]);
    document.write( h2 );
    console.log( h2 );
    label = "nt_electrodes";
    document.data[label] = {};
    document.data[label]["rndid"] = GetRandObjId();
    var h3 = GetWebContentFromObj2(document.data[label], label, "template", document.data[label]["rndid"]);
    document.write( h3 );
    document.write( nt_module );
}

function LoadCfgBoard(id)
{
    SetStatusLineRaw(id, STATUS_CMD_SENDING, "Started load Data from board."+"["+debug_get_timestamp()+"]");
    pcm.IsBoardDetected().then((res) => {

    if(res.data != false)
    {
    	
    	//delete all electrodes
    	var label = "nt_electrodes";
    	var obj = GetDescrObj(label+".tablediv");
    	DeleteDynSubStruct(obj, label, 2); //argument 2 resets the index to 0
    	incrementerSetCnt("nt_electrodes.tablediv", obj['rndid'], 0);
    	//add all electrodes
    	for(var i in NTElectrode.All)
    		incrementerinc("nt_electrodes.control_count",document.data[label]["rndid"], 1);
    	
    	//delete controls
    	label = "nt_controls";
    	obj = GetDescrObj(label+".tablediv");
    	DeleteDynSubStruct(obj, label, 2); //argument 2 resets the index to 0
    	incrementerSetCnt("nt_controls.tablediv", obj['rndid'], 0);
    	//add all default controls
    	for(var i in NTControl.All)
    		incrementerinc("nt_controls.control_count",document.data[label]["rndid"], 1);
        
    	//updatetype of controls
    	var basepath = "nt_control";
    	var pathinterface = basepath+".interface";
    	obj = GetDescrObj(pathinterface);
    	var objbase = GetDescrObj(basepath);
    	for(var i in NTControl.All)
    	{
    		rndid = GetLinkId(objbase['links'][i]);
    		var objstrname = GetDescrObj(basepath+".sructname");
    		obj['lasttext'] = GetHtmlObjVal(basepath+".sructname-"+rndid, objstrname['template']).slice(0, -2); //read the controls name and remove last 2 chars "_1"
    		SetHtmlObjVal(pathinterface+"-"+rndid, obj['template'], "&nt_control_"+NTControl.All[i].GetType()+"_interface");
    		enumonchange(id, pathinterface+"-"+rndid);
    	}
      
    	//delete modules
      label = "nt_module";
      obj = GetDescrObj(label+".links");
      if(obj)
      {
        for(i = 0; i<obj.length; i++)
        {
          rndid = GetLinkId(obj[i]);
          AddDellHwWebContent(rndid, 2); //delete
        }
      }
      
      obj = GetDescrObj("cs_config_t");
      if(obj && ("blockedshowrndid" in obj)) obj["blockedshowrndid"] = [];
      
      obj = GetDescrObj("tsi_config_t");
      if(obj&&("blockedshowrndid" in obj)) obj["blockedshowrndid"] = [];


      
    	label = "nt_modules";

    	obj = GetDescrObj(label+".tablediv");
    	DeleteDynSubStruct(obj, label, 2); //argument 2 resets the index to 0
    	incrementerSetCnt("nt_modules.tablediv", obj['rndid'], 0);
      
    	//add all default modules
    	for(var i in NTModule.All)
    		incrementerinc("nt_modules.control_count",document.data[label]["rndid"], 1);
        
    	//updatetype of modules
    	var basepath = "nt_module";
    	var pathinterface = basepath+".interface";
    	obj = GetDescrObj(pathinterface);
    	var objbase = GetDescrObj(basepath);
    	for(var i in NTModule.All)
    	{
    		rndid = GetLinkId(objbase['links'][i]);
    		var objstrname = GetDescrObj(basepath+".sructname");
    		obj['lasttext'] = GetHtmlObjVal(basepath+".sructname-"+rndid, objstrname['template']).slice(0, -2); //read the controls name and remove last 2 chars "_1"
    		SetHtmlObjVal(pathinterface+"-"+rndid, obj['template'], "&nt_module_"+NTControl.All[i].GetType()+"_interface");
    		enumonchange(id, pathinterface+"-"+rndid);
    	}
        
    	//updatetype of electrodes
    	basepath = "nt_electrode";
    	pathinterface = basepath+".keydetector_interface";
    	obj = GetDescrObj(pathinterface);
    	objbase = GetDescrObj(basepath);
    	for(var i in NTElectrode.All)
    	{
    		rndid = GetLinkId(objbase['links'][i]);
    		var objstrname = GetDescrObj(basepath+".sructname");
    		SetHtmlObjVal(pathinterface+"-"+rndid, obj['template'], "&nt_keydetector_"+NTElectrode.All[i].GetKeydetType()+"_interface");
    		enumonchange(id, pathinterface+"-"+rndid);
    	}
    	//updatetype of shielding electrodes
    	var electrode_basepath = "nt_electrode";
    	var electrodebaseobj = GetDescrObj(electrode_basepath);
    	for(var el_i in NTElectrode.All)
    	{
        	//read pointer for shielding and check with addr of electrode and select the right el/index in enum
    		el_addr = NTElectrode.All[el_i]._shielding_addr;
    		if(el_addr == 0)
    			continue;
    		for(var el_check_i in NTElectrode.All)
    		{
    			if(NTElectrode.All[el_check_i]._romaddr == el_addr)
    			{
    				var elenumindex = Number(el_check_i);
    				if(el_check_i<el_i)
    					elenumindex = Number(el_check_i)+Number(1);
    				SetHtmlObjNum("nt_electrode.shielding_electrode-"+elid, "tableenum", elenumindex);
    				break;
    			}
    		}
    	}

      //update shared config for electrodes
      var detected_shared_cfg = -1;
      for(var el_i in NTElectrode.All)
      {
        detected_shared_cfg = -1;
        for(var el_i2 in NTElectrode.All)
        {
          if(el_i == el_i2)
            break;
          if((NTElectrode.All[el_i]._keydetcfg_addr == NTElectrode.All[el_i2]._keydetcfg_addr) && (detected_shared_cfg == -1))
          {
            detected_shared_cfg = Number(el_i2);
            break;
          }
        }
        if(detected_shared_cfg>=0)
        {
          //set shared config to electrode el_i with electrode el_i2\
          var elid = GetLinkId(electrodebaseobj['links'][el_i]);
          SetHtmlObjNum('nt_electrode.share_keydet_cfg-'+elid, "tableenumsubstrshareditem", Number(el_i2)+Number(1));
          onchangetableenumsubstrshareditem(this,'nt_electrode.share_keydet_cfg-'+elid);
        }
      }
      //update shared configrraiton for modules
      var module_basepath = "nt_module";
      var modulebaseobj = GetDescrObj(module_basepath);
      for(var mod_i in NTModule.All)
      {
        var detected_shared_cfg = -1;
        for(var mod_i2 in NTModule.All)
        {
          if(mod_i == mod_i2)
            break;
          if((NTModule.All[mod_i]._hw_config_addr == NTModule.All[mod_i2]._hw_config_addr) && (detected_shared_cfg == -1))
          {
            detected_shared_cfg = Number(mod_i2);
            break;
          }
        }
        if(detected_shared_cfg>=0)
        {
          //set shared config to electrode el_i with electrode el_i2\
          var modid = GetLinkId(modulebaseobj['links'][mod_i]);
          SetHtmlObjNum('nt_module.share_tsi_hw_cfg-'+modid, "tableenumsubstrshareditem", Number(mod_i2)+Number(1));
          onchangetableenumsubstrshareditem(this,'nt_module.share_tsi_hw_cfg-'+modid);
        }
      }
    	
    	//update checkboxes/electrodes in controls
    	var control_basepath = "nt_control";
    	var controlbaseobj = GetDescrObj(control_basepath);
    	var electrode_basepath = "nt_electrode";
    	var electrodebaseobj = GetDescrObj(electrode_basepath);
    	
    	for(var ctrl_i in NTControl.All)
    	{
    		var ctrl_els_arr = NTControl.All[ctrl_i]._electrodes;
    		//for(elindex in electrodebaseobj)
    		for(var el_i in NTElectrode.All)
    		{
    			for(var control_el_i in ctrl_els_arr)
    			{
    				if(NTElectrode.All[el_i]._addr == ctrl_els_arr[control_el_i]._addr)
        			{
        				var ctrlid = GetLinkId(controlbaseobj['links'][ctrl_i]);
        				var elid = GetLinkId(electrodebaseobj['links'][el_i]);
        				SetHtmlObjVal(ctrlid+"_"+elid, "checkboxitem", 1);
        				checkboxitemonchange(1,"nt_control.electrodes-"+ctrlid+"_"+elid);
        			}
    			}
    		}
    	}
      
      
    	//update checkboxes/electrodes in modules
    	var control_basepath = "nt_module";
    	var controlbaseobj = GetDescrObj(control_basepath);
    	var electrode_basepath = "nt_electrode";
    	var electrodebaseobj = GetDescrObj(electrode_basepath);
    	
    	for(var ctrl_i in NTModule.All)
    	{
    		var ctrl_els_arr = NTModule.All[ctrl_i]._electrodes;
    		//for(elindex in electrodebaseobj)
    		for(var el_i in NTElectrode.All)
    		{
    			for(var control_el_i in ctrl_els_arr)
    			{
    				if(NTElectrode.All[el_i]._addr == ctrl_els_arr[control_el_i]._addr)
        			{
        				var ctrlid = GetLinkId(controlbaseobj['links'][ctrl_i]);
        				var elid = GetLinkId(electrodebaseobj['links'][el_i]);
        				SetHtmlObjVal(ctrlid+"_"+elid, "checkboxitem", 1);
        				checkboxitemonchange(1,"nt_module.electrodes-"+ctrlid+"_"+elid);
        			}
    			}
    		}
    	}
      
      //update rest of items
    	var label = "nt_system";
    	ReadDatafromBoardToWeb(document.data[label], label, label, document.data[label]["rndid"]);
    	label = "nt_module";
    	ReadDatafromBoardToWeb(document.data[label], label, label, document.data[label]["rndid"]);
    	label = "nt_control";
    	ReadDatafromBoardToWeb(document.data[label], label, label, document.data[label]["rndid"]);
    	label = "nt_electrode";
    	ReadDatafromBoardToWeb(document.data[label], label, label, document.data[label]["rndid"]);
      SetStatusLineRaw(id, STATUS_OK, "Data from board was loaded."+"["+debug_get_timestamp()+"]");
      }
      else
      {
      SetStatusLineRaw(id, STATUS_ERROR, "Board not connected."+"["+debug_get_timestamp()+"]");
      }
    })
    .catch((err) => {
    	OnError(err, "Failed LoadCfgBoard");
	SetStatusLineRaw(id, STATUS_ERROR, "Board not connected."+"["+debug_get_timestamp()+"]");
    });
}

function LoadCfgData(id)
{
    var str = "";
    //load file
    var filename = "nt_cfg.xml";
    var obj = document.getElementById(id);
    if(obj)
        filename = obj.value;
    SetStatusLineRaw(id, STATUS_CMD_SENDING, "Started load Data from"+filename+"."+"["+debug_get_timestamp()+"]");
    var st = pcm.LocalFileOpen("FMSTR_PROJECT_PATH/"+filename,"r").then((res) => {
      if(res.success)
      {
        var paramFile = res.xtra.retval;
        var st = pcm.LocalFileReadString(paramFile).then((readdata) => {
            var fileString = readdata.data;
            var st = pcm.LocalFileClose(paramFile).then((res) => {
              //json to dict array
              var cfgdata = JSON.parse(fileString);
              //set data to web
              var label = "paths";
              SetDataToWeb2(document.data[label], cfgdata[label][0], label, label, document.data[label]['rndid']);
              var idboardfile = "paths.boardfile-"+document.data[label]['rndid'];
              var file = document.getElementById(idboardfile + "boardfile");
              filename="";
              if(file)
                filename = file.value;
              var st = parseboardcfgupdatepins(idboardfile, true).then((res) => {
                var descrPathsObj = GetDescrObj2("paths.boardfile");
                var descrPinInputObj = GetDescrObj2("nt_electrode.pin_input");
                if((descrPathsObj) || (descrPinInputObj))
                {
                  if("cfg_str" in descrPathsObj)
                    descrPinInputObj["cfg_str"] = descrPathsObj["cfg_str"];
                }
                
                obj = GetDescrObj("cs_config_t");
                if(obj && ("blockedshowrndid" in obj)) obj["blockedshowrndid"] = [];
                obj = GetDescrObj("tsi_config_t");
                if(obj&&("blockedshowrndid" in obj)) obj["blockedshowrndid"] = [];

                label = "nt_system";
                SetDataToWeb2(document.data[label], cfgdata[label][0], label, label, document.data[label]['rndid']);
                label = "nt_modules";
                SetDataToWeb2(document.data[label], cfgdata[label][0], label, label, document.data[label]['rndid']);
                label = "nt_controls";
                SetDataToWeb2(document.data[label], cfgdata[label][0], label, label, document.data[label]['rndid']);
                label = "nt_electrodes";
                SetDataToWeb2(document.data[label], cfgdata[label][0], label, label, document.data[label]['rndid']);
                label = "nt_electrode.share_keydet_cfg";
                SyncOnChange(document.data[label], label)
                SetStatusLineRaw(id, STATUS_OK, "Data from config file "+filename+" was loaded."+"["+debug_get_timestamp()+"]");
              })
              return st;
          });
          return st;
        });
      }
      else
      {
        SetStatusLineRaw(id, STATUS_ERROR, "Board config file '"+filename+"'' was not found"+"["+debug_get_timestamp()+"]");
      }
      return st;
    })
    .catch((err) => {
    	OnError(err, "Failed LoadCfgData");
      SetStatusLineRaw(id, STATUS_ERROR, "Board config file '"+filename+"'' was not found"+"["+debug_get_timestamp()+"]");
    });
}

function SaveCfgData(id)
{
    var cfgdata = {};
    var filename = "nt_cfg.xml";
    var obj = document.getElementById(id);
    if(obj)
        filename = obj.value;
    //convert to dict array
    SetStatusLineRaw(id, STATUS_CMD_SENDING, "Started store Data to"+filename+"."+"["+debug_get_timestamp()+"]");
    var label = "nt_system";
    cfgdata[label] = GetDataFromWeb2(document.data[label], label, label, document.data[label]['rndid']);
    label = "nt_modules";
    cfgdata[label] = GetDataFromWeb2(document.data[label], label, label, document.data[label]['rndid']);
    label = "nt_controls";
    cfgdata[label] = GetDataFromWeb2(document.data[label], label, label, document.data[label]['rndid']);
    label = "nt_electrodes";
    cfgdata[label] = GetDataFromWeb2(document.data[label], label, label, document.data[label]['rndid']);

    label = "paths";
    cfgdata[label] = GetDataFromWeb2(document.data[label], label, label, document.data[label]['rndid']);

    // convert to json
    var str = JSON.stringify(cfgdata);
    console.log(str);
    var filename = "nt_cfg.xml";
    var obj = document.getElementById(id);
    if(obj)
        filename = obj.value;

    //save to file
    var cfgFile = pcm.LocalFileOpen("FMSTR_PROJECT_PATH/"+filename,"w").then((res) => {
      if(res.success)
      {
        var cfgFile = res.xtra.retval;
        var st = pcm.LocalFileWriteString (cfgFile, str).then((res) => {
          var st = pcm.LocalFileClose(cfgFile).then((res) => {
            SetStatusLineRaw(id, STATUS_OK, "Data to config file "+filename+" was saved."+"["+debug_get_timestamp()+"]");
          });
          return st;
        });
        return st;
     }
     else
     {
      SetStatusLineRaw(id, STATUS_ERROR, "Data to config file "+filename+" was not opened."+"["+debug_get_timestamp()+"]");
     }
    })
    .catch((err) => {
      OnError(err, "Failed SaveCfgData");
      SetStatusLineRaw(id, STATUS_ERROR, "Data to config file "+filename+" was not saved."+err.msg+"["+debug_get_timestamp()+"]");
    });
    document.cookie = filename;
}

function GenerateOut(id)
{
  var str = "/*\n  * Copyright 2013 - 2016, Freescale Semiconductor, Inc. \n  * Copyright 2016-2021 NXP \n  * All rights reserved.\n * \n  * SPDX-License-Identifier: BSD-3-Clause \n */ \n /* \n Static configuration c file for the NXP Touch Library \n generated by the NXP Touch GUI Tool\n*/\n";
  var str = str + "#include \"nt_setup.h\"\n#include \"board.h\"\n\n"
  SetStatusLineRaw(id, STATUS_CMD_SENDING, "Started store Data to static files."+"["+debug_get_timestamp()+"]");
  var label;
  label = "nt_crosstalk";
  const n = document.data[label]["current"];
  str += n.calculateAverageOfAverages();
  label = "nt_keydetector_afid";
  str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);
  label = "nt_keydetector_safa";
  str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);
  label = "nt_keydetector_usafa";
  str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);
  label = "nt_electrode";
  str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);
  label = "tsi_config_t";
  str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);
  label = "cs_config_t";
  str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);
  
  {
    label = "nt_tsi_recalib_config";
    str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);
    if(GetModeFromWeb("modeid-1_form", "modeid-1rb"))
    {
      label = "nt_module_safety_gpio_params";
      str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);    
    }
  }
    
  label = "nt_control_el_arr";
  document.data[label]["rndid"] = 4;
  str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);
  label = "nt_control_arotary";
  str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);
  label = "nt_control_aslider";
  str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);
  label = "nt_control_keypad";
  str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);
  label = "nt_control_proxi";
  str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);
  label = "nt_control_matrix";
  str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);
  
  label = "nt_control";
  str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);
  
  label = "nt_module_el_arr";
  document.data[label]["rndid"] = 3;
  str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);
  label = "nt_module";
  str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);
  label = "nt_control_arr";
  document.data[label]["rndid"] = 1;
  str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);
  label = "nt_module_arr";
  document.data[label]["rndid"] = 2;
  str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);
  label = "nt_system";
  str = str + GenFileContent2(document.data[label], label, label, document.data[label]["rndid"]);
  
  str = str + "void nt_enable(void)\n{\n";
  label = "nt_electrode";
  str = str + GenEnFunctContent(document.data[label], label, label, document.data[label]["rndid"]);
  label = "nt_control";
  str = str + GenEnFunctContent(document.data[label], label, label, document.data[label]["rndid"]);
  str = str + "}\n";
  
  //generate template callbacks
  str = str + "\n//Following functions are generate, copy them to your application and implement there behaviour of events\n"
  str = str + callbacktemplates['System'];
  label = "nt_control";
  str = str + GenTemplateFunctContent(document.data[label], label, label, document.data[label]["rndid"]);
  
  //find template to get ID, the list
  
  console.log(str);
  var file = document.getElementById(id);
  var filename="";
  if(file)
    filename = file.value;
  
  //save to c file
  var genFile = pcm.LocalFileOpen("FMSTR_PROJECT_PATH/"+filename+".c","w").then((res) => {
    console.log(res.xtra.retval);
    var genFile = res.xtra.retval;
    var st = pcm.LocalFileWriteString (genFile, str).then((res) => {
      var st = pcm.LocalFileClose(genFile).then((res) => {
        SetStatusLineRaw(id, STATUS_OK, "Data to static file "+filename+" was saved."+"["+debug_get_timestamp()+"]");
      });
      return st;
    });
    return st;
  })
  .catch((err) => {
      OnError(err, "Failed GenerateOut");
    SetStatusLineRaw(id, STATUS_ERROR, "Data to config c file "+filename+" was not saved."+err.msg+"["+debug_get_timestamp()+"]");
  });


  if(document.getElementById("gencfgid"))
  {
     var res = str.split("\n");
     var finalstr = "<p class=\"debug-log\">" + res.join("</p><p class=\"debug-log\">") + "</p>";
     gencfgid.innerHTML = finalstr;
     
  }
  
  var strh = "/*\n  * Copyright 2013 - 2016, Freescale Semiconductor, Inc. \n  * Copyright 2016-2021 NXP \n  * All rights reserved.\n * \n  * SPDX-License-Identifier: BSD-3-Clause \n */ \n /* \n Static configuration header file for the NXP Touch Library \n generated by the NXP Touch GUI Tool\n*/\n";
  strh = strh + "#ifndef NT_SETUP_H\n";
  strh = strh + "#define NT_SETUP_H\n\n";
  strh = strh + "#include \"nt.h\"\n";

	  
  var labelh;
  labelh = "nt_electrode";
  strh = strh + GenHFileContent(document.data[labelh], labelh, labelh, document.data[labelh]["rndid"]);
  labelh = "nt_control";
  strh = strh + GenHFileContent(document.data[labelh], labelh, labelh, document.data[labelh]["rndid"]);
  labelh = "nt_module";
  strh = strh + GenHFileContent(document.data[labelh], labelh, labelh, document.data[labelh]["rndid"]);
  labelh = "nt_system";
  strh = strh + GenHFileContent(document.data[labelh], labelh, labelh, document.data[labelh]["rndid"]);
  strh = strh + "void nt_enable(void);\n";

  var labelm = "nt_control";
  strh = strh + GetMacroConfigInit(document.data[labelm], labelm, labelm, document.data[labelm]["rndid"], document.data["nt_control_macro_arr"]);
  strh = strh + "\n\n#endif\n";
  //save to h file
  var st = pcm.LocalFileOpen("FMSTR_PROJECT_PATH/"+filename+".h","w").then((res) => {
    if(res.success)
    {
      var genFile = res.xtra.retval;
      var st = pcm.LocalFileWriteString (genFile, strh).then((res) => {
        var st = pcm.LocalFileClose(genFile).then((res) => {
          SetStatusLineRaw(id, STATUS_OK, "Data to static file "+filename+" was saved."+"["+debug_get_timestamp()+"]");
        });
        return st;
      });
      return st;
    }
    else
    {
      SetStatusLineRaw(id, STATUS_ERROR, "Data to config h file "+filename+" was not opened.["+debug_get_timestamp()+"]");
    }
  })
  .catch((err) => {
      OnError(err, "Failed GenerateOut");
    SetStatusLineRaw(id, STATUS_ERROR, "Data to config c file "+filename+" was not saved."+err.msg+"["+debug_get_timestamp()+"]");
  });
  
}

function ChangePinInput(id)
{
  var st = parseboardcfgupdatepins(id, false).then((res) => {
    SyncNodeContentToWeb("nt_electrode.pin_input");
  })
}


function pcm_open_file(id, filename, param)
{
  return new Promise((resolve, reject) => {
    var st = pcm.LocalFileOpen(filename, param).then((res) => {
        resolve( { id: id, paramFile: res.xtra.retval, xtra: res.xtra}  );
    }).catch((err) => {
      if(err.hasOwnProperty("msg"))
        debug_print("Failed define var:"+ err.msg, true);
      else
        debug_print("Failed define var:"+ err, true);
      reject(err);
    });
  });
}


function pcm_read_file(id, paramFile)
{
  return new Promise((resolve, reject) => {
    var st = pcm.LocalFileReadString(paramFile).then((res) => {
      var st2 = pcm.LocalFileClose(paramFile);
      resolve( { id: id, data: res.data, xtra: res.xtra}  );
      return st2;
    }).catch((err) => {
      if(err.hasOwnProperty("msg"))
        debug_print("Failed define var:"+ err.msg, true);
      else
        debug_print("Failed define var:"+ err, true);
      reject(err);
    });
  });
}


function parseboardcfgupdatepins_string(id, fileString, filename)
{
  //json to dict array
  var filelines = fileString.split("\n");
  //remove cr, lf
  for(line in filelines)
    filelines[line] = filelines[line].replace(/[n\r]+/g, '');

  //get interesting lines (with TF_TSI_MUTUAL_CAP or TF_TSI_SELF_CAP)
  var electrodelines = [];

  for(line in filelines)
  {
    if(filelines[line].indexOf("TF_TSI_MUTUAL_CAP_") > 1)
      electrodelines.push(filelines[line]);
    if(filelines[line].indexOf("TF_TSI_SELF_CAP_") > 1)
      electrodelines.push(filelines[line]);
  }

  //get list of electrodes
  var electrodes = [];
  for(electrodedef in electrodelines)
  {
    var ellinearr = electrodelines[electrodedef].split(" ")
    electrodes.push(ellinearr[1])
  }

  //update data dict array by the pin names in electrodes array
  PutPinDefinitionsToElectrode("nt_electrode.pin_input", electrodes);
  SyncNodeContentToWeb("nt_electrode.pin_input");
  SetStatusLineRaw(id, STATUS_OK, "Pin configuration file "+filename+" was loaded.");


}
function parseboardcfgupdatepins(id, force_load)
{
  return new Promise((resolve, reject) => {
    var file = document.getElementById(id + "boardfile");
    var filename="";
    if(file)
      filename = file.value;
    var path = GetPathFromPathId(id); 
    var descrPathObj = GetDescrObj2(path);
    if(descrPathObj)
    {
      if((force_load) || (!("cfg_str" in descrPathObj)))
        descrPathObj["cfg_str"] = "";
      if(descrPathObj["cfg_str"] != "")
      {
        parseboardcfgupdatepins_string(id, descrPathObj["cfg_str"], filename);
        return 0;
      }
    }

    SetStatusLineRaw(id, STATUS_CMD_SENDING, "Started parsing file"+filename+"."+"["+debug_get_timestamp()+"]");
    if(filename == "")
      return;
    var st = pcm_open_file(id, "FMSTR_PROJECT_PATH/"+filename,"r").then((res) => {
      if(res.paramFile)
      {
        var st = pcm_read_file(res.id, res.xtra.retval).then((res) => {
            var fileString = res.data;
              var id = res.id;
              descrPathObj["cfg_str"] = fileString;
              parseboardcfgupdatepins_string(id, fileString, filename);
              resolve( { id: id } );
        })
      }
      else
      {
        SetStatusLineRaw(id, STATUS_ERROR, "Config file "+filename+" was not found"+"["+debug_get_timestamp()+"]");
      }
      return st;
    })
    .catch((err) => {
      OnError(err, "Failed parseboardcfgupdatepins");
      SetStatusLineRaw(id, STATUS_ERROR, "Config file "+filename+" was not found"+"["+debug_get_timestamp()+"]");
      reject(err);
    });
  });
}

  
function Put_Paths()
{
  var label = "paths";
  //var data = {};
  if(!(label in document.data)) document.data[label] = {};
  document.data[label]["rndid"] = GetRandObjId();
  var paths = GetWebContentFromObj2(document.data[label], label, "template", document.data[label]["rndid"]);

  var obj = document.getElementById("put_paths");
  if(obj)
    obj.innerHTML = paths;
}


function Put_nt_module_interface()
{
    var label = "moduleinterface";
    if(!(label in document.data)) document.data[label] = {};
    document.data[label]["rndid"] = GetRandObjId();
    
    var descrPathObj = GetDescrObj2(label+".modulesel");
    if(descrPathObj)
    {
      if("linktype" in descrPathObj)
      {
        var linkPathObj = GetDescrObj2(descrPathObj["linktype"]);
        var modulebase = ReducePath(descrPathObj["linktype"], 1)
        if(modulebase)
        {
          modulebaseObj = GetDescrObj2(modulebase)
          if('links' in modulebaseObj)
          {
            descrPathObj["default"] = [];
            linkarr = GetListOfLinks(modulebaseObj["links"]);
            for(link in linkarr)
            {
              var rndid = linkarr[link]['key'];
              var valname = GetValFromPage(rndid, descrPathObj["linktype"]);
              var item = {};
              item['item'] = rndid
              item['sublabel'] = valname;
              descrPathObj["default"].push(item);
            }
          }
          
        }
        
      }
    }
    var h = GetWebContentFromObj2(document.data[label], label, "template", document.data[label]["rndid"],"");

    var obj = document.getElementById("put_nt_module_interface");
    if(obj)
      obj.innerHTML = h;
      
    var moduleselpath = "moduleinterface.modulesel";
    var moduleselObj = GetDescrObj2(moduleselpath)
    var moduleselid = moduleselpath+"-"+moduleselObj['rndid'];
    onchangetableenummodsel(document.getElementById(moduleselid), moduleselid);
}

function PutSet_BlockImgContent(rndid, add_remove, label, objidbase, filter)
{
    var objid = objidbase+"-"+rndid;
    if(add_remove == 1)//add
    {
      document.data[label]["rndid"] = rndid;
      var h = GetWebContentFromObj2(document.data[label], label, "template", document.data[label]["rndid"], filter);
      var obj = document.getElementById(objidbase);
      if(obj)
      {
        var divobj = document.createElement("DIV");
        divobj.innerHTML = h;
        divobj.setAttribute("id", objid);
        divobj.setAttribute('style', 'display:none');
        obj.appendChild(divobj);
      }
    }
    else if(add_remove == 2)//del
    {
      var divobj = document.getElementById(objid);
      if(divobj)
      {
        divobj.outerHTML = "";
        delete divnobj;
      }
    }
    else if(add_remove == 4)//visible
    {
      var show = true;
      if("blockedshowrndid" in document.data[label])
      {
				for(linkindex in document.data[label]["blockedshowrndid"])
				{
					var id = GetLinkId(document.data[label]["blockedshowrndid"][linkindex])
					if(id == rndid)
					{
						show = false;
					}
				}
      }
      var divobj = document.getElementById(objid);
      if(divobj && show)
      {
        var type = '';
        divobj.style.display = type;
      }
    }
    else if(add_remove == 5)//hide
    {
      var divobj = document.getElementById(objid);
      if(divobj)
      {
        var type = '';
        type = 'none';
        divobj.style.display = type;
      }
    }
    else if(add_remove == 6)//hide all, show one
    {
      var divobj = document.getElementById(objid);
      if(divobj)
      {
        var type = '';
        type = 'none';
        divobj.style.display = type;
      }
    }
    else if(add_remove == 9)//register rndid, which wil be disabled all the time
    {
      if(!("blockedshowrndid" in document.data[label]))
        document.data[label]["blockedshowrndid"] = [];
      link = {};
      link[rndid] = label;
      document.data[label]["blockedshowrndid"].push(link);
    }
    else if(add_remove == 10)//unregister rndid, which wil be disabled all the time
    {
      if("blockedshowrndid" in document.data[label])
      {
				for(linkindex in document.data[label]["blockedshowrndid"])
				{
					var id = GetLinkId(document.data[label]["blockedshowrndid"][linkindex])
					if(id == rndid)
					{
						document.data[label]["blockedshowrndid"].splice(linkindex,1);
					}
				}
      }
    }

}

function Put_Main_Clock(rndid,add_remove)
{
    var label = "tsi_config_t";
    if(!(label in document.data)) document.data[label] = {};
    document.data[label]["rndid"] = rndid;
    var objidbase = "put_Main_Clock";
    var filter = "Main_Clock";
    PutSet_BlockImgContent(rndid, add_remove, label, objidbase, filter);
}



function Put_Comparator(rndid,add_remove)
{
    var label = "tsi_config_t";
    var objidbase = "put_Comparator";
    var filter = "Comparator";
    PutSet_BlockImgContent(rndid, add_remove, label, objidbase, filter);
}

function Put_PrechrgDischrg(rndid,add_remove)
{
    var label = "tsi_config_t";
    var objidbase = "put_PrechrgDischrg";
    var filter = "PrechrgDischrg";
    PutSet_BlockImgContent(rndid, add_remove, label, objidbase, filter);
}

function Put_Analog_Front_End_Mutualcap_sensing(rndid,add_remove)
{
    var label = "tsi_config_t";
    var objidbase = "put_Analog_Front_End_Mutual-cap_sensing";
    var filter = "Analog_Front_End_Mutual-cap_sensing";
    PutSet_BlockImgContent(rndid, add_remove, label, objidbase, filter);
}

function Put_Analog_FrontEnd_Selfcap_sensing(rndid,add_remove)
{
    var label = "tsi_config_t";
    var objidbase = "put_Analog_FrontEnd_Self-cap_sensing";
    var filter = "Analog_FrontEnd_Self-cap_sensing";
    PutSet_BlockImgContent(rndid, add_remove, label, objidbase, filter);

}

function Put_Digital_SINC_Filter(rndid,add_remove)
{
    var label = "tsi_config_t";
    
    var objidbase = "put_Digital_SINC_Filter";
    var filter = "Digital_SINC_Filter";
    PutSet_BlockImgContent(rndid, add_remove, label, objidbase, filter);
}

function Put_SSC_Divider(rndid,add_remove)
{
    var label = "tsi_config_t";
    
    var objidbase = "put_SSC_Divider";
    var filter = "SSC_Divider";
    PutSet_BlockImgContent(rndid, add_remove, label, objidbase, filter);
}

function Put_CLK_DIV(rndid,add_remove)
{
    var label = "cs_config_t";
    if(!(label in document.data)) document.data[label] = {};
    document.data[label]["rndid"] = rndid;
    var objidbase = "put_CLK_DIV";
    var filter = "CLK_DIV";
    PutSet_BlockImgContent(rndid, add_remove, label, objidbase, filter);
}

function Put_Counter(rndid,add_remove)
{
    var label = "cs_config_t";
    if(!(label in document.data)) document.data[label] = {};
    document.data[label]["rndid"] = rndid;
    var objidbase = "put_Counter";
    var filter = "Counter";
    PutSet_BlockImgContent(rndid, add_remove, label, objidbase, filter);
}

function Put_INTgeneration(rndid,add_remove)
{
    var label = "cs_config_t";
    if(!(label in document.data)) document.data[label] = {};
    document.data[label]["rndid"] = rndid;
    var objidbase = "put_INTgeneration";
    var filter = "INTgeneration";
    PutSet_BlockImgContent(rndid, add_remove, label, objidbase, filter);
}

function Put_Registers(rndid,add_remove)
{
    var label = "cs_config_t";
    if(!(label in document.data)) document.data[label] = {};
    document.data[label]["rndid"] = rndid;
    var objidbase = "put_Registers";
    var filter = "Registers";
    PutSet_BlockImgContent(rndid, add_remove, label, objidbase, filter);
}

function Put_oscillator(rndid,add_remove)
{
    var label = "cs_config_t";
    if(!(label in document.data)) document.data[label] = {};
    document.data[label]["rndid"] = rndid;
    
    var objidbase = "put_oscillator";
    var filter = "oscillator";
    PutSet_BlockImgContent(rndid, add_remove, label, objidbase, filter);
}
function Put_moduletype(rndid,add_remove)
{
    var label = "moduleinterface";
    if(!(label in document.data)) document.data[label] = {};
    document.data[label]["rndid"] = rndid;
    
    var objidbase = "modulesel";
    var filter = "";
}

function Put_nt_system()
{
    var label = "nt_system";
    if(!(label in document.data)) document.data[label] = {};
    document.data[label]["rndid"] = GetRandObjId();
    var h = GetWebContentFromObj2(document.data[label], label, "template", document.data[label]["rndid"]);

    var obj = document.getElementById("put_nt_system");
    if(obj)
      obj.innerHTML = h;
}

function Put_nt_controls()
{
    var label = "nt_controls";
    if(!(label in document.data)) document.data[label] = {};
    document.data[label]["rndid"] = GetRandObjId();
    var h = GetWebContentFromObj2(document.data[label], label, "template", document.data[label]["rndid"]);

    var obj = document.getElementById("put_nt_controls");
    if(obj)
      obj.innerHTML = h;
}

function Put_nt_electrodes()
{
    var label = "nt_electrodes";
    if(!(label in document.data)) document.data[label] = {};
    document.data[label]["rndid"] = GetRandObjId();
    var h = GetWebContentFromObj2(document.data[label], label, "template", document.data[label]["rndid"]);

    var obj = document.getElementById("put_nt_electrodes");
    if(obj)
      obj.innerHTML = h;
}

function Put_put_nt_module()
{
    var label = "nt_modules";
    if(!(label in document.data)) document.data[label] = {};
    document.data[label]["rndid"] = GetRandObjId();
    var h = GetWebContentFromObj2(document.data[label], label, "template", document.data[label]["rndid"], "nt_modules");

    var obj = document.getElementById("put_nt_module");
    if(obj)
      obj.innerHTML = h;
}

function Put_nt_crosstalk() {
  var label = "nt_crosstalk";
  if(!(label in document.data)) document.data[label] = {};
  document.data[label]["rndid"] = GetRandObjId();
  const n = new CrossTalk(4, "tab", 2);
  const av = new AverageMatrix(4, "tabavg");
  n.createMatrix(document.getElementById("touchArea"));
  av.createMatrix(document.getElementById("average"));
  n.setDiagonalValue();
  av.setDiagonalValue();
  document.data[label]["current"] = n;
  document.data[label]["average"] = av;
}


function Build_WebContent()
{
  Put_Paths();
  Put_nt_module_interface();
  Put_nt_system();
  Put_nt_controls();
  Put_nt_electrodes();
  Put_put_nt_module();
  Put_nt_crosstalk();
}

function AddDellHwWebContent(rndid, add_remove)
{
  Put_Main_Clock(rndid, add_remove);
  Put_Comparator(rndid, add_remove);
  Put_PrechrgDischrg(rndid, add_remove);
  Put_Analog_Front_End_Mutualcap_sensing(rndid, add_remove);
  Put_Analog_FrontEnd_Selfcap_sensing(rndid, add_remove);
  Put_Digital_SINC_Filter(rndid, add_remove);
  Put_SSC_Divider(rndid, add_remove);
  Put_CLK_DIV(rndid, add_remove);
  Put_Counter(rndid, add_remove);
  Put_INTgeneration(rndid, add_remove);
  Put_Registers(rndid, add_remove);
  Put_oscillator(rndid, add_remove);
  Put_moduletype(rndid, add_remove);
}