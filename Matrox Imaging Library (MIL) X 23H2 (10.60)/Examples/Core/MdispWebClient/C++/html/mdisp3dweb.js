//////////////////////////////////////////////////////////////
//
// File name: mdisp3dweb.js
//
// Synopsis:  This program shows how to use MIL web API 
//            to access to a remote published objects.
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//////////////////////////////////////////////////////////////

// URL for Websocket server
var loc = window.location
var URL ="ws://localhost:7681";
if (loc.hostname != "") {
    URL = "ws://"+ loc.hostname + ":7681";
}
var CanvasDisplayId = document.getElementById("displaycanvas");
var TextId = document.getElementById("textpre");
var AppId = MilWeb.M_NULL;

// Hook Called when "Display" object is connected
function ConnectDisplayHandler(hookType, eventId, userVar) {
    console.log("ConnectDisplayHandler");   
    var DisplayId = MilWeb.MobjGetHookInfo(eventId, MilWeb.M_OBJECT_ID);
    MilWeb.MdispSelectWindow(DisplayId, CanvasDisplayId);
}

// Hook Called when "Message" object is updated
function UpdateMessageHandler(hookType, eventId, userVar) {
    console.log("ConnectMessageHandler");
    var MsgId      = MilWeb.MobjGetHookInfo(eventId, MilWeb.M_OBJECT_ID);
    var MsgPtr     = {data:null};
    var MsgOutSize = {data:null};
    var MsgTag     = {data:null};
    var MsgStatus  = {data:null};
    var retstring;

    // Read message and display it in and HTML element
    MilWeb.MobjMessageRead(MsgId, MsgPtr, MilWeb.M_DEFAULT, MsgOutSize, MsgTag, MsgStatus, MilWeb.M_DEFAULT);
    if(MsgPtr.data) 
    {
	retstring = MilWeb.convertUTF8ToString(MsgPtr.data);
	TextId.innerHTML = retstring;
	if(MsgTag.data == 2) {
	    var MilDiv = document.getElementById("MilDiv");
	    MilDiv.innerHTML = "";
	    if(AppId != MilWeb.M_NULL)
		MilWeb.MappCloseConnection(AppId);
	    AppId = MilWeb.M_NULL;
	}
    }
}

// Hook Called when application is connected
function ConnectHandler(hookType, eventId, userVar) {
    console.log("ConnectHandler");
    TextId.innerHTML = "";
    // Connect to specific objects
    var MilWebApp = MilWeb.MobjGetHookInfo(eventId, MilWeb.M_OBJECT_ID);
    var MilWebDisplay = MilWeb.MappInquireConnection(MilWebApp, MilWeb.M_WEB_PUBLISHED_NAME, "Display",MilWeb.M_DEFAULT);
    MilWeb.MobjHookFunction(MilWebDisplay, MilWeb.M_CONNECT, ConnectDisplayHandler);
    var MilWebMessage = MilWeb.MappInquireConnection(MilWebApp, MilWeb.M_WEB_PUBLISHED_NAME, "Message", MilWeb.M_DEFAULT);
    MilWeb.MobjHookFunction(MilWebMessage, MilWeb.M_UPDATE_WEB, UpdateMessageHandler);
}

// Hook Called when application is disconnected
function DisconnectHandler(hookType, eventId, userVar) {
    console.log("DisconnectHandler");
    var MilDiv = document.getElementById("MilDiv");
    MilDiv.innerHTML = "";
}

// Main function
// called when the page is loaded
function Start() {
    console.log("Start");
    var UserVar = {};      
    MilWeb.MappOpenConnection(URL, MilWeb.M_DEFAULT, MilWeb.M_DEFAULT, UserVar);
    AppId = UserVar.data;
    
    // Connection hook
    MilWeb.MappHookFunction(AppId, MilWeb.M_CONNECT, ConnectHandler);

    // Disconnect hook	  
    MilWeb.MappHookFunction(AppId, MilWeb.M_DISCONNECT, DisconnectHandler);
}
