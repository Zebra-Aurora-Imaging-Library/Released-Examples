//////////////////////////////////////////////////////////////////////////
//
// Filename   : mil.js
// Revision   : 10.60.0776
// Content    : MIL web client in javascript
//
// Copyright © Matrox Electronic Systems Ltd., 1992-2023.
// All Rights Reserved
//////////////////////////////////////////////////////////////////////////

"use strict";
var MilWeb = MilWeb || {
    //////////////////////////////////////////////////////////////////////////
    // MIL Defines
    //////////////////////////////////////////////////////////////////////////
    M_INVALID:                                  -1,
    M_DEFAULT:                                  0x10000000,
    M_DISABLE:                                  -9999,
    M_ENABLE:                                   -9997,
    M_NOW:                                      29,
    M_UNHOOK:                                   0x04000000,
    M_NULL:                                     0,
    M_SUCCESS:                                  0,
    M_FORCE:                                    -9998,

    // Object Type
    M_APPLICATION:                              0x00000200,
    M_IMAGE:                                    0x00000004,
    M_LUT:                                      0x00040000,
    M_KERNEL:                                   0x00400000,
    M_STRUCT_ELEMENT:                           0x08000000,
    M_ARRAY:                                    0x00000040,
    M_DISPLAY:                                  0x00200000,
    M_GROUP:                                    0x400000000000,
    M_GROUP_ID:                                 18,
    M_MODIFIED_BUFFER:                          0x40000000,
    M_BUFFER_ID:                                0x00160000,
    M_OBJECT_ID:                                0x001B0000,
    M_MESSAGE_MAILBOX:                          0x800000000000,
    M_OBJECT_NAME:                              47,
    M_OBJECT_NAME_SIZE:                         0x50000000000 + 47,
    M_OBJECT_TYPE:                              136,
    M_COMPONENT_ID_LIST:                        0x00100000,
    M_WEB_CLIENT_INDEX:                         219,
    M_READ_WRITE:                               0x00000010,
    M_READ_ONLY:                                0x00000020,
    M_NO:                                       0,
    M_WEB_GRAPHIC_LIST_INTERACTIVE:             0x00010000,
    M_WEB_KEYBOARD_USE:                         0x00020000,
    M_WEB_MOUSE_USE:                            0x00040000,
    M_MAILBOX_MODE_WEBTEXT:                     0x00000800,

    // Type
    M_SIGN:                                     1014,
    M_SIZE_BAND:                                1005,
    M_SIZE_BIT:                                 0x1007,
    M_UNSIGNED:                                 0x00000000,
    M_SIGNED:                                   0x08000000,
    M_FLOAT:                                   (0x40000000 | 0x08000000),
    M_USER_DATA_PTR:                            0x00000001,
    M_PIXEL_FORMAT:                             3023,
    M_DATA_FORMAT:                              1041,
    M_PITCH:                                    0x20000000,

    // Inquires/Controls
    M_WEB_PUBLISH:                              14,
    M_WEB_PUBLISHED_LIST:                       4,
    M_WEB_PUBLISHED_LIST_SIZE:                  5,
    M_WEB_PUBLISHED_NAME:                       6,
    M_SIZE_X:                                   1536,
    M_SIZE_Y:                                   1537,
    M_TYPE:                                     1008,
    M_SIZE_BYTE:                                5061,
    M_MESSAGE_LENGTH:                           16,
    M_INTERACTIVE:                              0,
    M_IMAGE_HOST_ADDRESS:                       0x00010000,
    M_FRAME_RATE:                               6002,
    M_CLOSE_CONNECTION:                         15504,

    // Mouse/Keyboards Events
    M_MOUSE_LEFT_BUTTON:                        0x00080000,
    M_MOUSE_RIGHT_BUTTON:                       0x00200000,
    M_MOUSE_MIDDLE_BUTTON:                      0x00100000,
    M_KEY_SHIFT:                                0x00010000,
    M_KEY_CTRL:                                 0x00020000,
    M_KEY_ALT:                                  0x00040000,
    M_KEY_WIN:                                  0x00400000,

    M_MOUSE_LEFT_BUTTON_DOWN:                   56,
    M_MOUSE_RIGHT_BUTTON_DOWN:                  57,
    M_MOUSE_LEFT_BUTTON_UP:                     58,
    M_MOUSE_RIGHT_BUTTON_UP:                    59,
    M_MOUSE_MOVE:                               64,
    M_MOUSE_WHEEL:                              65,
    M_MOUSE_MIDDLE_BUTTON_DOWN:                 66,
    M_MOUSE_MIDDLE_BUTTON_UP:                   67,
    M_KEY_DOWN:                                 68,
    M_KEY_UP:                                   69,
    M_MOUSE_LEAVE:                              85,
    M_UPDATE_INTERACTIVE_STATE:                 84,
    CLIENT_VERSION:                             0x1000000,
    APP_CLOSE_TIMEOUT:                          1000,

    // Hooks
    M_OBJ_HOOK_RANGE_START:                     0x000000800,

    M_CONNECTING:                               1,
    M_DISCONNECT:                               2,
    M_CONNECT:                                  3,
    M_TIMEOUT:                                  2077,
    M_UPDATE_WEB:                               3187,
    M_UPDATE_END:                               0x000000800 + 8,
    M_OBJECT_FREE:                              0x00010000,
    M_OBJECT_PUBLISH_WEB:                       38,
    M_COMPONENT_ADD:                            0x00040000,
    M_COMPONENT_REMOVE:                         0x000000800 + 10,
    M_GET_END:                                  32,
    M_READ_END:                                 11,
    M_MESSAGE_PTR:                              101,


    // Errors
    M_ERROR:                                    0x40000000,
    M_MESSAGE:                                  0x20000000,
    M_CURRENT:                                  0x00000002,
    M_CURRENT_FCT:                              0x00030000,
    M_CURRENT_SUB_NB:                           0x00000004,
    M_CURRENT_SUB:                              0x00050000,
    M_CURRENT_SUB_1:                            0x00050000,
    M_CURRENT_SUB_2:                            0x00060000,
    M_PRINT_DISABLE:                            0,
    M_PRINT_ENABLE:                             1,


    // Disp Hook Info
    M_MOUSE_POSITION_X:                         1,
    M_MOUSE_POSITION_Y:                         2,
    M_COMBINATION_KEYS:                         7,
    M_EVENT_VALUE:                              8,
    M_MOUSE_WHEEL_VALUE:                        8,
    M_KEY_VALUE:                                8,

    // application type
    M_WEB_APPLICATION:                          0x1,
    M_DEBUG:                                    0,

    M_RGB32:                                    0x900,

    FRAMES_PER_SECOND_DEFAULT:                  10,
    ENABLE_STAT:                                0,

    //////////////////////////////////////////////////////////////////////////
    // MIL Functions
    //////////////////////////////////////////////////////////////////////////
    MappOpenConnection: function (url, initFlag, controlFlag, userVar) {
        var filterByAppUrlFn = function (obj, url) {
            var ret = false;
            if (obj && obj.getType && obj.getType() == MilWeb.M_APPLICATION && obj.geturl() == url) {
                ret = true;
            }
            return ret;
        };
        var App = MilWeb.IdTable.filter(filterByAppUrlFn, url)[0];
        if (App == undefined)
            App = new MilApp(url, initFlag, controlFlag, userVar);

        MilWeb.updateUserVar(userVar, App.getId());
    },

    MappCloseConnection: function (appId) {
        var App = MilWeb.IdTable.get(appId);
        if (App) {
            if (App instanceof MilApp)
                App.terminate();
            else
                MilWeb.reportError(appId, 1);
        }
    },

    MappInquire: function (appId, inquireType, userVar) {
        var App = MilWeb.IdTable.get(appId);
        if (App instanceof MilApp) {
            var result = App.inquire(inquireType);
            MilWeb.updateUserVar(userVar, result);
            return result;
        } else {
            MilWeb.reportError(appId, 1);
        }
    },

    MappHookFunction: function (appId, hookType, handler, userParam) {
        var webObject = MilWeb.IdTable.get(appId);
        if (webObject instanceof MilApp) {
            webObject.hookFunction(hookType, handler, userParam);
        } else {
            MilWeb.reportError(appId, 1);
        }
    },

    MappGetHookInfo: function (appId, eventId, infoType, userVar) {
        if (eventId instanceof Array) {
            for (var i = 0; i < eventId.length; i++) {
                if (eventId[i].Type === infoType) {
                    var result = eventId[i].Value;
                    MilWeb.updateUserVar(userVar, result);
                    return result;
                }
            }
        } else {
            MilWeb.reportError(appId, 2);
        }
        return null;
    },

    MappInquireConnection: function (appId, inquireType, controlFlag, extraFlag, userVar) {
        var App = MilWeb.IdTable.get(appId);
        if (App instanceof MilApp) {
            var result = App.inquireConnection(inquireType, controlFlag, extraFlag);
            MilWeb.updateUserVar(userVar, result);
            return result;
        } else {
            MilWeb.reportError(appId, 1);
        }
        return MilWeb.M_NULL;
    },

    MappControl: function (appId, controlType, controlValue) {
        var App = MilWeb.IdTable.get(appId);
        if (App instanceof MilApp) {
            App.control(controlType, controlValue);
        } else {
            MilWeb.reportError(appId, 1);
        }
    },

    MdispInquire: function (dispId, inquireType, userVar) {
        var webObject = MilWeb.IdTable.get(dispId);
        if (webObject instanceof MilDisplay) {
            var result = webObject.inquire(inquireType);
            MilWeb.updateUserVar(userVar, result);
            return result;
        } else {
            MilWeb.reportError(dispId, 3);
        }
        return MilWeb.M_NULL;
    },

    MdispControl: function (dispId, controlType, controlValue) {
        var webObject = MilWeb.IdTable.get(dispId);
        if (webObject) {
            if (webObject instanceof MilDisplay) {
                webObject.control(controlType, controlValue);
            } else {
                MilWeb.reportError(dispId, 3);
            }
        }
    },

    MdispMessage: function (dispId, eventType, mousePositionX, mousePositionY, eventValue, combinationKeys, userVar) {
        var webObject = MilWeb.IdTable.get(dispId);
        if (webObject) {
            if (webObject instanceof MilDisplay) {
                webObject.RCSendDisplayMessage(webObject.getbufferName(),
                    eventType,
                    mousePositionX,
                    mousePositionY,
                    eventValue,
                    combinationKeys);
                var result = null;
                MilWeb.updateUserVar(userVar, result);
            } else {
                MilWeb.reportError(dispId, 3);
            }
        }
    },

    MdispZoom: function (dispId, xfactor, yfactor) {
        var webObject = MilWeb.IdTable.get(dispId);
        if (webObject) {
            if (webObject instanceof MilDisplay) {
                webObject.RCSendDisplayZoom(webObject.getbufferName(), xfactor, yfactor);
            } else {
                MilWeb.reportError(dispId, 3);
            }
        }
    },

    MdispPan: function (dispId, xoffset, yoffset) {
        var webObject = MilWeb.IdTable.get(dispId);
        if (webObject) {
            if (webObject instanceof MilDisplay) {
                webObject.RCSendDisplayPan(webObject.getbufferName(), xoffset, yoffset);
            } else {
                MilWeb.reportError(dispId, 3);
            }
        }
    },

    MdispSelectWindow: function (dispId, canvasElement) {
        var display = MilWeb.IdTable.get(dispId);
        MilWeb.assert(display instanceof MilDisplay, "Not the right type.");
        if (!(display instanceof MilDisplay)) {
            MilWeb.reportError(dispId, 3);
            return;
        }
        if (canvasElement && !(canvasElement instanceof Element)) {
            MilWeb.reportError(dispId, 4);
            return;
        }
        canvasElement ? display.selectCanvas(canvasElement) : display.deselectCanvas();
    },

    MdispHookFunction: function (dispId, hookType, handler, userParam) {
        var webObject = MilWeb.IdTable.get(dispId);
        if (webObject instanceof MilDisplay) {
            webObject.hookFunction(hookType, handler, userParam);
        }
        else {
            MilWeb.reportError(dispId, 3);
        }

    },

    MdispGetHookInfo: function (eventId, infoType, userVar) {
        if (eventId instanceof Array) {
            for (var i = 0; i < eventId.length; i++) {
                if (eventId[i].Type === infoType) {
                    var result = eventId[i].Value;
                    MilWeb.updateUserVar(userVar, result);
                    return result;
                }
            }
        } else {
            MilWeb.reportError(MilWeb.M_NULL, 2);
        }
        return MilWeb.M_NULL;
    },

    MbufInquire: function (bufId, inquireType, userVar) {
        var webObject = MilWeb.IdTable.get(bufId);
        if ((webObject instanceof MilArray) ||
            (webObject instanceof MilImage)) {
            var result = webObject.inquire(inquireType);
            MilWeb.updateUserVar(userVar, result);
            return result;
        } else {
            MilWeb.reportError(bufId, 5);
        }
        return MilWeb.M_NULL;
    },

    MbufGetHookInfo: function (eventId, infoType, userVar) {
        if (eventId instanceof Array) {
            for (var i = 0; i < eventId.length; i++) {
                if (eventId[i].Type === infoType) {
                    var result = eventId[i].Value;
                    MilWeb.updateUserVar(userVar, result);
                    return result;
                }
            }
        } else {
            MilWeb.reportError(MilWeb.M_NULL, 2);
        }
        return null;
    },

    MbufGet: function (bufId, userVar) {
        var webBuffer = MilWeb.IdTable.get(bufId);
        if (webBuffer instanceof MilWebObject) {
            var result = webBuffer.getData();
            MilWeb.updateUserVar(userVar, result);
        } else {
            MilWeb.reportError(bufId, 6);
        }
    },

    MobjHookFunction: function (objId, hookType, handler, userParam) {
        var webObject = MilWeb.IdTable.get(objId);
        if (webObject instanceof MilWebObject) {
            webObject.hookFunction(hookType, handler, userParam);
        } else {
            MilWeb.reportError(objId, 6);
        }
    },

    MobjGetHookInfo: function (eventId, infoType, userVar) {
        if (eventId instanceof Array) {
            for (var i = 0; i < eventId.length; i++) {
                if (eventId[i].Type === infoType) {
                    var result = eventId[i].Value;
                    MilWeb.updateUserVar(userVar, result);
                    return result;
                }
            }
        } else {
            MilWeb.reportError(MilWeb.M_NULL, 2);
        }
        return null;
    },

    MobjControl: function (objId, controlType, controlValue) {
        var webObject = MilWeb.IdTable.get(objId);
        if (webObject) {
            webObject.control(controlType, controlValue);
        } else {
            MilWeb.reportError(objId, 6);
        }
    },

    MobjInquire: function (webObjectId, inquireType, userVar) {
        var webObject = MilWeb.IdTable.get(webObjectId);
        if (webObject) {
            var result = webObject.inquire(inquireType);
            MilWeb.updateUserVar(userVar, result);
            return result;
        } else {
            MilWeb.reportError(webObjectId, 6);
        }
        return MilWeb.M_NULL;
    },

    MobjMessageRead: function (msgId, msgPtr, msgInSize, msgOutSize, msgTag, msgStatus, operationFlag) {
        var message = MilWeb.IdTable.get(msgId);
        var msgLength = 0;
        if (message) {
            if ((message instanceof MilMessage)) {
                MilWeb.updateUserVar(msgPtr, message.getData());
                var msgdata = message.getData();
                if (msgdata) {
                    if (message.getMessageType() != MilWeb.M_MAILBOX_MODE_WEBTEXT) {
                        MilWeb.assert(msgdata instanceof ArrayBuffer, " must be an ArrayBuffer.");
                        msgLength = msgdata.byteLength;
                    } else {
                        MilWeb.assert(typeof msgdata == "string", " must be a string.");
                        msgLength = msgdata.length;
                    }

                    MilWeb.updateUserVar(msgOutSize, msgLength);
                    MilWeb.updateUserVar(msgTag, message.getTag());
                    MilWeb.updateUserVar(msgStatus, MilWeb.M_SUCCESS);
                }
            } else {
                MilWeb.reportError(msgId, 7);
            }
        }
        return msgLength;
    },
    MobjMessageWrite: function (msgId, msgData, msgLength, msgTag, operationFlag) {
        var message = MilWeb.IdTable.get(msgId);
        if ((message instanceof MilMessage)) {
            if (message.getAccessType() != MilWeb.M_READ_WRITE) {
                MilWeb.reportError(msgId, 8);
                return;
            }
            if (msgData instanceof ArrayBuffer) {
                message.RCSendMessage(message.getbufferName(), msgData, msgLength, msgTag, operationFlag);
            } else if (typeof msgData == "string") {
                message.RCSendMessage(message.getbufferName(), msgData, msgLength, msgTag, operationFlag);
            } else {
                MilWeb.reportError(msgId, 9);
                return;
            }
        } else {
            MilWeb.reportError(msgId, 7);
            return;
        }
    },

    //////////////////////////////////////////////////////////////////////////
    // MIL Utilities
    //////////////////////////////////////////////////////////////////////////
    Utility: (function () {
        var IMAGE_FORMATS = {
            // Image Format
            M_MONO1: 0x100,
            M_MONO8: 0x200,
            M_MONO16: 0x300,
            M_MONO32: 0x400,
            M_MONO10: 0x500,
            M_MONO64: 0x2900,
            M_MONO12: 0x2a00,
            M_RGB15: 0x600,
            M_RGB16: 0x700,
            M_RGB24: 0x800,
            M_RGBX32: 0x900,
            M_RGB32: 0x900,
            M_XRGB32: 0xa00,
            M_RGB48: 0xb00,
            M_RGB96: 0xc00,
            M_RGB3: 0xd00,
            M_BGR15: 0xe00,
            M_BGR16: 0xf00,
            M_BGR24: 0x1000,
            M_BGRX32: 0x1100,
            M_BGR32: 0x1100,
            M_XBGR32: 0x1200,
            M_BGR30: 0x1300,
            M_RGB192: 0x1400,
            M_YUV9: 0x1500,
            M_YUV1611: 0x1500,
            M_YUV12: 0x1600,
            M_YUV411: 0x1600,
            M_YUV16: 0x1700,
            M_YUV422: 0x1700,
            M_YUV24: 0x1b00,
            M_YUV444: 0x1b00,
            M_YUV16_YUYV: 0x1c00,
            M_YUV422_YUYV: 0x1c00,
            M_YUV16_UYVY: 0x1d00,
            M_YUV12_1394: 0x1f00,
            M_YUV411_1394: 0x1f00,
            M_YUV32: 0x2000
        };

        function drawRGB32(imageData, imageSrc) {
            var src = imageSrc;
            var data = imageData.data;
            for (var i = 0; i < src.length; i += 4) {
                data[i] = src[i];     // R
                data[i + 1] = src[i + 1]; // G
                data[i + 2] = src[i + 2]; // B
                data[i + 3] = 255;      // A
            }
        };
        function drawBGR32(imageData, imageSrc) {
            var src = imageSrc;
            var data = imageData.data;
            for (var i = 0; i < src.length; i += 4) {
                data[i] = src[i + 2];     // R
                data[i + 1] = src[i + 1];     // G
                data[i + 2] = src[i];       // B
                data[i + 3] = 255;          // A
            }
        };
        function drawRGB24(imageData, imageSrc) {
            var src = imageSrc;
            var data = imageData.data;
            for (var i = 0, j = 0; i < data.length, j < src.length; i += 4, j += 3) {
                data[i] = src[j];   // R
                data[i + 1] = src[j + 1]; // G
                data[i + 2] = src[j + 2]; // B
                data[i + 3] = 255;      // A
            }
        };

        function drawMono8(imageData, imageSrc) {
            var src = imageSrc;
            var buf = new ArrayBuffer(imageData.data.length);
            var buf8 = new Uint8ClampedArray(buf);
            var buf32 = new Uint32Array(buf);
            var value;
            for (var j = 0; j < src.length; j++) {
                value = (src[j] & 0xff);
                buf32[j] = (value |   // R
                    (value << 8) |   // G
                    (value << 16) |   // B
                    (255 << 24));   // A
            }
            imageData.data.set(buf8);
        };

        function drawUnsupported(imageData, imageSrc) {
            var buf = new ArrayBuffer(imageData.data.length);
            var buf8 = new Uint8ClampedArray(buf);
            var buf32 = new Uint32Array(buf);
            for (var j = 0; j < buf32.length; j++) {
                buf32[j] = 0xff0000ff;
            }
            imageData.data.set(buf8);
        };
        return {
            Draw: function (imageData, imageSrc, imageFormat) {
                switch (imageFormat) {
                    case IMAGE_FORMATS.M_MONO8:
                        drawMono8(imageData, imageSrc);
                        break;
                    case IMAGE_FORMATS.M_RGB32:
                        drawRGB32(imageData, imageSrc);
                        break;
                    case IMAGE_FORMATS.M_BGR32:
                        drawBGR32(imageData, imageSrc);
                        break;
                    case IMAGE_FORMATS.M_RGB24:
                        drawRGB24(imageData, imageSrc);
                        break;
                    default:
                        drawUnsupported(imageData, imageSrc);
                        break;
                }
            }
        };
    })(),
    extend: function (base, sub) {
        var subProto = sub.prototype;
        sub.prototype = new base();
        for (var key in subProto) {
            sub.prototype[key] = subProto[key];
        }
        sub.prototype.constructor = sub;
    },

    assert: function (value, desc) {
        if (MilWeb.M_DEBUG === 1 && (value === false || value == null || value == undefined)) {
            alert(desc);
        }
    },
    log: function (message) {
        if (MilWeb.M_DEBUG === 1)
            try {
                console.log("[" + new Date().toLocaleString() + "] " + message);
            } catch (e) {
                alert(message);
            }
    },
    convertUTF8ToString: function (data) {
        var str = "";
        if (data) {
            try {
                var Uint8View = new Uint8Array(data);
                if (typeof(TextEncoder)!='undefined') {
                    str = new TextDecoder("utf-8").decode(Uint8View);
                } else {
                    var encodedString = String.fromCharCode.apply(null, Uint8View);
                    str = decodeURIComponent(escape(encodedString));
                }
            } catch (e) {
                MilWeb.assert(false, "Convert UTF8 To String failed.");
            }
        }
        return str;
    },
    convertUTF16ToString: function (data) {
        var str = "";
        if (data) {
            var bytes = new Uint8Array(data);
            var ix = 0;
            var offset1 = 1, offset2 = 0;
            for (; ix < bytes.length; ix += 2) {
                var byte1 = bytes[ix + offset1];
                var byte2 = bytes[ix + offset2];
                var word1 = (byte1 << 8) + byte2;
                if (byte1 < 0xD8 || byte1 >= 0xE0) {
                    str += String.fromCharCode(word1);
                } else {
                    ix += 2;
                    var byte3 = bytes[ix + offset1];
                    var byte4 = bytes[ix + offset2];
                    var word2 = (byte3 << 8) + byte4;
                    str += String.fromCharCode(word1, word2);
                }
            }
        }
        return str;
    },
    convertFromUTF16toArray: function (str) {
        var bytes = [];
        for (var i = 0; i < str.length; ++i) {
            bytes.push(str.charCodeAt(i));
        }
        return bytes;
    },
    convertFromUTF8ToArray: function (str) {
        var utf8 = unescape(encodeURIComponent(str));
        var bytes = [];
        for (var i = 0; i < utf8.length; i++) {
            bytes.push(utf8.charCodeAt(i));
        }
        return bytes;
    },
    updateUserVar: function (userVar, result) {
        if (userVar && (typeof userVar == 'object'))
            userVar.data = result;
    },
    errorCounter: (function () {
        var id = 0;
        return {
            get: function () {
                return id;
            },
            increment: function () {
                return id++;
            },
            decrement: function () {
                return id--;
            }
        };
    })(),

    reportError: function (objId, value) {
        var M_ERRORS = [
            "",
            "Invalid parameter, Must be a MIL Application context.",
            "Invalid parameter, Invalid object type for EventId.",
            "Invalid parameter, Must be a MIL Display.",
            "Invalid parameter, Must be DOM HTML5 canvas element.",
            "Invalid parameter, Must be a MIL buffer.",
            "Invalid parameter, Must be a MIL object.",
            "Invalid parameter, Must be a MIL Message.",
            "Error writing to read only message.",
            "Message data must be an ArrayBuffer.",
            "Unsupported control type.",
            "Unsupported inquire type.",
            "Invalid control type value.",
            "Invalid hook type for group.",
            "Unsupported control value.",
            "Not connected yet"
        ];
        var App = null;
        var webObject = MilWeb.IdTable.get(objId);
        MilWeb.errorCounter.increment();
        if (MilWeb.errorCounter.get() == 1) {
            if (webObject) {
                if (webObject instanceof MilApp) {
                    App = webObject;
                } else {
                    App = MilWeb.IdTable.get(webObject.getAppId());
                }
            }

            if (!App) {
                //get first application    
                var filterAllAppFn = function (obj, unused) {
                    var ret = false;
                    if (obj &&
                        obj.getType &&
                        obj.getType() == MilWeb.M_APPLICATION) {
                        ret = true;
                    }
                    return ret;
                };
                var unusedparam;
                var webList = MilWeb.IdTable.filter(filterAllAppFn, unusedparam);
                MilWeb.assert(webList.length > 0, "No Application did you connect ?");
                if (webList && webList.length > 0)
                    App = webList[0];
            }
            MilWeb.assert(App, "No Application did you connect ?");
            var msg = "";
            var msgId = 0;
            if (typeof value == "number") {
                if (value > 0 && value < M_ERRORS.length) {
                    msg = M_ERRORS[value];
                    msgId = value;
                }
            } else {
                msg = value;
            }

            var eventHook = [
                {
                    Type: MilWeb.M_OBJECT_ID,
                    Value: objId

                },
                {
                    Type: MilWeb.M_CURRENT,
                    Value: 1

                },
                {
                    Type: MilWeb.M_CURRENT_SUB_1,
                    Value: msgId

                },
                {
                    Type: MilWeb.M_CURRENT_SUB_NB,
                    Value: 1

                },
                {
                    Type: MilWeb.M_CURRENT + MilWeb.M_MESSAGE,
                    Value: "MilWeb Error"

                },
                {
                    Type: MilWeb.M_CURRENT_SUB_1 + MilWeb.M_MESSAGE,
                    Value: msg

                }
            ];

            if (App) {
                App.callhookFunction(MilWeb.M_ERROR, eventHook);
                if (!App.__PRINT_DISABLE_VALUE && msg) {
                    var e = new Error("MilWeb Error Message: " + msg);
                    console.error("MilWeb Error Message: " + msg);
                }
            } else {
                if (msg) {
                    var e = new Error("MilWeb Error Message: " + msg);
                    console.error("MilWeb Error Message: " + msg);
                }
            }
        }
        MilWeb.errorCounter.decrement();
    },

    IdTable: (function () {
        var M_START_OF_ID = 100;
        var table = {};
        var index = M_START_OF_ID; //start of id's
        var l = 0;
        function look(obj) {
            var id = MilWeb.M_INVALID;
            if (obj) {
                for (var i = M_START_OF_ID; i < index; i++) {
                    if (table[i] === obj)
                        id = i;
                }
            } else
                id = MilWeb.M_NULL;
            return id;
        }

        return {
            get: function (id) {
                if (!isNaN(id)) {
                    MilWeb.assert((id >= M_START_OF_ID && id < index), "invalid MIL ID value");
                    return table[id];
                }
                return null;
            },
            put: function (obj) {
                var id = look(obj);
                if (id == MilWeb.M_INVALID) {
                    l++;
                    id = index++;
                    table[id] = obj;
                    MilWeb.assert(obj.setId, "The Object must support the setId function");
                    if (obj.setId)
                        obj.setId(id);
                }
                return id;
            },
            remove: function (id) {
                if (!isNaN(id)) {
                    MilWeb.assert((id >= M_START_OF_ID && id < index), "invalid MIL ID value");
                    if (table[id]) {
                        MilWeb.log("remove id " + id);
                        delete table[id];
                        l--;
                    }
                    MilWeb.assert(l >= 0, "length must be greater or equal to zero");
                }
            },
            filter: function (fn, param1, param2) {
                MilWeb.assert(typeof fn == 'function', "Should be a function");
                var list = [];
                for (var i = M_START_OF_ID; i < index; i++) {
                    var obj = table[i];
                    if (fn && fn(obj, param1, param2))
                        list.push(obj);
                }
                return list;
            },
            size: function () {
                MilWeb.log("length is " + l);
                return l;
            }
        };
    })()
};


