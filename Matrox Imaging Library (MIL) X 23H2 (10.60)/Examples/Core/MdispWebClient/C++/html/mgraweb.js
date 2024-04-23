"use strict";
//////////////////////////////////////////////////////////////
//
// File name: mgraweb.js
//
// Synopsis:  This program shows how to use MIL web API 
//            to access to a remote published objects.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//////////////////////////////////////////////////////////////

// URL for Websocket server
var loc = window.location
var URL ="ws://localhost:7682";
if (loc.hostname != "") {
    URL = "ws://"+ loc.hostname + ":7682";
}
var CanvasDisplayId = document.getElementById("displaycanvas");
var TextId = document.getElementById("textpre");
var UpdateId = document.getElementById("updatepre");
var MilWebApp   = MilWeb.M_NULL;
var Interactive = MilWeb.M_ENABLE;
var ContinueTag = 99991;
var StopTag     = 99992;
var QuitTag     = 99993;



// Interactive button
function OnSetInteractive() {
    var DispId = MilWeb.MappInquireConnection(MilWebApp, MilWeb.M_WEB_PUBLISHED_NAME, "Display",MilWeb.M_DEFAULT);
    if(DispId != MilWeb.M_NULL) {
	MilWeb.MdispControl(DispId, MilWeb.M_INTERACTIVE, MilWeb.M_ENABLE);
	console.log("set interactive");
    }
}

function OnContinue() {
    var MsgId = MilWeb.MappInquireConnection(MilWebApp, MilWeb.M_WEB_PUBLISHED_NAME, "MessageInput",MilWeb.M_DEFAULT);
    if(MsgId != MilWeb.M_NULL) {
	var DOMInputButton = document.getElementById("Continue");
	var ar = new ArrayBuffer(2);
	if (DOMInputButton.value == "Restart") {
	    MilWeb.MobjMessageWrite(MsgId,  ar,  MilWeb.M_DEFAULT,  StopTag, MilWeb.M_DEFAULT);
	    TextId.innerHTML = "";
	}
	else {
	    MilWeb.MobjMessageWrite(MsgId,  ar,  MilWeb.M_DEFAULT,  ContinueTag, MilWeb.M_DEFAULT);
	}
    }
}

function OnQuit() {
    var MsgId = MilWeb.MappInquireConnection(MilWebApp, MilWeb.M_WEB_PUBLISHED_NAME, "MessageInput",MilWeb.M_DEFAULT);
    if(MsgId != MilWeb.M_NULL) {
	var ar = new ArrayBuffer(2);
	MilWeb.MobjMessageWrite(MsgId,  ar,  MilWeb.M_DEFAULT,  QuitTag, MilWeb.M_DEFAULT);
    }
}

document.onkeyup = function(e) {
	console.log("keyboard event");
    // Ctrl + M (Continue)
	if (e.ctrlKey && e.which == 77) {
		console.log("OnContinue shortcut");
		OnContinue();
	// Ctrl + M (Quit)	
	} else if (e.ctrlKey && e.which == 66) {
		console.log("OnQuit shortcut");
		OnQuit();
	}
}

function ShowButtons(tag) {
    try {
	document.getElementById("Continue").style.visibility="hidden";
	document.getElementById("Continue").value="Continue";
	document.getElementById("Quit").style.visibility="hidden";
	var DOMContinueButton = document.getElementById("Continue");
	DOMContinueButton.disabled   = (Interactive != MilWeb.M_ENABLE);
	DOMContinueButton.style.visibility="visible";
	if (tag == StopTag) {
	    DOMContinueButton.value="Restart";
            var DOMQuitButton = document.getElementById("Quit");
	    DOMQuitButton.disabled   = (Interactive != MilWeb.M_ENABLE);
	    DOMQuitButton.style.visibility="visible";
	} else {
	    UpdateId.innerHTML = "";	    
	}
    } catch(e) {
	
    }
    
}
function InteractiveDisplayHandler(hookType, eventId, userVar) {
    var DispId         = MilWeb.MobjGetHookInfo(eventId, MilWeb.M_OBJECT_ID);
    var NewInteractive = MilWeb.MdispInquire(DispId, MilWeb.M_INTERACTIVE);
    if(NewInteractive != Interactive) {
	Interactive = NewInteractive;
	if(Interactive != MilWeb.M_ENABLE)
	    document.getElementById("IntState").innerHTML="Another web client owns the control ";
	else
	    document.getElementById("IntState").innerHTML="You are the web client who owns the control ";
	
	var DOMContinueButton = document.getElementById("Continue");
	DOMContinueButton.disabled   = (Interactive != MilWeb.M_ENABLE);
	var DOMQuitButton = document.getElementById("Quit");
	DOMQuitButton.disabled   = (Interactive != MilWeb.M_ENABLE);
    }
}

