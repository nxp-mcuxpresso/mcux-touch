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

//function for the html
var ActivePage = 0;
function start_page()
{
    SetCurrenPageIndex(ActivePage);
}

function start_page2()
{
    SetCurrenPageIndex(ActivePage);
}

function SelectTabIndex(tabindex, action)
{
    var strid = "tab"+String(tabindex);
    var obj = document.getElementById(strid);
    var objli = document.getElementById(tab_li_ids[tabindex]);
    
    if(obj)
    {
      if(action)
        obj.style.display = "Block";
      else
        obj.style.display = "None";
    }
    if(objli)
    {
      if(action)
        objli.className ="active-link";
      else
        objli.className = "";
    }
}

function SetCurrenPageIndex(index)
{
    ActivePage = index;
}

function SetCurrenPage(index)
{
    if(ActivePage != index)
    {
       //disable all items
       for(var i=0; i<5; i++)
       {
         SelectTabIndex(i, 0);
       }
	   
    }
    SelectTabIndex(index, 1);
    SetCurrenPageIndex(index);
}