function MilWebObject(url, bufferName, groupName, clientName) {
    this.__url = url;
    this.__bufferName = bufferName;
    this.__groupName = groupName;
    this.__clientName = clientName;
    this.__socket = null;
    this.__isconnected = false;
    this.__isconnecting = false;
    this.__isinitialized = false;
    this.__issubscribed = false;
    this.__isenabled = true;
    this.__isinteractive = true;
    this.__type = MilWeb.M_INVALID;
    this.__datatype = MilWeb.M_INVALID;
    this.__sizex = MilWeb.M_INVALID;
    this.__sizey = MilWeb.M_INVALID;
    this.__format = MilWeb.M_INVALID;;
    this.__accesstype = MilWeb.M_INVALID;
    this.__sizebyte = MilWeb.M_INVALID;
    this.__bufferchanged = false;
    this.__updateInterval = MilWeb.TIMEOUT;
    this.__messagetag = MilWeb.M_INVALID;
    this.__messagetype = MilWeb.M_INVALID;
    this.__serialcounter = MilWeb.M_INVALID;
    this.__groupcounter = MilWeb.M_INVALID;
    this.__lastserialcounter = 0;
    this.__data = null;
    this.__textmessage = "";
    this.__frameMin = 100;
    this.__frameMax = -100;
    this.__enableDisplayTime = false;
    this.__id = MilWeb.M_NULL;
    this.__appid = MilWeb.M_NULL;
    this.__groupid = MilWeb.M_NULL;
    this.__clientId = MilWeb.M_INVALID;
    this.__clientGroupId = MilWeb.M_INVALID;
    this.__framesPerSecond = 0;
    this.__requestid = 0;
    this.__timeDiv = document.createElement("div");
    this.__serverDataAvailable = true;
    this.__closeFromUser = false;
    this.__lastCommmand = null;
    this.__freed = false;
    if (this.webSocketSupported()) {
        this.__supported = true;
    } else {
        this.__supported = false;
        alert("websocket not supported");
    }


    // Hooks
    this.__HookList = [];

    // Protocol
    this.Protocol = {
        CLIENT_APP_START: 101,
        CLIENT_APP_END: 102,
        CLIENT_SET_PARAMETERS: 103,
        CLIENT_OBJECT_CONNECT: 104,
        CLIENT_OBJECT_DISCONNECT: 105,
        CLIENT_REQUEST_OBJECT_DATA: 125,
        CLIENT_REQUEST_OBJECT_INFO: 126,
        CLIENT_SEND_MESSAGE: 128,
        CLIENT_GET_OBJECT_LIST: 129,
        CLIENT_FORCE_UPDATE_IN_PROGRESS: 130,
        CLIENT_SEND_DISPLAY_MESSAGE: 135,
        CLIENT_SET_DISPLAY_INTERACTIVE: 136,
        CLIENT_SEND_DISPLAY_ZOOM: 137,
        CLIENT_SEND_DISPLAY_PAN: 138,

        SERVER_OBJECT_DATA: 221,
        SERVER_OBJECT_INFO: 222,
        SERVER_CONNECTION_INFO: 223,
        SERVER_OBJECT_LIST: 224,
        SERVER_OBJECT_CONNECTED: 225,
        SERVER_OBJECT_DISCONNECTED: 226,
        SERVER_OBJECT_REFRESH_LIST: 236,
        SERVER_OBJECT_REMOVE: 237,
        SERVER_OBJECT_ADD: 238,
        SERVER_OBJECT_FREE: 239,
        SERVER_DISPLAY_INTERACTIVE_STATE: 240
    };
}

