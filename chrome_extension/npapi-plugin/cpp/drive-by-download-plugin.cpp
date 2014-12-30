#include "drive-by-download-plugin.h"
#include "stubs.h"
#include "drive-by-download.h"
#include <sstream>

NPPluginFuncs *pluginFuncs = NULL;
NPNetscapeFuncs *browserFuncs = NULL;
NPObject *javascriptListener = NULL;

static NPClass JavascriptListener_NPClass = {
  NP_CLASS_STRUCT_VERSION_CTOR,
  Allocate,
  Deallocate,
  StubInvalidate,
  HasJavascriptMethod,
  InvokeJavascript,
  StubInvokeDefault,
  StubHasProperty,
  StubGetProperty,
  StubSetProperty,
  StubRemoveProperty,
  StubEnumerate,
  StubConstruct
}; //NPClass JavascriptListener_NPClass

extern "C" {
NPError NP_Initialize(NPNetscapeFuncs *browser_funcs, NPPluginFuncs *plugin_funcs) {
  NPError error = SetPluginFuncs(plugin_funcs);
  if (error != NPERR_NO_ERROR) {
    ResetFuncs();
    return error;
  }
  error = SetBrowserFuncs(browser_funcs);
  if (error != NPERR_NO_ERROR) {
    ResetFuncs();
  }
  return error;
}

char *NP_GetMIMEDescription(void) {
  return (char *)"application/x-drive-by-download-plugin::Allows filesystem access to Chrome extensions;";
}
/* The above is for windows. p
 * */
//This is also the NPP_GetValue()
NPError NP_GetValue(void *instance, NPPVariable variable, void *value) {
  switch(variable) {
    case NPPVpluginNameString: {
      *((char **)value) = (char *)"Chrome extensions for Drive-by Download project";
      break;
    }
    case NPPVpluginDescriptionString: {
      *((char **)value) = (char *)"Plugin to allow Chrome extensions to start local application (firefox)";
      break;
    }
    default: {
      return NPERR_INVALID_PARAM;
    }
  }
  return NPERR_NO_ERROR;
}
}

extern "C" {
NPError NP_Shutdown(void) {
  if (javascriptListener != NULL) {
    browserFuncs->releaseobject(javascriptListener);
  } 
  return NPERR_NO_ERROR;
}
}

NPError SetPluginFuncs(NPPluginFuncs *plugin_funcs) {
  if (pluginFuncs != NULL) {
    return NPERR_INVALID_FUNCTABLE_ERROR;
  }
  pluginFuncs = plugin_funcs;
  
  plugin_funcs->newp = StubNewInstance;
  plugin_funcs->destroy = StubDestroy;
  plugin_funcs->setwindow = StubSetWindow;
  plugin_funcs->newstream = StubNewStream;
  plugin_funcs->destroystream = StubDestroyStream;
  plugin_funcs->asfile = StubStreamAsFile;
  plugin_funcs->writeready = StubWriteReady;
  plugin_funcs->write = StubWrite;
  plugin_funcs->print = StubPrint;
  plugin_funcs->event = StubHandleEvent;
  plugin_funcs->urlnotify = StubURLNotify;
  plugin_funcs->getvalue = GetValue;
  plugin_funcs->setvalue = StubSetValue;
  
  return NPERR_NO_ERROR;
}

NPError SetBrowserFuncs(NPNetscapeFuncs *browser_funcs) {
  if (browserFuncs != NULL) {
    return NPERR_INVALID_FUNCTABLE_ERROR;
  }
  browserFuncs = browser_funcs;
  return NPERR_NO_ERROR;
}

void ResetFuncs(void) {
  browserFuncs = NULL;
  pluginFuncs = NULL;
}

NPObject *Allocate(NPP instance, NPClass *clazz) {
  NPObject *obj = (NPObject *)new NPClassWithNPP;
  obj->_class = clazz;
  obj->referenceCount = 0;
  return obj;
}

void Deallocate(NPObject *obj) {
  delete (NPClassWithNPP *)obj;
}

void SetInstance(NPP instance, NPObject *passedObj) {
  NPClassWithNPP *obj = (NPClassWithNPP *)passedObj;
  obj->npp = instance;
}

NPP GetInstance(NPObject *passedObj) {
  NPClassWithNPP *obj = (NPClassWithNPP *)passedObj;
  return obj->npp;
}

// This is the NPP_GetValue
NPError GetValue(NPP instance, NPPVariable variable, void *value) {
  switch (variable) {
    case NPPVpluginScriptableNPObject: {
      javascriptListener = (NPObject *)browserFuncs->createobject(instance, (NPClass *)&JavascriptListener_NPClass); //createobject === NPN_CreateObject()
      *((NPObject **)value) = javascriptListener;//this is very hard to understand. pay attention.
      SetInstance(instance, javascriptListener);
      break;
    }
    case NPPVpluginNeedsXEmbed: {
      *((bool *)value) = true;
      break;
    }
    default: {
      return NPERR_INVALID_PARAM;
    }
  }
  return NPERR_NO_ERROR;
}

bool HasJavascriptMethod(NPObject *npobj, NPIdentifier name) {
  //const char *method = browser_funcs_->utf8fromidentifier(name);
  return true;
}

bool InvokeJavascript(NPObject *npobj,
                      NPIdentifier name,
                      const NPVariant *args,
                      uint32_t argCount,
                      NPVariant *result) {
  const char *methodName = browserFuncs->utf8fromidentifier(name);
  bool success = false;
  if(argCount == 0){
		if (!strcmp(methodName, "startFirefox")) {
    char *value = NULL;
    size_t len = 0;
			if (startFirefox(value, len)) {
				success = SetReturnValue(value, len, *result);
				delete[] value;
			}
		}
	}
		
  browserFuncs->memfree((void *)methodName);
  return success;
}

NPVariant *eval(NPP instance, const char *scriptString) {
  NPString script;
  script.UTF8Characters = scriptString;
  script.UTF8Length = strlen(script.UTF8Characters);
  
  NPObject *window = NULL;
  browserFuncs->getvalue(instance, NPNVWindowNPObject, &window);
  NPVariant *result = new NPVariant();
  browserFuncs->evaluate(instance, window, &script, result);
  browserFuncs->releaseobject(window);
  return result;
}
/*
bool SetReturnValue(const bool value, NPVariant &result) {
  BOOLEAN_TO_NPVARIANT(value, result);
  return true;
}
*/
bool SetReturnValue(const char *value, const size_t len, NPVariant &result) {
  const size_t dstLen = len + 1;
  char *resultString = (char *)browserFuncs->memalloc(dstLen);
  if (!resultString) {
    return false;
  }
  memcpy(resultString, value, len);
  resultString[dstLen - 1] = 0;
  STRINGN_TO_NPVARIANT(resultString, len, result);
  return true;
}



