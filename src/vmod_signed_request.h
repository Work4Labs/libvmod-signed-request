#include <stdio.h>
#include <string.h>
#include <cJSON.h>

// Logging/debugging
#include <syslog.h>

// VMOD includes
#include <stdlib.h>

#include "vcl.h"
#include "vrt.h"
#include "bin/varnishd/cache.h"

#include "vcc_if.h"

#define SIGNED_REQ_HDR "\025X-VMOD-SIGNEDREQ-PTR:"

// The structure of the decoded request:
struct w4uSignedRequest {
    unsigned magic;
#define VMOD_SIGNED_MAGIC 0x8b4f21af
    char* app_data;
    char* is_admin;
    char* locale;
    char* page_id;
    char* is_liked;
};

enum SIGNEDREQ_KEY { APP_DATA, IS_ADMIN, LOCALE, PAGE_ID, IS_LIKED };

struct w4uSignedRequest *vmodsigned_get(struct sess *);
struct w4uSignedRequest *vmodsigned_get_pointer(struct sess *);
const char *get_key(struct sess *, enum SIGNEDREQ_KEY key);

static int vmod_Hook_unset_deliver(struct sess *);
static int vmod_Hook_unset_bereq(struct sess *);

void vmod_free_all(struct sess *);
static void vmodreq_free(struct w4uSignedRequest *);
const char *vmod_get_page_id(struct sess *);
const char *vmod_get_locale(struct sess *);

//////////////////////////////////////////
// Hook to free memory
static unsigned           hook_done          = 0;
static vcl_func_f         *vmod_Hook_miss    = NULL;
static vcl_func_f         *vmod_Hook_pass    = NULL;
static vcl_func_f         *vmod_Hook_pipe    = NULL;
static vcl_func_f         *vmod_Hook_deliver = NULL;

static pthread_mutex_t    vmod_mutex = PTHREAD_MUTEX_INITIALIZER;
