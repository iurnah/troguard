#ifndef STUBS_H_
#define STUBS_H_

#include "third_party/npapi/bindings/nphostapi.h"

//For NPPluginFuncs
NPError StubNewInstance(NPMIMEType pluginType,
                        NPP instance,
                        uint16 mode,
                        int16 argc,
                        char *argn[],
                        char *argv[],
                        NPSavedData *saved) {
  return NPERR_NO_ERROR;
}

NPError StubSetWindow(NPP instance, NPWindow *window) {
  return NPERR_NO_ERROR;
}

static NPError StubDestroy(NPP instance, NPSavedData **save) {
  return NPERR_NO_ERROR;
}

static NPError StubNewStream(NPP instance,
                             NPMIMEType type,
                             NPStream *stream,
                             NPBool seekable,
                             uint16 *stype) {
  return NPERR_NO_ERROR;
}

static NPError StubDestroyStream(NPP instance,
                                 NPStream *stream,
                                 NPReason reason) {
  return NPERR_NO_ERROR;
}

static void StubStreamAsFile(NPP instance,
                             NPStream* stream,
                             const char *fname) {
}

static int32 StubWriteReady(NPP instance, NPStream *stream) {
  return 0;
}

static int32 StubWrite(NPP instance,
                       NPStream* stream,
                       int32 offset, 
                       int32 len, 
                       void *buf) {
  return -1;
}

static void StubPrint(NPP instance, NPPrint *PrintInfo) {}

static int16 StubHandleEvent(NPP instance, void* event) {
  return false;
}

static void StubURLNotify(NPP instance,
                          const char *url,
                          NPReason reason, 
                          void *notifyData) {
}

static NPError StubSetValue(NPP instance, NPNVariable variable,
                            void *ret_alue) {
  return NPERR_NO_ERROR;
}

//For Javascript listener
static void StubInvalidate(NPObject *npobj) {}

static bool StubInvokeDefault(NPObject *npobj,
                              const NPVariant *args,
                              uint32_t argCount,
                              NPVariant *result) {
  return false;
}

static bool StubHasProperty(NPObject * npobj, NPIdentifier name) {
  return false;
}

static bool StubGetProperty(NPObject *npobj,
                            NPIdentifier name,
                            NPVariant *result) {
  return false;
}

static bool StubSetProperty(NPObject *npobj,
                            NPIdentifier name,
                            const NPVariant *value) {
  return false;
}

static bool StubRemoveProperty(NPObject *npobj, NPIdentifier name) {
  return false;
}

static bool StubEnumerate(NPObject *npobj, NPIdentifier **identifier,
                          uint32_t *count) {
  return false;
}

static bool StubConstruct(NPObject *npobj,
                          const NPVariant *args,
                          uint32_t argCount,
                          NPVariant *result) {
  return false;
}

#endif //STUBS_H