// Hook Called when "Display" object is connected
function ConnectDisplayHandler(hookType, eventId, userVar) {
    console.log("ConnectDisplayHandler");   
    var DisplayId = MilWeb.MobjGetHookInfo(eventId, MilWeb.M_OBJECT_ID);
    Interactive = MilWeb.MdispInquire(DisplayId, MilWeb.M_INTERACTIVE);
    MilWeb.MdispSelectWindow(DisplayId, CanvasDisplayId);
    MilWeb.MobjHookFunction(DisplayId, MilWeb.M_UPDATE_INTERACTIVE_STATE, InteractiveDisplayHandler);
    MilWeb.MobjControl(DisplayId, MilWeb.M_FRAME_RATE, 20);
    document.getElementById("Interactive").style.visibility="visible";
    
    if(Interactive != MilWeb.M_ENABLE)
	document.getElementById("IntState").innerHTML="Another web client owns the control ";
    else
	document.getElementById("IntState").innerHTML="You are the web client who owns the control ";

}

// Hook Called when "MessageOutput" object is updated
function UpdateMessageHandler(hookType, eventId, userVar) {
    var MsgId      = MilWeb.MobjGetHookInfo(eventId, MilWeb.M_OBJECT_ID);
    var MsgPtr     = {data:null};
    var MsgOutSize = {data:null};
    var MsgTag     = {data:null};
    var MsgStatus  = {data:null};
    var retstring;

    // Read message and display it in and HTML element
    MilWeb.MobjMessageRead(MsgId, MsgPtr, MilWeb.M_DEFAULT, MsgOutSize, MsgTag, MsgStatus, MilWeb.M_DEFAULT);
    if(MsgPtr.data) {
	retstring = MilWeb.convertUTF8ToString(MsgPtr.data);
	if(MsgTag.data == ContinueTag) 
	    TextId.innerHTML = retstring + "\n";
	else if(MsgTag.data == StopTag) {
	    UpdateId.innerHTML = retstring;
	} else
	    TextId.innerHTML = retstring;

	ShowButtons(MsgTag.data);
    }
}

// Hook Called when application is connected
function ConnectHandler(hookType, eventId, userVar) {
    console.log("ConnectHandler");
    
    // Connect to specific objects
    MilWebApp = MilWeb.MobjGetHookInfo(eventId, MilWeb.M_OBJECT_ID);
    if(MilWebApp) {
	var MilWebDisplay = MilWeb.MappInquireConnection(MilWebApp, MilWeb.M_WEB_PUBLISHED_NAME, "Display",MilWeb.M_DEFAULT);
	MilWeb.MobjHookFunction(MilWebDisplay, MilWeb.M_CONNECT, ConnectDisplayHandler);
	var MilWebMessage = MilWeb.MappInquireConnection(MilWebApp, MilWeb.M_WEB_PUBLISHED_NAME, "MessageOutput", MilWeb.M_DEFAULT);
	MilWeb.MobjHookFunction(MilWebMessage, MilWeb.M_UPDATE_WEB, UpdateMessageHandler);
	MilWeb.MappInquireConnection(MilWebApp, MilWeb.M_WEB_PUBLISHED_NAME, "MessageInput", MilWeb.M_DEFAULT);
    }   
}

// Hook Called when application is disconnected
function DisconnectHandler(hookType, eventId, userVar) {
    console.log("DisconnectHandler");
    var TopDiv = document.getElementById("top");
    TopDiv.innerHTML = "";
    var MilDiv = document.getElementById("MilDiv");
    MilDiv.innerHTML = "";
    var LogDiv = document.getElementById("log");
    LogDiv.innerHTML = "";
    document.getElementById("Continue").style.visibility="hidden";
    document.getElementById("Quit").style.visibility="hidden";
}

// Main function
// called when the page is loaded
function Start() {
    console.log("Start");
    var LogDiv = document.getElementById("log");
    LogDiv.innerHTML = "";
    
    var UserVar = {};
    MilWeb.MappOpenConnection(URL, MilWeb.M_DEFAULT, MilWeb.M_DEFAULT, UserVar);
    var MilWebApp = UserVar.data;
    if(MilWebApp) {
	// Connection hook
	MilWeb.MappHookFunction(MilWebApp, MilWeb.M_CONNECT, ConnectHandler);
	
	// Disconnect hook	  
	MilWeb.MappHookFunction(MilWebApp, MilWeb.M_DISCONNECT, DisconnectHandler);
    }
}