//////////////////////////////////////////////////////////////////////////
// MIL WEB Objects
//////////////////////////////////////////////////////////////////////////
MilWebObject.prototype = {
    connected: function () {
        return this.__isconnected;
    },

    enabled: function () {
        return this.__isenabled;
    },

    initialized: function () {
        return this.__isinitialized;
    },

    subscribed: function () {
        return this.__issubscribed;
    },

    interactive: function () {
        return this.__isinteractive;
    },
    geturl: function () {
        return this.__url;
    },

    getbufferName: function () {
        return this.__bufferName;
    },

    getgroupName: function () {
        return this.__groupName;
    },

    setgroupName: function (grpName) {
        this.__groupName = grpName;
    },

    getWebSocket: function () {
        return this.__socket;
    },

    getData: function () {
        return this.__data;
    },

    getType: function () {
        return this.__type;
    },

    getAccessType: function () {
        return this.__accesstype;
    },

    isBufferChanged: function () {
        return this.__bufferchanged;
    },

    getClientId: function () {
        return this.__clientId;
    },

    setClientId: function (ClientId) {
        this.__clientId = ClientId;
    },

    setClientGroupId: function (clientGroupId) {
        this.__clientGroupId = clientGroupId;
    },

    setframesPerSecond: function (FPS) {
        this.__framesPerSecond = FPS;
    },

    getframesPerSecond: function () {
        return this.__framesPerSecond;
    },

    serialCounter: function () {
        return this.__serialcounter;
    },

    serialCounterDiff: function () {
        return this.__serialcounter - this.__lastserialcounter;
    },

    setId: function (milid) {
        this.__id = milid;
    },

    getId: function () {
        return this.__id;
    },

    setAppId: function (milid) {
        this.__appid = milid;
    },

    getAppId: function () {
        return this.__appid;
    },

    getGroupId: function () {
        return this.__groupid;
    },

    setGroupId: function (milid) {
        this.__groupid = milid;
    },

    getHookInfo: function () {
        var ret =
            [
                {
                    Type: MilWeb.M_OBJECT_ID,
                    Value: this.getId()

                }
            ];
        return ret;
    },

    getTag: function () {
        return this.__messagetag;
    },

    getMessageType: function () {
        return this.__messagetype;
    },

    isFreed: function () {
        return this.__freed;
    },

    setCloseFromUser: function (val) {
        this.__closeFromUser = val;
    },

    getCloseFromUser: function () {
        return this.__closeFromUser;
    },

    toString: function () {
        return "Buffer : " + this.__bufferName + "ID : " + this.milid;
    },

    closeSocket: function () {
        if (this.__socket) {
            this.__socket.close();
            this.__socket = null;
        }
    },

    webSocketSupported: function () {
        if ("WebSocket" in window) {
            return true;
        }
        else {
            return false;
        }
    },

    displayTime: function (elementId, text) {
        if (this.__enableDisplayTime) {
            var newElement = document.getElementById(elementId + "_time");
            if (!newElement) {
                this.__timeDiv.innerHTML = (this.getGroupId() == MilWeb.M_NULL) ? "<h3>Time(" + this.__bufferName + ")</h3>" : "<h3>Time(" + this.__bufferName + "[" + this.__groupName + "] )</h3>";
                document.body.appendChild(this.__timeDiv);
                newElement = document.createElement("pre");
                newElement.id = elementId + "_time";
                this.__timeDiv.appendChild(newElement);
            }
            newElement.innerHTML = text;
        }
    },

    hookFunction: function (hookTypeParam, HookHandler, userParam) {
        MilWeb.log("Hook/Unhook on " + hookTypeParam);
        var id = this.getId();
        MilWeb.assert((typeof HookHandler == "function"), "Hook Handler is not a function");
        MilWeb.assert((typeof hookTypeParam == "number"), "Hook Type is not a number");
        if (typeof HookHandler == "function" && typeof hookTypeParam == "number") {
            var objHook = { Type: hookTypeParam, Handler: HookHandler, UserData: userParam };
            var index = -1;

            // UnHOOK
            var result = hookTypeParam & MilWeb.M_UNHOOK;
            if (result === MilWeb.M_UNHOOK) {
                objHook = { Type: (hookTypeParam - MilWeb.M_UNHOOK), Handler: HookHandler, UserData: userParam };
                MilWeb.assert(HookHandler.Hooked == true, "Must be hooked before");
                if (hookTypeParam === (MilWeb.M_UPDATE_WEB + MilWeb.M_UNHOOK)) {
                    // Reinit state to allow retrieving new data
                    this.__dataAvailable = true;
                }
                // Remove the hook
                index = this.indexOfHookList(objHook);
                MilWeb.assert(index != -1, "Must be in the hook list");
                if (index != -1)
                    this.__HookList.splice(index, 1);
                delete HookHandler.Hooked;

                // HOOK
            } else {
                index = this.indexOfHookList(objHook);
                if (index == -1) {
                    this.__HookList.push(objHook);
                    HookHandler.Hooked = true;
                }
                if (this.__isinitialized && (hookTypeParam === MilWeb.M_UPDATE_WEB) || (hookTypeParam === MilWeb.M_UPDATE_END)) {
                    var currentThis = this;
                    setTimeout(function () { currentThis.onUpdate(); }, 5);
                }
            }
        }
    },

    callhookFunction: function (hookTypeParam, hookEventParam) {
        for (var i = 0; i < this.__HookList.length; i++) {
            var objHook = this.__HookList[i];
            if (objHook.Type == hookTypeParam) {
                objHook.Handler.call(objHook.Handler, hookTypeParam, hookEventParam, objHook.UserData);
            }
        }
    },

    indexOfHookList: function (objHookParam) {
        var index = -1;
        for (var i = 0; i < this.__HookList.length; i++) {
            var objHook = this.__HookList[i];
            if (objHook.Type == objHookParam.Type && objHook.Handler == objHookParam.Handler) {
                index = i;
                break;
            }
        }
        return index;
    },

    isHooked: function (hookTypeParam) {
        for (var i = 0; i < this.__HookList.length; i++) {
            var objHook = this.__HookList[i];
            if (objHook.Type == hookTypeParam) {
                return true;
            }
        }
        return false;
    },

    isHookedOnUpdate: function () {
        return this.isHooked(MilWeb.M_UPDATE_WEB);
    },

    processMessage: function (msg) {
        if (msg.data instanceof ArrayBuffer) {
            this.__data = msg.data;
            this.onData(this.__data);
        }
        else {
            try {
                if (this.__messagetype == MilWeb.M_MAILBOX_MODE_WEBTEXT &&
                    this.__lastCommand == this.Protocol.SERVER_OBJECT_DATA &&
                    this.__sizebyte > 0) {
                    // This is message data in text format
                    this.__data = msg.data
                    this.onData(this.__data);
                    this.__lastCommand = null;
                } else {
                    this.__textmessage = msg.data;
                    var message = JSON.parse(this.__textmessage);
                    this.onCommand(message);
                }
            } catch (e) {
                MilWeb.assert(false, e.message);
            }

        }

    },

    subscribe: function () {
        if (this.__isinitialized && this.__isconnected && this.__type != MilWeb.M_GROUP) {
            this.RCSetExchangeBuffer(this.__bufferName, this.__clientGroupId, this.__clientName, true);
            this.__issubscribed = true;
        }
    },

    connect: function () {
        if (this.__supported && !this.__isconnected) {
            this.__isconnecting = true;
            var errorconnect = {};
            var currentThis = this;
            try {
                this.__socket = new WebSocket(this.__url);
                this.__socket.binaryType = "arraybuffer";
                this.__socket.onopen = function ws_onopen() {
                    currentThis.onConnect();
                };

                this.__socket.onclose = function ws_onclose(msg) {
                    currentThis.onDisconnect(msg);
                };

                this.__socket.onmessage = function ws_onmessage(msg) {
                    if (currentThis.__socket) {
                        currentThis.processMessage(msg);
                    }
                };
                this.__socket.onerror = function ws_onerror(msg) {
                    if (currentThis.getId())
                        MilWeb.reportError(currentThis.getId(), "websocket error");
                };
            } catch (e) {
                MilWeb.reportError(currentThis.getId(), e.message);
            }
            return true;
        }
        else {
            return false;
        }
    },

    RCClientInit: function (version) {
        if (this.webSocketSupported() && this.__socket) {
            var msg = {
                Command: this.Protocol.CLIENT_APP_START,
                ClientVersion: version,
                ApplicationType: MilWeb.M_WEB
            };
            try {
                this.__socket.send(JSON.stringify(msg));
            } catch (e) {
                MilWeb.log(e);
            }
        }

        if (this.webSocketSupported() && !this.__isconnected) {
            this.onError(15);
        }
    },

    RCClientTerminate: function () {
        if (this.webSocketSupported() && this.__socket && this.__isconnected) {
            var msg = {
                Command: this.Protocol.CLIENT_APP_END
            };
            try {
                this.__socket.send(JSON.stringify(msg));
            } catch (e) {
                MilWeb.log(e);
            }
        }

    },

    RCGetBufferList: function () {
        if (this.webSocketSupported() && this.__socket) {
            var msg = {
                Command: this.Protocol.CLIENT_GET_OBJECT_LIST
            };
            try {
                this.__socket.send(JSON.stringify(msg));
            } catch (e) {
                MilWeb.log(e);
            }
        }

        if (this.webSocketSupported() && !this.__isconnected) {
            this.onError(15);
        }
    },

    RCRequestBufferInfo: function (bufferName) {
        if (this.webSocketSupported() && this.__socket && bufferName) {
            var msg = {
                Command: this.Protocol.CLIENT_REQUEST_OBJECT_INFO,
                ExchangeBufferId: bufferName
            };
            try {
                this.__socket.send(JSON.stringify(msg));
            } catch (e) {
                MilWeb.log(e);
            }
        }

        if (this.webSocketSupported() && !this.__isconnected) {
            this.onError(15);
        }
    },

    RCForceUpdateInProgress: function () {
        if (this.webSocketSupported() && this.__socket) {
            var msg = {
                Command: this.Protocol.CLIENT_FORCE_UPDATE_IN_PROGRESS,
                ExchangeBufferId: bufferName
            };
            try {
                this.__socket.send(JSON.stringify(msg));
            } catch (e) {
                MilWeb.log(e);
            }
        }

        if (this.webSocketSupported() && !this.__isconnected) {
            this.onError(15);
        }
    },

    RCSetExchangeBuffer: function (bufferName, clientGroupId, clientName, bwaitForUpdate) {
        if (this.webSocketSupported() && this.__socket && bufferName) {
            MilWeb.assert(clientGroupId != MilWeb.M_INVALID, "Invalid Client GroupId");
            var msg = {
                Command: this.Protocol.CLIENT_SET_PARAMETERS,
                ExchangeBufferId: bufferName,
                WaitForUpdate: bwaitForUpdate,
                ClientName: clientName,
                ClientGroupId: clientGroupId,
                ApplicationType: MilWeb.M_WEB
            };
            try {
                this.__socket.send(JSON.stringify(msg));
            } catch (e) {
                MilWeb.log(e);
            }
        }

        if (this.webSocketSupported() && !this.__isconnected) {
            this.onError(15);
        }
    },

    RCGetExchangeBufferData: function (bufferName, forceUpdate, requestid) {
        if (this.webSocketSupported() && this.__socket && bufferName) {
            var msg = {
                Command: this.Protocol.CLIENT_REQUEST_OBJECT_DATA,
                ExchangeBufferId: bufferName,
                ForceTransfer: forceUpdate,
                GetAsync: false,
                RequestId: requestid
            };
            try {
                if (this.__isconnected) {
                    this.__socket.send(JSON.stringify(msg));
                }
            } catch (e) {
                MilWeb.assert(0, e);
            }
        }

        if (this.webSocketSupported() && !this.__isconnected) {
            this.onError(15);
        }
    },

    RCSendDisplayZoom: function (bufferName, xfactor, yfactor) {
        if (this.webSocketSupported() && this.__socket && this.interactive()) {
            var msg = {
                Command: this.Protocol.CLIENT_SEND_DISPLAY_ZOOM,
                ExchangeBufferId: bufferName,
                DisplayData: {
                    XFactor: xfactor,
                    YFactor: yfactor
                }
            };
            try {
                MilWeb.log(msg);
                this.__socket.send(JSON.stringify(msg));
            } catch (e) {
                MilWeb.log(e);
            }
        }

        if (this.webSocketSupported() && !this.__isconnected) {
            this.onError(15);
        }
    },

    RCSendDisplayPan: function (bufferName, xoffset, yoffset) {
        if (this.webSocketSupported() && this.__socket && this.interactive()) {
            var msg = {
                Command: this.Protocol.CLIENT_SEND_DISPLAY_PAN,
                ExchangeBufferId: bufferName,
                DisplayData: {
                    XOffset: xoffset,
                    YOffset: yoffset
                }
            };
            try {
                MilWeb.log(msg);
                this.__socket.send(JSON.stringify(msg));
            } catch (e) {
                MilWeb.log(e);
            }
        }

        if (this.webSocketSupported() && !this.__isconnected) {
            this.onError(15);
        }
    },

    RCSendDisplayMessage: function (bufferName, eventtype, mousepositionx, mousepositiony, eventvalue, combinationkeys) {
        if (this.webSocketSupported() && this.__socket && this.interactive()) {
            var msg = {
                Command: this.Protocol.CLIENT_SEND_DISPLAY_MESSAGE,
                ExchangeBufferId: bufferName,
                DisplayData: {
                    EventType: eventtype,
                    MousePositionX: mousepositionx,
                    MousePositionY: mousepositiony,
                    EventValue: eventvalue,
                    CombinationKeys: combinationkeys
                }
            };
            try {
                //MilWeb.log(msg);
                this.__socket.send(JSON.stringify(msg));
            } catch (e) {
                MilWeb.log(e);
            }
        }

        if (this.webSocketSupported() && !this.__isconnected) {
            this.onError(15);
        }
    },

    RCSendMessage: function (bufferName, msgData, msgLength, msgTag, msgFlag) {
        if (this.webSocketSupported() && this.__socket) {
            try {
                if (msgData instanceof ArrayBuffer) {
                    var data = msgData.slice(0, msgLength);
                    var msg = {
                        Command: this.Protocol.CLIENT_SEND_MESSAGE,
                        ExchangeBufferId: bufferName,
                        MessageTag: msgTag,
                        OperationFlag: msgFlag,
                        MessageBinary: true
                    };
                    this.__socket.send(JSON.stringify(msg));
                    this.__socket.send(data);
                } else if (typeof msgData == "string") {
                    var data = msgData.slice(0, msgLength);
                    var msg = {
                        Command: this.Protocol.CLIENT_SEND_MESSAGE,
                        ExchangeBufferId: bufferName,
                        MessageTag: msgTag,
                        MessageData: data,
                        OperationFlag: msgFlag,
                        MessageBinary: false
                    };
                    this.__socket.send(JSON.stringify(msg));
                } else {
                    MilWeb.assert(false, "must be an arraybuffer or string type  message");
                }
            } catch (e) {
                MilWeb.log(e);
            }
        }

        if (this.webSocketSupported() && !this.__isconnected) {
            this.onError(15);
        }
    },
    RCSetInteractiveDisplay: function (bufferName, controlValue) {
        if (this.webSocketSupported() && this.__socket && bufferName) {
            var msg = {
                Command: this.Protocol.CLIENT_SET_DISPLAY_INTERACTIVE,
                ExchangeBufferId: bufferName,
                DisplayInteractive: controlValue
            };
            try {
                this.__socket.send(JSON.stringify(msg));
            } catch (e) {
                MilWeb.log(e);
            }
        }

        if (this.webSocketSupported() && !this.__isconnected) {
            this.onError(15);
        }
    },

    // Abstract
    doJob: function () {
    },

    terminate: function () {
        if (this.__socket) {
            MilWeb.log("close socket for " + this.__bufferName);
        }
        this.closeSocket();
        if (!this.__closeFromUser) {
            MilWeb.IdTable.remove(this.getId());
        }
    },

    callOnUpdate: function () {
        MilWeb.log("CallOnUpdate " + this.__bufferName);
        if (this.getGroupId()) {
            var webGroup = MilWeb.IdTable.get(this.getGroupId());
            if (webGroup && !webGroup.isInGroup(this)) {
                // Make sure to rebuild the group
                this.__serverDataAvailable = true;              
                webGroup.addToGroup(this);
                var currentGroup = webGroup;
                setTimeout(function () { currentGroup.onUpdate(); }, 0);
            }
        } else {

            if (this.isHookedOnUpdate()) {
                var currentThis = this;
                setTimeout(function () { currentThis.onUpdate(); }, 5);
            }
        }
    },

    onConnect: function (fromUser) {
        MilWeb.log("Connect " + this.__bufferName);
        this.__isconnected = true;
        if (this.__bufferName) {
            this.RCRequestBufferInfo(this.__bufferName);
        }
    },

    onDisconnect: function (msg) {
        MilWeb.log("Disconnect " + this.__bufferName + "  from user " + this.__closeFromUser);
        if (this.__isconnected) {
            this.__groupcounter = MilWeb.M_INVALID;
            this.__isconnected = false;
            this.__serverDataAvailable = true;
            this.callhookFunction(MilWeb.M_DISCONNECT, this.getHookInfo());          
            if (!this.__closeFromUser) {
                if (!this.__freed) {
                    this.__freed = true;
                    // Fire Application M_OBJECT_PUBLISH_WEB hook in case
                    // the object get disconnected (like here) before receiving App refresh list
                    var App = MilWeb.IdTable.get(this.getAppId());
                    if (App)
                        App.callhookFunction(MilWeb.M_OBJECT_PUBLISH_WEB, this.getHookInfo());
                }

                MilWeb.IdTable.remove(this.getId());
                this.__id = MilWeb.M_NULL;
                this.__HookList = [];
            }
        }
        this.__isinitialized = false;
    },

    onError: function (msgId) {
        MilWeb.log("Error " + this.__bufferName + " " + msgId);
        MilWeb.reportError(this.getId(), msgId);
    },

    onFree: function (fromUser) {
        MilWeb.log("Free " + this.__bufferName);
        this.__closeFromUser = fromUser;
        if (this.getGroupId()) {
            var milWebGroupObject = MilWeb.IdTable.get(this.getGroupId());
            if (milWebGroupObject) {
                milWebGroupObject.removeFromGroup(this, fromUser);
            }
        }
        // remove the object
        this.terminate();
    },

    onData: function (data) {
        MilWeb.log("Get data " + this.__bufferName);
        this.__serverDataAvailable = true;
        var timeout = (this.getframesPerSecond() > 0) ? (1000 / this.getframesPerSecond()) : this.getframesPerSecond();
        if (this.initialized()) {
            if (this.getGroupId() == MilWeb.M_NULL) {
                if (this.isBufferChanged() && this.enabled())
                    this.doJob();
                // still hooked then ask for data
                if (this.isHookedOnUpdate()) {
                    var currentThis = this;
                    setTimeout(function () { currentThis.onUpdate(); }, timeout);
                }
            } else {
                var webGroup = MilWeb.IdTable.get(this.getGroupId());
                webGroup.updateData(this);
            }
        }
    },

    retrieveData: function () {
	MilWeb.log("Server Data " + this.__serverDataAvailable);
        if (!this.__serverDataAvailable)
            return;
        this.__serverDataAvailable = false;
        MilWeb.log("Ask for data " + this.__bufferName + " request " + this.__requestid);
        this.RCGetExchangeBufferData(this.__bufferName, false, this.__requestid++);
    },

    onUpdate: function (parameters) {
        MilWeb.log("onUpdate " + this.__bufferName);
        if (this.__isinitialized && this.__isconnected) {
            if (this.getGroupId() == MilWeb.M_NULL) {
                MilWeb.log("single object : ask data");
                this.retrieveData();
            }
            else {
                MilWeb.log("group member : ask data for group");
                var webGroup = MilWeb.IdTable.get(this.getGroupId());
                webGroup.onUpdate();
            }
        }
    },

    onCommand: function (message) {
        MilWeb.log("OnCommand " + message.Command + " " + this.__bufferName);
        var Command = message.Command;
        this.__lastCommand = Command;
        switch (Command) {
            case this.Protocol.SERVER_CONNECTION_INFO:
                break;
            case this.Protocol.SERVER_OBJECT_LIST:
                break;
            case this.Protocol.SERVER_OBJECT_INFO:
                break;
            case this.Protocol.SERVER_OBJECT_DATA:
                {
                    if (this.__isinitialized) {
                        try {
                            this.__sizebyte = message.BufferDataSize;
                            //MilWeb.assert(this.__sizebyte > 0, "Wrong message ");
                            if (message.BufferStruct) {
                                //MilWeb.log(message.BufferStruct);
                                MilWeb.assert(this.__bufferName === message.BufferStruct.Name, "Expect the same buffer name !!!");
                                MilWeb.assert(this.__type === message.BufferStruct.Type, "Expect the same buffer type !!!");
                                this.__bufferchanged = (message.BufferStruct.Changed == 1);
                                this.__lastserialcounter = this.__serialcounter;
                                this.__serialcounter = message.BufferStruct.SerialCounter;
                                this.__groupcounter = message.BufferStruct.GroupCounter;
                                this.__messagetag = message.BufferStruct.MessageTag;
                                this.__messagetype = message.BufferStruct.MessageType;
                                MilWeb.assert(this.__groupcounter < Number.MAX_VALUE, "Invalid group counter value");
                                MilWeb.assert(this.__serialcounter < Number.MAX_VALUE, "Invalid counter value");
                                this.__isinteractive = (message.BufferStruct.DisplayInteractive != 0);
                                // data is comming ...
                                this.__serverDataAvailable = false;
                            }
                        } catch (e) {
                            this.__isinitialized = false;
                        }
                    }
                }
                break;
            default:
                break;
        }
    },

    control: function (controlType, controlValue) {
        switch (controlType) {
            case MilWeb.M_FRAME_RATE:
                {
                    if (controlValue == MilWeb.M_DEFAULT)
                        this.setframesPerSecond(MilWeb.FRAMES_PER_SECOND_DEFAULT);
                    else if (controlValue >= 0)
                        this.setframesPerSecond(controlValue);
                    else
                        this.onError(14);
                }
                break;
            case MilWeb.M_UPDATE_WEB:
                {
                    if (controlValue == MilWeb.M_FORCE)
                        this.RCForceUpdateInProgress();
                }
                break;
            default:
                this.onError(10);
                break;
        }
    },
    inquire: function (inquireType) {
        switch (inquireType) {
            case MilWeb.M_SIZE_X:
                return this.__sizex;
                break;
            case MilWeb.M_SIZE_Y:
                return this.__sizey;
                break;
            case MilWeb.M_TYPE:
                return this.__datatype;
                break;
            case MilWeb.M_SIZE_BYTE:
                return this.__sizebyte;
                break;
            case MilWeb.M_OBJECT_NAME:
                return this.getbufferName();
                break;
            case MilWeb.M_OBJECT_TYPE:
                return this.getType();
                break;
            case MilWeb.M_WEB_PUBLISH:
                return this.getAccessType();
                break;
            case MilWeb.M_GROUP_ID:
                return this.getGroupId();
                break;
            case MilWeb.M_FRAME_RATE:
                return this.getframesPerSecond();
                break;
            default:
                this.onError(11);
                return MilWeb.M_NULL;
                break;
        };
        return MilWeb.M_NULL;
    }


};


