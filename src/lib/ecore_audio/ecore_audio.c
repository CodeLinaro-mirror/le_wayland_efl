#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef HAVE_FEATURES_H
#include <features.h>
#endif
#include <ctype.h>
#include <errno.h>

#include "ecore_audio_private.h"

int _ecore_audio_log_dom = -1;
static int _ecore_audio_init_count = 0;

#ifdef HAVE_SNDFILE
Ecore_Audio_Lib_Sndfile *ecore_audio_sndfile_lib = NULL;
#endif /* HAVE_SNDFILE */

/* externally accessible functions */

ECORE_AUDIO_API int
ecore_audio_init(void)
{

   if (++_ecore_audio_init_count != 1)
     return _ecore_audio_init_count;

   if (!ecore_init())
     return --_ecore_audio_init_count;

   if (!efl_object_init()) {
     ecore_shutdown();
     return --_ecore_audio_init_count;
   }

   _ecore_audio_log_dom = eina_log_domain_register("ecore_audio", ECORE_AUDIO_DEFAULT_LOG_COLOR);
   if (_ecore_audio_log_dom < 0)
     {
        EINA_LOG_ERR("Impossible to create a log domain for the ecore audio module.");
        return --_ecore_audio_init_count;
     }

   DBG("Ecore_Audio init");
   eina_log_timing(_ecore_audio_log_dom,
                   EINA_LOG_STATE_STOP,
                   EINA_LOG_STATE_INIT);

   return _ecore_audio_init_count;
}

ECORE_AUDIO_API int
ecore_audio_shutdown(void)
{
   DBG("Ecore_Audio shutdown");
   if (--_ecore_audio_init_count != 0)
     return _ecore_audio_init_count;

#ifdef HAVE_SNDFILE
// explicitly disabled - yes, we know to "fix a leak" you unload here, but
// objects may still exist at this point and may access functions/symbols
// from sndfile
//   ecore_audio_sndfile_lib_unload();
#endif /* HAVE_SNDFILE */

   /* FIXME: Shutdown all the inputs and outputs first */
   eina_log_timing(_ecore_audio_log_dom,
                   EINA_LOG_STATE_START,
                   EINA_LOG_STATE_SHUTDOWN);

   eina_log_domain_unregister(_ecore_audio_log_dom);
   _ecore_audio_log_dom = -1;

   efl_object_shutdown();
   ecore_shutdown();

   return _ecore_audio_init_count;
}

#ifdef HAVE_SNDFILE
Eina_Bool
ecore_audio_sndfile_lib_load(void)
{
   if (ecore_audio_sndfile_lib)
     {
        if (!ecore_audio_sndfile_lib->mod) return EINA_FALSE;
        return EINA_TRUE;
     }

   ecore_audio_sndfile_lib = calloc(1, sizeof(Ecore_Audio_Lib_Sndfile));
   if (!ecore_audio_sndfile_lib) return EINA_FALSE;
# define LOAD(x)                                                 \
   if (!ecore_audio_sndfile_lib->mod) {                          \
      if ((ecore_audio_sndfile_lib->mod = eina_module_new(x))) { \
         if (!eina_module_load(ecore_audio_sndfile_lib->mod)) {  \
            eina_module_free(ecore_audio_sndfile_lib->mod);      \
            ecore_audio_sndfile_lib->mod = NULL;                 \
         }                                                       \
      }                                                          \
   }
# if defined(_WIN32) || defined(__CYGWIN__)
   LOAD("libsndfile-1.dll");
   LOAD("libsndfile.dll");
   LOAD("sndfile.dll");
# elif defined(__APPLE__) && defined(__MACH__)
   LOAD("libsndfile.1.dylib");
   LOAD("libsndfile.1.so");
   LOAD("libsndfile.so.1");
# else
   LOAD("libsndfile.so.1");
# endif
# undef LOAD
   if (!ecore_audio_sndfile_lib->mod) return EINA_FALSE;

#define SYM(x) \
   if (!(ecore_audio_sndfile_lib->x = eina_module_symbol_get(ecore_audio_sndfile_lib->mod, #x))) { \
      ERR("Cannot find symbol '%s' in'%s", #x, eina_module_file_get(ecore_audio_sndfile_lib->mod)); \
      goto err; \
   }
   SYM(sf_open);
   SYM(sf_open_virtual);
   SYM(sf_close);
   SYM(sf_read_float);
   SYM(sf_write_float);
   SYM(sf_write_sync);
   SYM(sf_seek);
   SYM(sf_strerror);
#undef SYM
   return EINA_TRUE;
err:
   if (ecore_audio_sndfile_lib->mod)
     {
        eina_module_free(ecore_audio_sndfile_lib->mod);
        ecore_audio_sndfile_lib->mod = NULL;
        ERR("Cannot find libsndfile at runtime!");
     }
   return EINA_FALSE;
}

void
ecore_audio_sndfile_lib_unload(void)
{
   if (ecore_audio_sndfile_lib)
     {
        if (ecore_audio_sndfile_lib->mod)
          eina_module_free(ecore_audio_sndfile_lib->mod);
        free(ecore_audio_sndfile_lib);
        ecore_audio_sndfile_lib = NULL;
     }
}
#endif /* HAVE_SNDFILE */


ECORE_AUDIO_API const char*
ecore_audio_obj_name_get(const Efl_Object* obj)
{
   return efl_name_get(obj);
}

ECORE_AUDIO_API void
ecore_audio_obj_name_set(Efl_Object* obj, const char *name)
{
   efl_name_set(obj, name);
}


/**
 * @}
 */
