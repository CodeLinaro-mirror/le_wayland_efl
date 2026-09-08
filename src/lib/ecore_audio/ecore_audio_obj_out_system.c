#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ecore_audio_private.h"

#define MY_CLASS ECORE_AUDIO_OUT_SYSTEM_CLASS

typedef struct
{
   Eo *backend;
   Ecore_Job *ready_job;
} Ecore_Audio_Out_System_Data;

#if defined(_WIN32) || defined(HAVE_PULSE)
static void
_context_ready_cb(void *data, const Efl_Event *event)
{
   efl_event_callback_call(data, ECORE_AUDIO_OUT_SYSTEM_EVENT_CONTEXT_READY,
                          event->info);
}

static void
_context_fail_cb(void *data, const Efl_Event *event)
{
   efl_event_callback_call(data, ECORE_AUDIO_OUT_SYSTEM_EVENT_CONTEXT_FAIL,
                          event->info);
}
#endif

#ifdef _WIN32
static void
_stop_cb(void *data, const Efl_Event *event)
{
   efl_event_callback_call(data, ECORE_AUDIO_OUT_SYSTEM_EVENT_STOP, event->info);
}

static void
_ready_job(void *data)
{
   Ecore_Audio_Out_System_Data *pd = efl_data_scope_get(data, MY_CLASS);

   pd->ready_job = NULL;
   efl_event_callback_call(data, ECORE_AUDIO_OUT_SYSTEM_EVENT_CONTEXT_READY, NULL);
}
#endif

EOLIAN static Eo *
_ecore_audio_out_system_efl_object_constructor(Eo *eo_obj, Ecore_Audio_Out_System_Data *pd)
{
   eo_obj = efl_constructor(efl_super(eo_obj, MY_CLASS));
   if (!eo_obj) return NULL;

#ifdef _WIN32
   pd->backend = efl_add_ref(ECORE_AUDIO_OUT_WASAPI_CLASS, NULL);
#elif defined(HAVE_PULSE)
   pd->backend = efl_add_ref(ECORE_AUDIO_OUT_PULSE_CLASS, NULL);
#else
   ERR("No system audio output backend was compiled in");
#endif
   if (!pd->backend) return NULL;

#ifdef _WIN32
   efl_event_callback_add(pd->backend, ECORE_AUDIO_OUT_WASAPI_EVENT_CONTEXT_READY,
                          _context_ready_cb, eo_obj);
   efl_event_callback_add(pd->backend, ECORE_AUDIO_OUT_WASAPI_EVENT_CONTEXT_FAIL,
                          _context_fail_cb, eo_obj);
   efl_event_callback_add(pd->backend, ECORE_AUDIO_OUT_WASAPI_EVENT_STOP,
                          _stop_cb, eo_obj);
   /* WASAPI initializes synchronously and does not emit context,ready. */
   pd->ready_job = ecore_job_add(_ready_job, eo_obj);
#elif defined(HAVE_PULSE)
   efl_event_callback_add(pd->backend, ECORE_AUDIO_OUT_PULSE_EVENT_CONTEXT_READY,
                          _context_ready_cb, eo_obj);
   efl_event_callback_add(pd->backend, ECORE_AUDIO_OUT_PULSE_EVENT_CONTEXT_FAIL,
                          _context_fail_cb, eo_obj);
#endif
   return eo_obj;
}

EOLIAN static void
_ecore_audio_out_system_efl_object_destructor(Eo *eo_obj, Ecore_Audio_Out_System_Data *pd)
{
   if (pd->ready_job) ecore_job_del(pd->ready_job);
   if (pd->backend)
     {
#ifdef _WIN32
        efl_event_callback_del(pd->backend, ECORE_AUDIO_OUT_WASAPI_EVENT_CONTEXT_READY,
                               _context_ready_cb, eo_obj);
        efl_event_callback_del(pd->backend, ECORE_AUDIO_OUT_WASAPI_EVENT_CONTEXT_FAIL,
                               _context_fail_cb, eo_obj);
        efl_event_callback_del(pd->backend, ECORE_AUDIO_OUT_WASAPI_EVENT_STOP,
                               _stop_cb, eo_obj);
#elif defined(HAVE_PULSE)
        efl_event_callback_del(pd->backend, ECORE_AUDIO_OUT_PULSE_EVENT_CONTEXT_READY,
                               _context_ready_cb, eo_obj);
        efl_event_callback_del(pd->backend, ECORE_AUDIO_OUT_PULSE_EVENT_CONTEXT_FAIL,
                               _context_fail_cb, eo_obj);
#endif
        efl_unref(pd->backend);
        pd->backend = NULL;
     }
   efl_destructor(efl_super(eo_obj, MY_CLASS));
}

EOLIAN static void
_ecore_audio_out_system_ecore_audio_volume_set(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Out_System_Data *pd, double volume)
{
   ecore_audio_obj_volume_set(pd->backend, volume);
}

EOLIAN static double
_ecore_audio_out_system_ecore_audio_volume_get(const Eo *eo_obj EINA_UNUSED, Ecore_Audio_Out_System_Data *pd)
{
   return ecore_audio_obj_volume_get(pd->backend);
}

EOLIAN static void
_ecore_audio_out_system_ecore_audio_paused_set(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Out_System_Data *pd, Eina_Bool paused)
{
   ecore_audio_obj_paused_set(pd->backend, paused);
}

EOLIAN static Eina_Bool
_ecore_audio_out_system_ecore_audio_paused_get(const Eo *eo_obj EINA_UNUSED, Ecore_Audio_Out_System_Data *pd)
{
   return ecore_audio_obj_paused_get(pd->backend);
}

EOLIAN static Eina_Bool
_ecore_audio_out_system_ecore_audio_out_input_attach(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Out_System_Data *pd, Eo *input)
{
   if (!efl_isa(input, ECORE_AUDIO_IN_CLASS)) return EINA_FALSE;
   return ecore_audio_obj_out_input_attach(pd->backend, input);
}

EOLIAN static Eina_Bool
_ecore_audio_out_system_ecore_audio_out_input_detach(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Out_System_Data *pd, Eo *input)
{
   if (!efl_isa(input, ECORE_AUDIO_IN_CLASS)) return EINA_FALSE;
   return ecore_audio_obj_out_input_detach(pd->backend, input);
}

EOLIAN static Eina_List *
_ecore_audio_out_system_ecore_audio_out_inputs_get(Eo *eo_obj EINA_UNUSED, Ecore_Audio_Out_System_Data *pd)
{
   return ecore_audio_obj_out_inputs_get(pd->backend);
}

#include "ecore_audio_out_system.eo.c"