//////////////////////////////////////////////////////////////////////////
// MIL Application
//////////////////////////////////////////////////////////////////////////

function MilApp(url, initFlag, controlFlag) {
    MilWebObject.call(this, url);
    this.__initFlag = initFlag;
    this.__controlFlag = controlFlag;
    this.__type = MilWeb.M_APPLICATION;
    this.__PRINT_DISABLE_VALUE = false;
    // Connect the Application
    this.connect(url);

    this.__clientVersion = MilWeb.CLIENT_VERSION;
    this.__serverVersion = MilWeb.M_INVALID;
    this.__numBufferList = MilWeb.M_INVALID;
    this.__bufferList = [];
    this.__enableStat = MilWeb.ENABLE_STAT;
    this.__serverProcessId = MilWeb.M_INVALID;
    this.__serverProcessName = "";
    this.__statDiv = document.createElement("div");
    this.__statDiv.innerHTML = "<h1>Stats</h1>";
    if (this.__enableStat) {
        document.body.appendChild(this.__statDiv);
    }
    this.filterByAppFn = function (obj, id) {
        var ret = false;
        if (obj && obj.getAppId && !isNaN(id) && obj.getAppId() == id) {
            ret = true;
        }
        return ret;
    };

    this.filterByGroupNameFn = function (obj, appId, groupName) {
        var ret = false;
        if (obj &&
            obj.getgroupName &&
            obj.getType &&
            obj.getgroupName() == groupName &&
            obj.getType() == MilWeb.M_GROUP &&
            obj.getAppId &&
            obj.getAppId() == appId) {

            ret = true;
        }
        return ret;
    };

    this.filterByOrphanGroupFn = function (obj, parameter) {
        var ret = false;
        if (obj &&
            obj.getgroupName &&
            obj.getType &&
            obj.getAppId &&
            obj.getAppId() == MilWeb.M_NULL &&
            obj.getType() == MilWeb.M_GROUP) {
            ret = true;
        }
        return ret;
    };

    this.filterByBufferNameFn = function (obj, appId, bufferName) {
        var ret = false;
        if (obj &&
            obj.getbufferName &&
            obj.getbufferName() == bufferName &&
            obj.getAppId &&
            obj.getAppId() == appId) {
            ret = true;
        }
        return ret;
    };

    var appid = MilWeb.IdTable.put(this);
    this.__bufferName = "App" + appid;
    window.addEventListener('beforeunload', this.beforeUnloadEvent.bind(this));

    if (this.__initFlag != MilWeb.M_DEFAULT)
        alert("Invalid Application init flag value");
}

MilApp.prototype = {
    setServerVersion: function (serverVersion) {
        this.serverVersion = serverVersion;
    },

    beforeUnloadEvent: function (evt) {
        if (this.__id) {
            this.callhookFunction(MilWeb.M_DISCONNECT, this.getHookInfo());
            this.terminate();
        }
        //evt.returnValue = "\o/";
    },

    onConnect: function () {
        this.__isconnected = true;
        // to be enabled when DPI is supported in MIL Display
        // var testDiv = document.createElement('div');
        // document.body.appendChild(testDiv);
        // testDiv.innerHTNL      = "";
        // testDiv.style.height   = "1in";
        // testDiv.style.left     = "-100%";
        // testDiv.style.position = "absolute";
        // testDiv.style.top      = "-100%";
        // testDiv.style.width    = "1in";
        // testDiv.id             = "testdiv";
        // document.body.appendChild(testDiv);
        // var devicePixelRatio = window.devicePixelRatio || 1;
        // var dpi_x = document.getElementById('testdiv').offsetWidth * devicePixelRatio;
        // var dpi_y = document.getElementById('testdiv').offsetHeight * devicePixelRatio;
        // document.body.removeChild(testDiv);
        this.callhookFunction(MilWeb.M_CONNECTING, this.getHookInfo());
        this.RCClientInit(this.__clientVersion);
    },

    onDisconnect: function (msg) {
        MilWeb.log("Disconnect " + this.__bufferName);
        if (this.__isconnected) {
            this.callhookFunction(MilWeb.M_DISCONNECT, this.getHookInfo());
            if (this.__socket) { // closed by server
                this.terminate();
            }
        } else {
            MilWeb.reportError(this.getId(), msg.reason.length ? msg.reason : "websocket error");
            this.terminate();
        }
        this.__HookList = [];
        this.__isconnected = false;
        
    },

    terminate: function () {
        MilWeb.log("terminate Application");
        var milwebObj = null;
        var webList = MilWeb.IdTable.filter(this.filterByAppFn, this.getId());
        // Real objects
        for (var i = 0; i < webList.length; i++) {
            milwebObj = webList[i];
            milwebObj.setCloseFromUser(false);
            milwebObj.terminate();
        }
        // Orphane groups
        webList = MilWeb.IdTable.filter(this.filterByOrphanGroupFn, this.getId());
        for (var k = 0; k < webList.length; k++) {
            milwebObj = webList[k];
            milwebObj.setCloseFromUser(false);
            milwebObj.terminate();
        }
        var currentThis = this;
        this.RCClientTerminate();
        MilWeb.IdTable.remove(this.getId());
        this.__id = MilWeb.M_NULL;
        setTimeout(function () { currentThis.closeSocket(); }, MilWeb.APP_CLOSE_TIMEOUT);
    },

    onCommand: function (message) {
        var Command = message.Command;
        switch (Command) {
            case this.Protocol.SERVER_CONNECTION_INFO:
                {
                    MilWeb.log("Connection Info");
                    this.__clientId = message.ClientId;
                    this.__serverVersion = message.ServerVersion;
                    this.__serverProcessId = message.ServerProcessId;
                    this.__serverProcessName = message.ServerProcessName;
                    MilWeb.log(message);
                    if (this.__serverVersion == this.__clientVersion) {
                        this.RCGetBufferList();
                        this.__isinitialized = true;
                    } else {
                        throw new TypeError('Server protocol version mismatch.');
                    }
                }
                break;
            case this.Protocol.SERVER_OBJECT_LIST:
                {
                    MilWeb.log("Object List [" + message.NumBufInList + "]");
                    this.__bufferList = message.BufferList;
                    this.__numBufferList = message.NumBufInList;
                    MilWeb.assert(this.__bufferList.length == this.__numBufferList, "Buffer list size mismatch");
                    this.__bufferList.forEach(function (element, index, array) {
                        var Buffer = element;
                        var bufferName = Buffer.ExchangeBufferId;
                        var groupName = Buffer.ExchangeGroupId;
                        var milWebObject = this.createObject(Buffer);

                        MilWeb.assert(milWebObject, "Invalid MIL web object");
                        milWebObject.setAppId(this.getId());
                        milWebObject.setClientGroupId(this.getClientId());

                        // this buffer is part of a group
                        if (groupName) {
                            // Create the group if needed.
                            var milWebGroupObject = this.createGroup(groupName);
                            // Add the object to the group
                            milWebGroupObject.addToGroup(milWebObject);
                        }
                    }, this);
                    this.callhookFunction(MilWeb.M_CONNECT, this.getHookInfo());
                }
                break;
            case this.Protocol.SERVER_OBJECT_REFRESH_LIST:
                {
                    MilWeb.log("object refresh list [" + message.NumBufInList + "]");
                    var inbufferList = function (objList, bufferName) {
                        var ret = false;
                        for (var i = 0; i < objList.length && !ret; i++) {
                            if (objList[i] && objList[i].ExchangeBufferId == bufferName)
                                ret = true;
                        }
                        return ret;
                    };
                    var orgbufferList = this.__bufferList;
                    var orgnumBufferList = this.__numBufferList;
                    var newbufferList = message.BufferList;
                    var newnumBufferList = message.NumBufInList;
                    MilWeb.assert(newbufferList.length == newnumBufferList, "Buffer list size mismatch");
                    var unionList = orgbufferList.concat(newbufferList);
                    unionList.forEach(function (element, index, array) {
                        var milWebObject = null;
                        var milWebGroupObject = null;
                        var Buffer = element;
                        var bufferName = Buffer.ExchangeBufferId;
                        var groupName = Buffer.ExchangeGroupId;
                        if (Buffer.BufferType) {
                            milWebObject = MilWeb.IdTable.filter(this.filterByBufferNameFn, this.getId(), bufferName)[0];
                            if (inbufferList(orgbufferList, bufferName) && (!inbufferList(newbufferList, bufferName))) {
                                // Object was deleted
                                MilWeb.log("object deleted");
                                if (milWebObject && milWebObject.__isinitialized) {
                                    // if object is part of a group remove it
                                    var milWebGroupObject = MilWeb.IdTable.get(this.getGroupId());
                                    if (milWebGroupObject) {
                                        milWebGroupObject.removeFromGroup(this, false);
                                    }
                                    milWebObject.__freed = true;
                                    this.callhookFunction(MilWeb.M_OBJECT_PUBLISH_WEB, milWebObject.getHookInfo());
                                }
                            } else if (!inbufferList(orgbufferList, bufferName) && (inbufferList(newbufferList, bufferName))) {
                                // Object was added
                                MilWeb.assert(milWebObject == undefined || !milWebObject.__isconnected || milWebObject.__freed, "MIL web object should be null, disconnecting or freed by the server.");
                                // Add
                                MilWeb.log("object Added " + bufferName + " groupname is " + groupName);
                                if (milWebObject == undefined) {
                                    if ((milWebObject = this.createObject(Buffer))) {
                                        milWebObject.setAppId(this.getId());
                                        milWebObject.setClientGroupId(this.getClientId());

                                        // Check if this object is part of a group
                                        if (groupName) {
                                            milWebObject.setgroupName(groupName);
                                            // Create the group if needed.
                                            milWebGroupObject = this.createGroup(groupName);
                                            // Add the object to the group
                                            milWebGroupObject.addToGroup(milWebObject);
                                        }

                                        // call user hook
                                        this.callhookFunction(MilWeb.M_OBJECT_PUBLISH_WEB, milWebObject.getHookInfo());
                                    }
                                }
                            } else if (inbufferList(orgbufferList, bufferName) && (inbufferList(newbufferList, bufferName))) {
                                // Object was not changed ( but may be the group)
                                if (groupName && milWebObject) {
                                    milWebObject.setgroupName(groupName);
                                    // Create the group if needed.
                                    milWebGroupObject = this.createGroup(groupName);
                                    // Add the object to the group
                                    milWebGroupObject.addToGroup(milWebObject);
                                }
                            } else {
                                MilWeb.assert(0, "invalid state");
                            }
                        }
                    }, this);

                    this.__bufferList = newbufferList;
                    this.__numBufferList = newnumBufferList;
                }
                break;
            default:
                MilWebObject.prototype.onCommand.call(this, message);
                break;
        }
    },

    createObject: function (BufferObj) {
        MilWeb.log("Create Object " + BufferObj.ExchangeBufferId);
        var bufferName = BufferObj.ExchangeBufferId;
        var groupName = BufferObj.ExchangeGroupId;
        var milWebObject = null;
        milWebObject = MilWeb.IdTable.filter(this.filterByBufferNameFn, this.getId(), bufferName)[0];
        if (milWebObject == undefined) {
            // New Object
            switch (BufferObj.BufferType) {
                case MilWeb.M_DISPLAY:
                    milWebObject = new MilDisplay(this.__url, bufferName, groupName);
                    break;

                case MilWeb.M_IMAGE:
                    milWebObject = new MilImage(this.__url, bufferName, groupName);
                    break;

                case MilWeb.M_ARRAY:
                    milWebObject = new MilArray(this.__url, bufferName, groupName);
                    break;

                case MilWeb.M_MESSAGE_MAILBOX:
                    milWebObject = new MilMessage(this.__url, bufferName, groupName);
                    break;
                default:
                    console.error("Not supported yet");
                    break;
            }
        }
        return milWebObject;
    },

    createGroup: function (Name) {
        var milWebGroupObject = MilWeb.IdTable.filter(this.filterByGroupNameFn, this.getId(), Name)[0];
        if (milWebGroupObject == undefined) {
            // New Group
            milWebGroupObject = new MilGroup(this.__url, Name);
            milWebGroupObject.setAppId(this.getId());
        } else {
            milWebGroupObject.setAppId(this.getId());
        }
        return milWebGroupObject;
    },

    displayStat: function (elementId, text) {
        if (this.__enableStat) {
            var newElement = document.getElementById(elementId + "_stat");
            if (!newElement) {
                newElement = document.createElement("pre");
                newElement.id = elementId + "_stat";
                this.__statDiv.appendChild(newElement);
            }
            newElement.innerHTML = text;
        }
    },

    fps: function (param, webObject) {
        if (this.__enableStat) {
            if (param > 0)
                webObject.__frameMin = Math.min(param, webObject.__frameMin);
            if (param < 100)
                webObject.__frameMax = Math.max(param, webObject.__frameMax);
            this.displayStat(webObject.getbufferName(), "FPS:(" + webObject.getbufferName() + " )(" + webObject.serialCounterDiff() + " )(" + 1000 / webObject.__frameMin + "|" + 1000 / webObject.__frameMax + "|" + (1000 / param) + ")");
        }
    },

    onUpdate: function (parameters) {
    },

    control: function (controlType, controlValue) {
        switch (controlType) {
            case MilWeb.M_CLOSE_CONNECTION:
                {
                    var webObject = MilWeb.IdTable.get(controlValue);
                    if (webObject)
                        webObject.onFree(true);
                }
                break;
            case MilWeb.M_ERROR:
                {
                    if (controlValue == MilWeb.M_PRINT_ENABLE)
                        this.__PRINT_DISABLE_VALUE = false;
                    else if (controlValue == MilWeb.M_PRINT_DISABLE)
                        this.__PRINT_DISABLE_VALUE = true;
                    else
                        this.onError(12);
                }
                break;
            default:
                this.onError(10);
                break;
        }
    },

    inquire: function (inquireType) {
        switch (inquireType) {
            case MilWeb.M_WEB_CLIENT_INDEX:
                return this.__clientId;
                break;
            case MilWeb.M_OBJECT_TYPE:
                return this.getType();
                break;
            default:
                break;
        }
        return MilWeb.M_NULL;
    },

    inquireConnection: function (inquireType, controlFlag, extraFlag) {
        switch (inquireType) {
            case MilWeb.M_WEB_PUBLISHED_NAME:
                {
                    var webObject = MilWeb.IdTable.filter(this.filterByBufferNameFn, this.getId(), controlFlag)[0];
                    if (webObject) {
                        if (webObject.getType() != MilWeb.M_GROUP) {
                            webObject.connect(webObject.geturl());
                        }
                        return webObject.getId();
                    }
                    else
                        return MilWeb.M_NULL;
                }
                break;

            case MilWeb.M_WEB_PUBLISHED_LIST:
                {
                    var webIdList = [];
                    var webList = MilWeb.IdTable.filter(this.filterByAppFn, this.getId());
                    for (var i = 0; i < webList.length; i++)
                        if (!webList[i].isFreed())
                            webIdList.push(webList[i].getId());
                    return webIdList;
                }
                break;

            default:
                break;
        }
        return null;
    }
};

MilWeb.extend(MilWebObject, MilApp);

//////////////////////////////////////////////////////////////////////////
// MIL Group
//////////////////////////////////////////////////////////////////////////
function MilGroup(url, groupName) {
    MilWebObject.call(this, url, groupName, groupName);
    this.__type = MilWeb.M_GROUP;
    this.__webObjectList = [];
    this.__memberList = [];
    this.__isinitialized = true;
    this.__lastgroupcounter = 0;
    MilWeb.IdTable.put(this);
}

MilGroup.prototype = {
    test: function () {
        MilWeb.log("Test group object");
    },

    isHookedOnUpdate: function () {
        return this.isHooked(MilWeb.M_UPDATE_END);
    },

    isInGroup: function (webObject) {
        var ret = false;
        for (var i = 0; i < this.__webObjectList.length && !ret; i++) {
            var objgrp = this.__webObjectList[i];
            MilWeb.assert(objgrp, "Invalid object group");
            if (objgrp === webObject)
                ret = true;
        }
        return ret;
    },

    getObjGroup: function (bufferName) {
        var ret = null;
        for (var i = 0; i < this.__webObjectList.length && !ret; i++) {
            var objgrp = this.__webObjectList[i];
            MilWeb.assert(objgrp, "Invalid object group");
            if (objgrp && objgrp.__name === bufferName)
                ret = objgrp;
        }
        return ret;
    },

    getObjGroupIndex: function (bufferName) {
        var ret = -1;
        for (var i = 0; i < this.__webObjectList.length && ret == -1; i++) {
            var objgrp = this.__webObjectList[i];
            MilWeb.assert(objgrp, "Invalid object group");
            if (objgrp && objgrp.__name === bufferName)
                ret = i;
        }
        return ret;
    },

    addToGroup: function (webObject) {
        MilWeb.assert((webObject instanceof MilDisplay) ||
            (webObject instanceof MilArray) ||
            (webObject instanceof MilMessage) ||
            (webObject instanceof MilImage),
            "group can contain only Array,Display,Message or Image type.");
        MilWeb.assert(webObject.__groupName === this.__groupName, "group name mismatch (1)" + this.__groupName);
        if (!this.getObjGroup(webObject.__bufferName)) {
            MilWeb.log("add to group");
            this.__webObjectList.push(
                {
                    webobj: webObject,
                    __name: webObject.__bufferName,
                }
            );
            this.__webObjectList.sort(function (webobject1, webobject2) { return webobject1.__sizebyte - webobject2.__sizebyte; });
            webObject.setGroupId(this.getId());
            this.__memberList.push(webObject.getId());
            var eventHook = [
                {
                    Type: MilWeb.M_OBJECT_ID,
                    Value: this.getId()

                },
                {
                    Type: MilWeb.M_COMPONENT_ADD,
                    Value: webObject.getId()

                },
            ];
            this.callhookFunction(MilWeb.M_COMPONENT_ADD, eventHook);
        }

    },
    removeFromGroup: function (webObject, fromUser) {
        MilWeb.assert((webObject instanceof MilDisplay) ||
            (webObject instanceof MilArray) ||
            (webObject instanceof MilMessage) ||
            (webObject instanceof MilImage),
            "group can contain only Array,Display, Message or Image type.");
        MilWeb.assert(webObject.__groupName === this.__groupName, "group name mismatch (1)" + this.__groupName);
        var index = this.getObjGroupIndex(webObject.__bufferName);
        if (index != -1) {
            delete this.__webObjectList[index];
            delete this.__memberList[index];
            this.__webObjectList.splice(index, 1);
            this.__memberList.splice(index, 1);
            if (!fromUser) {
                var ret = [
                    {
                        Type: MilWeb.OBJECT_ID,
                        Value: this.getId()

                    },
                    {
                        Type: MilWeb.OBJECT_REMOVE,
                        Value: webObject.getId()

                    },
                ];
                this.callhookFunction(MilWeb.M_COMPONENT_REMOVE, ret);
            }
            // if there is no members reset counter
            if (this.__webObjectList.length == 0) {
                this.__lastgroupcounter = 0;
            }
        }
    },

    updateData: function (webObject) {
        MilWeb.log("web object: " + webObject.__bufferName + ",GroupCounter:" + webObject.__groupcounter + ",serialCounter:" + webObject.__serialcounter + ",list: " + this.__webObjectList.length + ",LastCounter: " + this.__lastgroupcounter);
        MilWeb.assert(webObject.__groupName === this.__groupName, "group name mismatch");
        var objgrp = this.getObjGroup(webObject.__bufferName);
        MilWeb.assert(objgrp, "Invalid object group for object " + webObject.__bufferName);
        MilWeb.assert(webObject.__groupcounter < Number.MAX_VALUE, "Invalid group counter value");
        this.__lastgroupcounter = Math.max(this.__lastgroupcounter, webObject.__groupcounter);
        MilWeb.assert(!isNaN(this.__lastgroupcounter), "Invalid group counter value");
        MilWeb.assert(this.__lastgroupcounter < Number.MAX_VALUE, "Invalid group counter value");
        // Check for onUpdate hook
        var currentThis = this;
        setTimeout(function () { currentThis.onUpdate(); }, 0);
    },

    checkGroupIsSync: function () {
        var result = true;
        var webObjectArray = [];
        for (var i = 0; i < this.__webObjectList.length && result; i++) {
            webObjectArray[i] = this.__webObjectList[i];
            if (webObjectArray[i].__isconnecting && webObjectArray[i].__groupcounter != this.__lastgroupcounter) {
                MilWeb.log("webobject counter for  " + i + " " + webObjectArray[i].__groupcounter + " last " + this.__lastgroupcounter);
                result = false;
            }
        }

        if (!result)
            MilWeb.assert(result, "buffers in group " + this.__groupName + " are not sync");
        return result;
    },
    isAllConnected: function () {
        var result = true;
        for (var j = 0; j < this.__webObjectList.length; j++) {
            var webObject = this.__webObjectList[j].webobj;
            if (webObject.__isconnecting)
                result = result && webObject.__isconnected && webObject.__isinitialized;
        }
        return result;
    },

    isAllDataAvailable: function () {
        var result = true;
        for (var j = 0; j < this.__webObjectList.length; j++) {
            var webObject = this.__webObjectList[j].webobj;
            if (webObject.__isconnecting)
                result = result && webObject.__serverDataAvailable;
        }
        return result;
    },
    onUpdate: function (parameters) {
        MilWeb.log("on update group");
        if (this.isHookedOnUpdate()) {
            var bsync = true;
            if (this.__webObjectList.length && this.isAllConnected()) {
                for (var j = 0; j < this.__webObjectList.length; j++) {
                    var webObject = this.__webObjectList[j].webobj;

                    // Filter member of group that user don't want to connect to.
                    if (!webObject || !webObject.__isinitialized)
                        continue;

                    if (webObject.__groupcounter < this.__lastgroupcounter) {
                        bsync = false;
                        webObject.retrieveData();
                    }
                }

                if (!this.isAllDataAvailable()) {
                    bsync = false;
                }

                MilWeb.log("sync is " + bsync + " for group " + this.__groupName + " list " + this.__webObjectList.length);
                if (bsync) {
                    this.checkGroupIsSync();
                    this.callhookFunction(MilWeb.M_UPDATE_END, this.getHookInfo());
                    this.__lastgroupcounter++;
                    var timeout = (this.getframesPerSecond() > 0) ? (1000 / this.getframesPerSecond()) : this.getframesPerSecond();
                    // reclame new data
                    var currentThis = this;
                    setTimeout(function () { currentThis.onUpdate(); }, timeout);
                }
            } else {
                // We are hooked on but but no member hat connected yet
                // just fire another update
                var currentThis = this;
                setTimeout(function () { currentThis.onUpdate(); }, 0);
            }
        }
    },

    inquire: function (inquireType) {
        switch (inquireType) {
            case MilWeb.M_COMPONENT_ID_LIST:
                return this.__memberList;
                break;
            case MilWeb.M_OBJECT_TYPE:
                return this.getType();
                break;
            case MilWeb.M_OBJECT_NAME:
                return this.getbufferName();
                break;
            default:
                break;
        }
        return MilWeb.M_NULL;
    },

    hookFunction: function (hookTypeParam, HookHandler) {
        MilWeb.log("group hook");
        if ((hookTypeParam != MilWeb.M_UPDATE_END) &&
            (hookTypeParam != MilWeb.M_UPDATE_END + MilWeb.M_UNHOOK) &&
            (hookTypeParam != MilWeb.M_COMPONENT_ADD) &&
            (hookTypeParam != MilWeb.M_COMPONENT_ADD + MilWeb.M_UNHOOK) &&
            (hookTypeParam != MilWeb.M_COMPONENT_REMOVE) &&
            (hookTypeParam != MilWeb.M_COMPONENT_REMOVE + MilWeb.M_UNHOOK)) {
            // ERROR
            this.onError(13);
            return;
        }
        // if unhooking reset webobjects state
        for (var j = 0; j < this.__webObjectList.length; j++) {
            var webObject = this.__webObjectList[j].webobj;
            if (webObject) {
                webObject.__dataAvalaible = true;
            }
        }
        // call base class
        MilWebObject.prototype.hookFunction.call(this, hookTypeParam, HookHandler);
    }
};

MilWeb.extend(MilWebObject, MilGroup);
//////////////////////////////////////////////////////////////////////////
// MIL Canvas
//////////////////////////////////////////////////////////////////////////
function MilCanvas(domcanvas, parentId) {
    MilWeb.assert(domcanvas.getContext, "Not a canvas element");
    this.__parentId = parentId;
    this.__canvas = domcanvas;
    this.__canvasCtx = null;
    this.__drawMin = 100;
    this.__drawMax = -100;
    this.__convertMin = 100;
    this.__convertMax = -100;
    this.__otherFormats = false;
    this.__lastTouchIsEnd = false;

    if (this.supports_canvas()) {
        this.__canvasCtx = this.__canvas.getContext("2d");
        if (this.__parentId) {
            var displayObject = MilWeb.IdTable.get(this.__parentId);
            MilWeb.assert(displayObject, "The parent display should be valid");
			this.__canvas.addEventListener('contextmenu', function(event){event.preventDefault()});
			this.__canvas.addEventListener('auxclick', function(event) {event.preventDefault()});
            this.__canvas.addEventListener('mousemove', this.mouseMoveEvent.bind(this));
            this.__canvas.addEventListener('mousedown', this.mouseDownEvent.bind(this));
            this.__canvas.addEventListener('mouseup', this.mouseUpEvent.bind(this));
            this.__canvas.addEventListener('mouseleave', this.mouseLeaveEvent.bind(this));
            this.__canvas.addEventListener('wheel', this.mouseWheelEvent.bind(this));
            this.__canvas.addEventListener('keyup', this.keyUpEvent.bind(this));
            this.__canvas.addEventListener('keydown', this.keyDownEvent.bind(this));
            this.__canvas.addEventListener('touchstart', this.touchStartEvent.bind(this));
            this.__canvas.addEventListener('touchend', this.touchEndEvent.bind(this));
            this.__canvas.addEventListener('touchleave', this.touchEndEvent.bind(this));
            this.__canvas.addEventListener('touchcancel', this.touchCancelEvent.bind(this));
            this.__canvas.addEventListener('touchmove', this.touchMoveEvent.bind(this));
            this.__canvas.setAttribute('tabindex', '0');
            this.__canvas.focus();
        }

    }
    else {
        this.resize(10, 10);
    }

    this.__imageData = null;
    //////////////////////////
    this.__panning = false;
    this.__zooming = false;
    this.__startx = 0;
    this.__starty = 0;
    this.__startx0 = 0;
    this.__starty0 = 0;
    this.__endx = 0;
    this.__endy = 0;
    this.__endx0 = 0;
    this.__endy0 = 0;
    this.__startx1 = 0;
    this.__starty1 = 0;
    this.__endx1 = 0;
    this.__endy1 = 0;
    this.__ratio = 1;
    this.__startDistanceBetweenFingers = 0;
    this.__endDistanceBetweenFingers = 0;
}

MilCanvas.prototype = {
    supports_canvas: function () {
        return !!document.createElement('canvas').getContext;
    },

    resize: function (width, height) {
        if (this.__canvas && ((this.__imageData == null) || (width != this.__canvas.width) || (height != this.__canvas.height))) {

            this.__canvas.width = width;
            this.__canvas.height = height;

            // Created imageData.
            //this.__imageData = this.__canvasCtx.createImageData(this.__canvas.width, this.__canvas.height);
            //this.__imageData = this.__canvasCtx.getImageData(0, 0, this.__canvas.width, this.__canvas.height);
        }
    },

    logEvent: function (str) {
        //this.__log.textContent += "("+str + ")\n";

    },

    mouseDownEvent: function (evt) {
        //MilWeb.log(evt);
        this.__canvas.focus();
        var rect = this.__canvas.getBoundingClientRect();
        var x = Math.round((evt.clientX - rect.left));
        var y = Math.round((evt.clientY - rect.top));
        if (evt.button == 0 || evt.button == 1 || evt.button == 2) {

            var eventtype = (evt.button == 0) ? MilWeb.M_MOUSE_LEFT_BUTTON_DOWN : ((evt.button == 2) ? MilWeb.M_MOUSE_RIGHT_BUTTON_DOWN : MilWeb.M_MOUSE_MIDDLE_BUTTON_DOWN);
            var mousepositionx = x;
            var mousepositiony = y;
            var eventvalue = MilWeb.M_DEFAULT;
            var combinationkeys = (evt.button == 1) ? MilWeb.M_MOUSE_LEFT_BUTTON : ((evt.button == 2) ? MilWeb.M_MOUSE_RIGHT_BUTTON : MilWeb.M_MOUSE_MIDDLE_BUTTON);
            if (evt.altKey)
                combinationkeys |= MilWeb.M_KEY_ALT;
            if (evt.ctrlKey)
                combinationkeys |= MilWeb.M_KEY_CTRL;
            if (evt.shiftKey)
                combinationkeys |= MilWeb.M_KEY_SHIFT;
            if (evt.metaKey)
                combinationkeys |= MilWeb.M_KEY_WIN;

            var displayObject = MilWeb.IdTable.get(this.__parentId);
            MilWeb.assert(displayObject, "The parent display should be valid");
            if (displayObject && displayObject.mouseEnabled()) {
                displayObject.RCSendDisplayMessage(displayObject.__bufferName,
                    eventtype,
                    mousepositionx,
                    mousepositiony,
                    eventvalue,
                    combinationkeys);

                displayObject.callDispMessageHook(eventtype, mousepositionx, mousepositiony, eventvalue, combinationkeys);
            }
        }
    },

    mouseUpEvent: function (evt) {
        //MilWeb.log(evt);
        var rect = this.__canvas.getBoundingClientRect();
        var x = Math.round((evt.clientX - rect.left));
        var y = Math.round((evt.clientY - rect.top));
        if (evt.button == 0 || evt.button == 1 || evt.button == 2) {

            var eventtype = (evt.button == 0) ? MilWeb.M_MOUSE_LEFT_BUTTON_UP : ((evt.button == 2) ? MilWeb.M_MOUSE_RIGHT_BUTTON_UP : MilWeb.M_MOUSE_MIDDLE_BUTTON_UP);
            var mousepositionx = x;
            var mousepositiony = y;
            var eventvalue = MilWeb.M_DEFAULT;
            var combinationkeys = (evt.button == 1) ? MilWeb.M_MOUSE_LEFT_BUTTON : ((evt.button == 2) ? MilWeb.M_MOUSE_RIGHT_BUTTON : MilWeb.M_MOUSE_MIDDLE_BUTTON);
            if (evt.altKey)
                combinationkeys |= MilWeb.M_KEY_ALT;
            if (evt.ctrlKey)
                combinationkeys |= MilWeb.M_KEY_CTRL;
            if (evt.shiftKey)
                combinationkeys |= MilWeb.M_KEY_SHIFT;
            if (evt.metaKey)
                combinationkeys |= MilWeb.M_KEY_WIN;

            var displayObject = MilWeb.IdTable.get(this.__parentId);
            MilWeb.assert(displayObject, "The parent display should be valid");
            if (displayObject && displayObject.mouseEnabled()) {
                displayObject.RCSendDisplayMessage(displayObject.__bufferName,
                    eventtype,
                    mousepositionx,
                    mousepositiony,
                    eventvalue,
                    combinationkeys);

                displayObject.callDispMessageHook(eventtype, mousepositionx, mousepositiony, eventvalue, combinationkeys);
            }
        }
    },

    mouseMoveEvent: function (evt) {
        //MilWeb.log(evt);
        var rect = this.__canvas.getBoundingClientRect();
        var x = Math.round((evt.clientX - rect.left));
        var y = Math.round((evt.clientY - rect.top));

        if (evt.buttons == 1 || evt.buttons == 2 || evt.buttons == 4) {
            var eventtype = MilWeb.M_MOUSE_MOVE;
            var mousepositionx = x;
            var mousepositiony = y;
            var eventvalue = MilWeb.M_DEFAULT;
            var combinationkeys = (evt.buttons == 1) ? MilWeb.M_MOUSE_LEFT_BUTTON : ((evt.buttons == 2) ? MilWeb.M_MOUSE_RIGHT_BUTTON : MilWeb.M_MOUSE_MIDDLE_BUTTON);
            if (evt.altKey)
                combinationkeys |= MilWeb.M_KEY_ALT;
            if (evt.ctrlKey)
                combinationkeys |= MilWeb.M_KEY_CTRL;
            if (evt.shiftKey)
                combinationkeys |= MilWeb.M_KEY_SHIFT;
            if (evt.metaKey)
                combinationkeys |= MilWeb.M_KEY_WIN;

            var displayObject = MilWeb.IdTable.get(this.__parentId);
            MilWeb.assert(displayObject, "The parent display should be valid");
            if (displayObject) {

                displayObject.RCSendDisplayMessage(displayObject.__bufferName,
                    eventtype,
                    mousepositionx,
                    mousepositiony,
                    eventvalue,
                    combinationkeys);

                displayObject.callDispMessageHook(eventtype, mousepositionx, mousepositiony, eventvalue, combinationkeys);
            }
        }
    },
    mouseLeaveEvent: function (evt) {
        //MilWeb.log(evt);
        var rect = this.__canvas.getBoundingClientRect();
        var x = Math.round((evt.clientX - rect.left));
        var y = Math.round((evt.clientY - rect.top));

        if (evt.buttons == 1 || evt.buttons == 2 || evt.buttons == 4) {
            var eventtype = MilWeb.M_MOUSE_LEAVE;
            var mousepositionx = x;
            var mousepositiony = y;
            var eventvalue = MilWeb.M_DEFAULT;
            var combinationkeys = (evt.buttons == 1) ? MilWeb.M_MOUSE_LEFT_BUTTON : ((evt.buttons == 2) ? MilWeb.M_MOUSE_RIGHT_BUTTON : MilWeb.M_MOUSE_MIDDLE_BUTTON);
            if (evt.altKey)
                combinationkeys |= MilWeb.M_KEY_ALT;
            if (evt.ctrlKey)
                combinationkeys |= MilWeb.M_KEY_CTRL;
            if (evt.shiftKey)
                combinationkeys |= MilWeb.M_KEY_SHIFT;
            if (evt.metaKey)
                combinationkeys |= MilWeb.M_KEY_WIN;

            var displayObject = MilWeb.IdTable.get(this.__parentId);
            MilWeb.assert(displayObject, "The parent display should be valid");
            if (displayObject) {

                displayObject.RCSendDisplayMessage(displayObject.__bufferName,
                    eventtype,
                    mousepositionx,
                    mousepositiony,
                    eventvalue,
                    combinationkeys);

                displayObject.callDispMessageHook(eventtype, mousepositionx, mousepositiony, eventvalue, combinationkeys);
            }
        }
    },

    mouseWheelEvent: function (evt) {
        //MilWeb.log("delta Y " + evt.deltaY);
        var rect = this.__canvas.getBoundingClientRect();
        var x = Math.round((evt.clientX - rect.left));
        var y = Math.round((evt.clientY - rect.top));
        var delta = (evt.deltaY < 0 ? 1 : -1);

        var eventtype = MilWeb.M_MOUSE_WHEEL;

        var mousepositionx = x;
        var mousepositiony = y;
        var eventvalue = delta;
        var combinationkeys = MilWeb.M_NULL;
        if (evt.altKey)
            combinationkeys |= MilWeb.M_KEY_ALT;
        if (evt.ctrlKey)
            combinationkeys |= MilWeb.M_KEY_CTRL;
        if (evt.shiftKey)
            combinationkeys |= MilWeb.M_KEY_SHIFT;
        if (evt.metaKey)
            combinationkeys |= MilWeb.M_KEY_WIN;

        var displayObject = MilWeb.IdTable.get(this.__parentId);
        MilWeb.assert(displayObject, "The parent display should be valid");
        if (displayObject && displayObject.mouseEnabled()) {

            displayObject.RCSendDisplayMessage(displayObject.__bufferName,
                eventtype,
                mousepositionx,
                mousepositiony,
                eventvalue,
                combinationkeys);

            displayObject.callDispMessageHook(eventtype, mousepositionx, mousepositiony, eventvalue, combinationkeys);
        }
    },

    touchStartEvent: function (evt) {
        //MilWeb.log(evt);
        this.__panning = false;
        this.__zooming = false;
        this.__canvas.focus();
        var rect = this.__canvas.getBoundingClientRect();
        if (evt.touches.length == 1) {
            this.__panning = true;
            this.__startx0 = Math.round((evt.touches[0].clientX - rect.left));
            this.__starty0 = Math.round((evt.touches[0].clientY - rect.top));

            var eventtype = MilWeb.M_MOUSE_LEFT_BUTTON_DOWN;
            var mousepositionx = this.__startx0;
            var mousepositiony = this.__starty0;
            var eventvalue = MilWeb.M_DEFAULT;
            var combinationkeys = MilWeb.M_MOUSE_LEFT_BUTTON;
            var displayObject = MilWeb.IdTable.get(this.__parentId);

            MilWeb.assert(displayObject, "The parent display should be valid");
            if (displayObject && displayObject.mouseEnabled()) {
                displayObject.RCSendDisplayMessage(displayObject.__bufferName,
                    eventtype,
                    mousepositionx,
                    mousepositiony,
                    eventvalue,
                    combinationkeys);

                displayObject.callDispMessageHook(eventtype, mousepositionx, mousepositiony, eventvalue, combinationkeys);
            }
        }
        if (evt.touches.length == 2) {
            this.__zooming = true;
            this.__startx0 = Math.round((evt.touches[0].clientX - rect.left));
            this.__starty0 = Math.round((evt.touches[0].clientY - rect.top));
            this.__startx1 = Math.round((evt.touches[1].clientX - rect.left));
            this.__starty1 = Math.round((evt.touches[1].clientY - rect.top));
            this.startx = ((this.__startx0 + this.__startx1) / 2.0);
            this.starty = ((this.__starty0 + this.__starty1) / 2.0);
            this.__startDistanceBetweenFingers = Math.sqrt(Math.pow((this.__startx1 - this.__startx0), 2) + Math.pow((this.__starty1 - this.__starty0), 2));

        }
    },
    touchMoveEvent: function (evt) {
        //MilWeb.log(evt);
        evt.preventDefault();
        this.__canvas.focus();
        var rect = this.__canvas.getBoundingClientRect();
        if (this.__panning) {
            this.__endx0 = Math.round((evt.touches[0].clientX - rect.left));
            this.__endy0 = Math.round((evt.touches[0].clientY - rect.top));
            var eventtype = MilWeb.M_MOUSE_MOVE;
            var mousepositionx = this.__endx0;
            var mousepositiony = this.__endy0;
            var eventvalue = MilWeb.M_DEFAULT;
            var combinationkeys = MilWeb.M_MOUSE_LEFT_BUTTON;
            var displayObject = MilWeb.IdTable.get(this.__parentId);
            MilWeb.assert(displayObject, "The parent display should be valid");
            if (displayObject && displayObject.mouseEnabled()) {
                displayObject.RCSendDisplayMessage(displayObject.__bufferName,
                    eventtype,
                    mousepositionx,
                    mousepositiony,
                    eventvalue,
                    combinationkeys);

                displayObject.callDispMessageHook(eventtype, mousepositionx, mousepositiony, eventvalue, combinationkeys);
            }
        } else if (this.__zooming) {
            this.__endx0 = Math.round((evt.touches[0].clientX - rect.left));
            this.__endy0 = Math.round((evt.touches[0].clientY - rect.top));
            this.__endx1 = Math.round((evt.touches[1].clientX - rect.left));
            this.__endy1 = Math.round((evt.touches[1].clientY - rect.top));
            this.__endx = ((this.__endx0 + this.__endx1) / 2.0);
            this.__endy = ((this.__endy0 + this.__endy1) / 2.0);
            this.__endDistanceBetweenFingers = Math.sqrt(Math.pow((this.__endx1 - this.__endx0), 2) + Math.pow((this.__endy1 - this.__endy0), 2));
            this.__ratio = this.__endDistanceBetweenFingers / this.__startDistanceBetweenFingers;
        }
    },

    touchEndEvent: function (evt) {
        //MilWeb.log(evt);
        this.__canvas.focus();
        var rect = this.__canvas.getBoundingClientRect();
        if (this.__panning) {
            this.__panning = false;
            this.__endx0 = Math.round((evt.touches[0].clientX - rect.left));
            this.__endy0 = Math.round((evt.touches[0].clientY - rect.top));
            var eventtype = MilWeb.M_MOUSE_LEFT_BUTTON_UP;
            var mousepositionx = this.__endx0;
            var mousepositiony = this.__endy0;
            var eventvalue = MilWeb.M_DEFAULT;
            var combinationkeys = MilWeb.M_MOUSE_LEFT_BUTTON;
            var displayObject = MilWeb.IdTable.get(this.__parentId);
            MilWeb.assert(displayObject, "The parent display should be valid");
            if (displayObject && displayObject.mouseEnabled()) {
                displayObject.RCSendDisplayMessage(displayObject.__bufferName,
                    eventtype,
                    mousepositionx,
                    mousepositiony,
                    eventvalue,
                    combinationkeys);

                displayObject.callDispMessageHook(eventtype, mousepositionx, mousepositiony, eventvalue, combinationkeys);
            }
        } else if (this.__zooming) {
            this.__zooming = false;
            var eventtype = MilWeb.M_MOUSE_WHEEL;
            var mousepositionx = this.__endx;
            var mousepositiony = this.__endy;
            var eventvalue = this.__ratio - 1;
            var combinationkeys = MilWeb.M_DEFAULT;
            var displayObject = MilWeb.IdTable.get(this.__parentId);
            MilWeb.assert(displayObject, "The parent display should be valid");
            if (displayObject && displayObject.mouseEnabled()) {

                displayObject.RCSendDisplayMessage(displayObject.__bufferName,
                    eventtype,
                    mousepositionx,
                    mousepositiony,
                    eventvalue,
                    combinationkeys);

                displayObject.callDispMessageHook(eventtype, mousepositionx, mousepositiony, eventvalue, combinationkeys);
            }

        }
    },
    touchCancelEvent: function (evt) {
        //MilWeb.log(evt);
        this.__canvas.focus();
        var rect = this.__canvas.getBoundingClientRect();
        if (this.__panning) {
            this.__panning = false;
        } else if (this.__zooming) {
            this.__zooming = false;
        }
    },

    keyUpEvent: function (evt) {
        //MilWeb.log(evt);
        var value = evt.keyCode;
        var eventtype = MilWeb.M_KEY_UP;
        var mousepositionx = MilWeb.M_DEFAULT;
        var mousepositiony = MilWeb.M_DEFAULT;
        var eventvalue = value;
        var combinationkeys = MilWeb.M_NULL;
        if (evt.altKey)
            combinationkeys |= MilWeb.M_KEY_ALT;
        if (evt.ctrlKey)
            combinationkeys |= MilWeb.M_KEY_CTRL;
        if (evt.shiftKey)
            combinationkeys |= MilWeb.M_KEY_SHIFT;
        if (evt.metaKey)
            combinationkeys |= MilWeb.M_KEY_WIN;
        var displayObject = MilWeb.IdTable.get(this.__parentId);
        MilWeb.assert(displayObject, "The parent display should be valid");
        if (displayObject && displayObject.keyboardEnabled()) {
            displayObject.RCSendDisplayMessage(displayObject.__bufferName,
                eventtype,
                mousepositionx,
                mousepositiony,
                eventvalue,
                combinationkeys);

            displayObject.callDispMessageHook(eventtype, mousepositionx, mousepositiony, eventvalue, combinationkeys);
        }
    },

    keyDownEvent: function (evt) {
        //MilWeb.log(evt);
        var value = evt.keyCode;
        var eventtype = MilWeb.M_KEY_DOWN;
        var mousepositionx = MilWeb.M_DEFAULT;
        var mousepositiony = MilWeb.M_DEFAULT;
        var eventvalue = value;
        var combinationkeys = MilWeb.M_NULL;
        if (evt.altKey)
            combinationkeys |= MilWeb.M_KEY_ALT;
        if (evt.ctrlKey)
            combinationkeys |= MilWeb.M_KEY_CTRL;
        if (evt.shiftKey)
            combinationkeys |= MilWeb.M_KEY_SHIFT;
        if (evt.metaKey)
            combinationkeys |= MilWeb.M_KEY_WIN;
        var displayObject = MilWeb.IdTable.get(this.__parentId);
        MilWeb.assert(displayObject, "The parent display should be valid");
        if (displayObject && displayObject.keyboardEnabled()) {
            displayObject.RCSendDisplayMessage(displayObject.__bufferName,
                eventtype,
                mousepositionx,
                mousepositiony,
                eventvalue,
                combinationkeys);

            displayObject.callDispMessageHook(eventtype, mousepositionx, mousepositiony, eventvalue, combinationkeys);
        }
    },

    drawTest: function () {
        this.__canvasCtx.fillRect(50, 25, 150, 100);
    },

    drawImageData: function (data, format) {
        var t0 = performance.now();
        var bytes = new Uint8ClampedArray(data);
        var currentThis = this;
        var tmpImageData = this.__canvasCtx.getImageData(0, 0, this.__canvas.width, this.__canvas.height);

        if (format == MilWeb.M_RGB32)
            tmpImageData.data.set(bytes);
        else
            MilWeb.Utility.Draw(tmpImageData, bytes, format);

        currentThis.__canvasCtx.putImageData(tmpImageData, 0, 0);
        data = undefined;
        bytes = undefined;
        var t1 = performance.now();
        this.__drawMin = Math.min((t1 - t0), this.__drawMin);
        this.__drawMax = Math.max((t1 - t0), this.__drawMax);
        if (MilWeb.ENABLE_STAT) {
            var displayObject = MilWeb.IdTable.get(this.__parentId);
            MilWeb.assert(displayObject, "The parent display should be valid");
            var App = MilWeb.IdTable.get(displayObject.getAppId());
            if (App)
                App.displayStat('canvas2', "DrawImage: (" + this.__drawMin + "|" + this.__drawMax + "|" + (t1 - t0) + ")");
        }
    }
};

//////////////////////////////////////////////////////////////////////////
// MIL Display
//////////////////////////////////////////////////////////////////////////
function MilDisplay(url, bufferName, groupName) {
    MilWebObject.call(this, url, bufferName, groupName);
    this.__type = MilWeb.M_DISPLAY;
    this.__canvas = null;
    this.__mouseEnabled = true;
    this.__keyboardEnabled = true;
    this.__start = 0;
    this.__end = 0;
    this.__diff = 0;
    MilWeb.IdTable.put(this);
}

MilDisplay.prototype = {
    test: function () {
        MilWeb.log("Test display object");
    },

    mouseEnabled: function () {
        return this.__mouseEnabled;
    },

    keyboardEnabled: function () {
        return this.__keyboardEnabled;
    },

    terminate: function () {
        MilWebObject.prototype.terminate.call(this);
        if (this.__canvas) {
            this.__canvas = null;
        }
    },

    onCommand: function (message) {
        //MilWeb.log(message);
        var Command = message.Command;
        switch (Command) {
            case this.Protocol.SERVER_OBJECT_INFO:
                {
                    try {
                        MilWeb.log(message);
                        var BufferType = message.BufferType;
                        MilWeb.assert(BufferType === this.__type, "Expect the same buffer type !!!");
                        this.__isinteractive = message.DisplayInteractive;
                        this.__clientId = message.ClientId;
                        if (message.DisplayStruct) {
                            MilWeb.log(message.DisplayStruct);
                            this.__sizex = message.DisplayStruct.SizeX;
                            this.__sizey = message.DisplayStruct.SizeY;
                            this.__format = message.DisplayStruct.Format;
                            this.__isenabled = (message.DisplayStruct.Enabled != 0);
                            this.__isinitialized = true;
                            this.__accesstype = (message.DisplayStruct.AccessType & 0xFF);
                            this.__mouseEnabled = ((message.DisplayStruct.AccessType & MilWeb.M_WEB_MOUSE_USE) == MilWeb.M_WEB_MOUSE_USE);
                            this.__keyboardEnabled = ((message.DisplayStruct.AccessType & MilWeb.M_WEB_KEYBOARD_USE) == MilWeb.M_WEB_KEYBOARD_USE);
                            this.subscribe();
                        }
                    } catch (e) {
                        throw (e);
                    }
                }
                break;

            case this.Protocol.SERVER_OBJECT_CONNECTED:
                {
                    this.callhookFunction(MilWeb.M_CONNECT, this.getHookInfo());
                    this.callOnUpdate();
                }
                break;

            case this.Protocol.SERVER_DISPLAY_INTERACTIVE_STATE:
                {
                    try {
                        MilWeb.log(message);
                        var BufferType = message.BufferType;
                        MilWeb.assert(BufferType === this.__type, "Expect the same buffer type !!!");
                        this.__isinteractive = message.DisplayInteractive;
                        this.callhookFunction(MilWeb.M_UPDATE_INTERACTIVE_STATE, this.getHookInfo());
                    } catch (e) {
                        throw (e);
                    }
                }
                break;

            default:
                MilWebObject.prototype.onCommand.call(this, message);
                break;
        }
    },

    doJob: function () {
        if (this.__data) {
            this.__start = performance.now();
            if (this.__canvas) {
                this.__canvas.drawImageData(this.__data, this.__format);
            }

            // cal user hooks
            this.callhookFunction(MilWeb.M_UPDATE_WEB, this.getHookInfo());
            this.__end = performance.now();
            this.__diff = this.__end - this.__start;
            if (MilWeb.ENABLE_STAT) {
                var App = MilWeb.IdTable.get(this.getAppId());
                if (App) {
                    App.fps(this.__diff, this);
                }
            }
        }
    },

    DummyUpdateHookHandler: function (hookType, eventId, userVar) {
    }
    ,
    selectCanvas: function (canvasElement) {
        MilWeb.assert(canvasElement.getContext, "Not a canvas element");
        MilWeb.assert(this.__isinitialized, "Not initialized");
        MilWeb.assert(this.__isconnected, "Not connected");
        if (this.__isinitialized && !this.__canvas) {
            if (this.getGroupId() == MilWeb.M_NULL && !this.isHookedOnUpdate()) {
                MilWeb.MobjHookFunction(this.getId(), MilWeb.M_UPDATE_WEB, this.DummyUpdateHookHandler);
            }
            this.__canvas = new MilCanvas(canvasElement, this.getId());
            this.__canvas.resize(this.__sizex, this.__sizey);
        }
    },

    deselectCanvas: function () {
        MilWeb.MobjHookFunction(this.getId(), MilWeb.M_UPDATE_WEB + MilWeb.M_UNHOOK, this.DummyUpdateHookHandler);
        if (this.__canvas) {
            this.__canvas = null;
        }
    },

    callDispMessageHook: function (eventtype, mousepositionx, mousepositiony, eventvalue, combinationkeys) {
        // Call Message hook
        var eventHook = [
            {
                Type: MilWeb.M_DISPLAY,
                Value: this.getId()

            },
            {
                Type: MilWeb.M_MOUSE_POSITION_X,
                Value: mousepositionx

            },
            {
                Type: MilWeb.M_MOUSE_POSITION_Y,
                Value: mousepositiony
            },
            {
                Type: MilWeb.M_EVENT_VALUE,
                Value: eventvalue
            },
            {
                Type: MilWeb.M_COMBINATION_KEYS,
                Value: combinationkeys
            },
        ];

        this.callhookFunction(eventtype, eventHook);
    },

    inquire: function (inquireType) {
        switch (inquireType) {
            case MilWeb.M_INTERACTIVE:
                {
                    if (this.__isinteractive)
                        return MilWeb.M_ENABLE;
                    else
                        return MilWeb.M_DISABLE;
                }
                break;
            case MilWeb.M_IMAGE_HOST_ADDRESS:
                {
                    return this.__data;
                }
                break;
            default:
                return MilWebObject.prototype.inquire.call(this, inquireType);
        };
        return MilWeb.M_NULL;
    },
    control: function (controlType, controlValue) {
        switch (controlType) {
            case MilWeb.M_UPDATE_WEB:
                if (controlValue == MilWeb.M_DISABLE) {
                    this.__isenabled = false;
                } else if (controlValue == MilWeb.M_NOW) {
                    this.doJob();
                } else if (controlValue == MilWeb.M_ENABLE) {
                    this.__isenabled = true;
                    var currentThis = this;
                    setTimeout(function () { currentThis.onUpdate(); }, 0);
                } else if (controlValue == MilWeb.M_FORCE) {
                    this.__isenabled = true;
                    this.RCForceUpdateInProgress();
                }

                break;
            case MilWeb.M_INTERACTIVE:
                if (controlValue != MilWeb.M_DISABLE && controlValue != MilWeb.M_ENABLE) {
                    this.onError(14);
                }
                else {
                    this.__isinteractive = (controlValue == MilWeb.M_ENABLE);
                    this.RCSetInteractiveDisplay(this.__bufferName, controlValue);
                }
                break;
            default:
                MilWebObject.prototype.control.call(this, controlType, controlValue);
        };
    }
};

MilWeb.extend(MilWebObject, MilDisplay);

//////////////////////////////////////////////////////////////////////////
// MIL Image
//////////////////////////////////////////////////////////////////////////
function MilImage(url, bufferName, groupName) {
    MilWebObject.call(this, url, bufferName, groupName);
    this.__type = MilWeb.M_IMAGE;
    MilWeb.IdTable.put(this);
}

MilImage.prototype = {
    test: function () {
        MilWeb.log("Test image object");
    },

    onCommand: function (message) {
        //MilWeb.log(message);
        var Command = message.Command;
        switch (Command) {
            case this.Protocol.SERVER_OBJECT_INFO:
                {
                    try {
                        var BufferType = message.BufferType;
                        MilWeb.assert(BufferType === this.__type, "Expect the same buffer type !!!");
                        this.__clientId = message.ClientId;
                        if (message.ImageStruct) {
                            MilWeb.log(message.ImageStruct);
                            this.__sizex = message.ImageStruct.SizeX;
                            this.__sizey = message.ImageStruct.SizeY;
                            this.__format = message.ImageStruct.Format;
                            this.__accesstype = message.ImageStruct.AccessType;
                            this.__datatype = message.ImageStruct.DataType;
                            this.__isinitialized = true;
                            this.subscribe();
                        }
                    } catch (e) {
                        throw (e);
                    }
                }
                break;
            case this.Protocol.SERVER_OBJECT_CONNECTED:
                {
                    this.callhookFunction(MilWeb.M_CONNECT, this.getHookInfo());
                    this.callOnUpdate();
                }
                break;

            default:
                MilWebObject.prototype.onCommand.call(this, message);
                break;
        }
    },

    doJob: function () {
        if (this.__data) {
            if (this.getGroupId() == MilWeb.M_NULL) {
                // cal user hooks
                this.callhookFunction(MilWeb.M_UPDATE_WEB, this.getHookInfo());
            }
        }
    }

};

MilWeb.extend(MilWebObject, MilImage);

//////////////////////////////////////////////////////////////////////////
// MIL Array
//////////////////////////////////////////////////////////////////////////
function MilArray(url, bufferName, groupName) {
    MilWebObject.call(this, url, bufferName, groupName);
    this.__type = MilWeb.M_ARRAY;
    MilWeb.IdTable.put(this);
}

MilArray.prototype = {
    test: function () {
        MilWeb.log("Test array object");
    },

    onCommand: function (message) {
        //MilWeb.log(message);
        var Command = message.Command;
        switch (Command) {
            case this.Protocol.SERVER_OBJECT_INFO:
                {
                    try {

                        var BufferType = message.BufferType;
                        MilWeb.assert(BufferType === this.__type, "Expect the same buffer type !!!");
                        this.__clientId = message.ClientId;
                        if (message.ArrayStruct) {
                            MilWeb.log(message.ArrayStruct);
                            this.__sizex = message.ArrayStruct.SizeX;
                            this.__sizey = message.ArrayStruct.SizeY;
                            this.__accesstype = message.ArrayStruct.AccessType;
                            this.__datatype = message.ArrayStruct.DataType;
                            this.__isinitialized = true;
                            this.subscribe();
                        }
                    } catch (e) {
                        this.__isinitialized = false;
                    }
                }
                break;

            case this.Protocol.SERVER_OBJECT_CONNECTED:
                {
                   this.callhookFunction(MilWeb.M_CONNECT, this.getHookInfo());
                   this.callOnUpdate();
                }
                break;

            default:
                MilWebObject.prototype.onCommand.call(this, message);
                break;
        }
    },

    doJob: function () {
        MilImage.prototype.doJob.call(this);
    }

};

MilWeb.extend(MilWebObject, MilArray);

//////////////////////////////////////////////////////////////////////////
// MIL Message
//////////////////////////////////////////////////////////////////////////
function MilMessage(url, bufferName, groupName) {
    MilWebObject.call(this, url, bufferName, groupName);
    this.__type = MilWeb.M_MESSAGE_MAILBOX;
    MilWeb.IdTable.put(this);
}

MilMessage.prototype = {
    test: function () {
        MilWeb.log("Test Message object");
    },
    onCommand: function (message) {
        //MilWeb.log(message);
        var Command = message.Command;
        switch (Command) {
            case this.Protocol.SERVER_OBJECT_INFO:
                {
                    try {
                        MilWeb.log(message);
                        var BufferType = message.BufferType;
                        //MilWeb.assert(BufferType === this.__type, "Expect the same buffer type !!!");
                        this.__clientId = message.ClientId;
                        if (message.MessageStruct) {
                            MilWeb.log(message.MessageStruct);
                            this.__sizebyte = message.MessageStruct.SizeByte;
                            this.__accesstype = message.MessageStruct.AccessType;
                            this.__datatype = message.MessageStruct.DataType;
                            this.__messagetag = message.MessageStruct.MessageTag;
                            this.__isinitialized = true;
                        }
                    } catch (e) {
                        this.__isinitialized = false;
                    }
                    if (this.__isinitialized) {
                        this.subscribe();
                    }
                }
                break;
            case this.Protocol.SERVER_OBJECT_CONNECTED:
                {
                    this.callhookFunction(MilWeb.M_CONNECT, this.getHookInfo());
		    this.callOnUpdate();
                }
                break;

            default:
                MilWebObject.prototype.onCommand.call(this, message);
                break;
        }
    },


    doJob: function () {
        if (this.__data) {
            if (this.getGroupId() == MilWeb.M_NULL) {
                // cal user hooks
                this.callhookFunction(MilWeb.M_UPDATE_WEB, this.getHookInfo());
            }
        }
    },

    inquire: function (inquireType) {
        switch (inquireType) {
            case MilWeb.M_MESSAGE_LENGTH:
                return this.__sizebyte;
                break;
            default:
                return MilWebObject.prototype.inquire.call(this, inquireType);
                break;
        }
    }
};

MilWeb.extend(MilWebObject, MilMessage);

