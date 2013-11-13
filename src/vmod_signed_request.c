#include <vmod_signed_request.h>

// VMOD
int init_function(struct vmod_priv *priv, const struct VCL_conf *conf)
{
    return (0);
}

static char* sigreq_strdup(const char* str)
{
      size_t len;
      char* copy;

      len = strlen(str) + 1;
      if (!(copy = (char*)malloc(len))) return 0;
      memcpy(copy,str,len);
      return copy;
}

/* ---- Base64 Encoding/Decoding Table --- */
char b64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* decodeblock - decode 4 '6-bit' characters into 3 8-bit binary bytes */
void decodeblock(unsigned char in[], char *clrstr) {
  unsigned char out[4];
  out[0] = in[0] << 2 | in[1] >> 4;
  out[1] = in[1] << 4 | in[2] >> 2;
  out[2] = in[2] << 6 | in[3] >> 0;
  out[3] = '\0';
  strncat(clrstr, out, sizeof(out));
}

void b64_decode(char *b64src, char *clrdst) {
  int c, phase, i;
  unsigned char in[4];
  char *p;
  int reset = 1;

  clrdst[0] = '\0';
  phase = 0; i=0;
  while(b64src[i]) {
    c = (int) b64src[i];
    if(c == '=') {
      decodeblock(in, clrdst);
      reset = 1;
      break;
    }
    p = strchr(b64, c);
    if(p) {
      in[phase] = p - b64;
      phase = (phase + 1) % 4;
      if(phase == 0) {
        decodeblock(in, clrdst);
        in[0]=in[1]=in[2]=in[3]=0;
        reset = 1;
      } else  {
        reset = 0;
      }
    }
    i++;
  }
  if (!reset) {
    decodeblock(in, clrdst);
  }
}

char* split(const char* string, int n)
{
    char *res = NULL;
    int count = 0;

    res = strtok(strdup(string), ".");
    while(res != NULL && count<n) {
        count = count+1;
        res   = strtok(NULL, ".");
    }
    return res;
}

// Replaces occurrences of from by to
void replace(char* string, char from, char to)
{
    unsigned int n = strlen(string);
    int i;
    for (i=0; i<n; i=i+1)
    {
        if (string[i] == from) {
            string[i] = to;
        }
    }
}

void base64UrlDecode(char* string, char* outstring)
{
    replace(string, '-', '+');
    replace(string, '_', '/');
    b64_decode(string, outstring);
}

const char* getAppData(cJSON* json)
{
    cJSON* app_data = cJSON_GetObjectItem(json, "app_data");
    return app_data ? app_data->valuestring : sigreq_strdup("");
}

const char* getPageId(cJSON* json)
{
    cJSON* page = cJSON_GetObjectItem(json, "page");
    if (page) {
        cJSON* id = cJSON_GetObjectItem(page, "id");
        if (id) {
            return id->valuestring;
        }
    }
    return sigreq_strdup("");
}

const char* getLocale(cJSON* json)
{
    cJSON* user = cJSON_GetObjectItem(json, "user");
    if (user) {
        cJSON* locale = cJSON_GetObjectItem(user, "locale");
        if (locale) {
            return locale->valuestring;
        }
    }
    return sigreq_strdup("en_US");
}

const char* getIsAdmin(cJSON* json)
{
    cJSON* page = cJSON_GetObjectItem(json, "page");
    if (page) {
        cJSON* admin = cJSON_GetObjectItem(page, "admin");
        if (admin) {
            return cJSON_Print(admin);
        }
    }
    return sigreq_strdup("false");
}

const char* getIsLiked(cJSON* json)
{
    cJSON* page = cJSON_GetObjectItem(json, "page");
    if (page) {
        cJSON* liked = cJSON_GetObjectItem(page, "liked");
        if (liked) {
            return cJSON_Print(liked);
        }
    }
    return sigreq_strdup("false");
}

// See https://developers.facebook.com/docs/authentication/signed_request/
// for the steps of decoding the signed request
void vmod_parse(struct sess *sp)
{
    struct w4uSignedRequest *req;
    char buf[64];
    buf[0] = 0;
    char base64decoded[1024] = "";

    const char *signed_request = VRT_GetHdr(sp, HDR_REQ, "\007sigreq:");

    // Allocate memory
    ALLOC_OBJ(req, VMOD_SIGNED_MAGIC);
    AN(req);

    // Storing pointer address for later
    snprintf(buf, 64, "%lu", req);
    VRT_SetHdr(sp, HDR_REQ, SIGNED_REQ_HDR, buf, vrt_magic_string_end);

    const char* app_data  = sigreq_strdup("");
    const char* page_id   = sigreq_strdup("");
    const char* locale    = sigreq_strdup("en_US");
    const char* is_admin  = sigreq_strdup("false");
    const char* is_liked  = sigreq_strdup("false");

    if (signed_request != NULL) {
        // TODO: Finish the check of the sig
        // char *encoded_sig = split(signed_request, 0);
        char *payload = split(signed_request, 1);

        // Ensure payload is not null
        payload = payload ?: "";

        // char *decoded_sig = base64UrlDecode(encoded_sig);
        base64UrlDecode(payload, base64decoded);

        cJSON* json = cJSON_Parse(base64decoded);

        if (json) {
            app_data = getAppData(json);
            page_id  = getPageId(json);
            locale   = getLocale(json);
            is_admin = getIsAdmin(json);
            is_liked = getIsLiked(json);
        }
        cJSON_Delete(json);
    }

    req->app_data = strndup(app_data, strlen(app_data));
    AN(req->app_data);
    req->page_id  = strndup(page_id, strlen(page_id));
    AN(req->page_id);
    req->locale   = strndup(locale, strlen(locale));
    AN(req->locale);
    req->is_admin = strndup(is_admin, strlen(is_admin));
    AN(req->is_admin);
    req->is_liked = strndup(is_liked, strlen(is_liked));
    AN(req->is_liked);
}

////////////////////////////////////////////////////
// Get struct pointer
struct w4uSignedRequest *vmodsigned_get(struct sess *sp)
{
    struct w4uSignedRequest *req;
    req = vmodsigned_get_pointer(sp);
    if(req)
        return req;
    return NULL;
}

// Retrieve struct pointer
// Retrieve struct pointer
struct w4uSignedRequest *vmodsigned_get_pointer(struct sess *sp)
{
    const char *tmp;
    struct w4uSignedRequest *req;

    tmp = VRT_GetHdr(sp, HDR_REQ, SIGNED_REQ_HDR);

    if(tmp){
        req = (struct w4uSignedRequest *)atol(tmp);
        return req;
    }
    return NULL;
}

const char *get_key(struct sess *sp, enum SIGNEDREQ_KEY key)
{
    if(!vmodsigned_get(sp)){
        VRT_panic(sp,"please write \"signed_request.parse();\" to 1st line in vcl_recv.",vrt_magic_string_end);
    }
    struct w4uSignedRequest *req = vmodsigned_get(sp);

    switch (key) {
        case APP_DATA:
            return req->app_data;
        case PAGE_ID:
            return req->page_id;
        case LOCALE:
            return req->locale;
        case IS_ADMIN:
            return req->is_admin;
        case IS_LIKED:
            return req->is_liked;
    }
}

static void vmodreq_free(struct w4uSignedRequest *req)
{
    syslog(6, "[TEST_LOG] {vmod_signed_request}: Calling vmodreq_free");
    CHECK_OBJ_NOTNULL(req, VMOD_SIGNED_MAGIC);
    free(req->app_data);
    free(req->page_id);
    free(req->locale);
    free(req->is_admin);
    free(req->is_liked);
    FREE_OBJ(req);
}

void vmod_free_all(struct sess *sp)
{
    syslog(6, "[TEST_LOG] {vmod_signed_request}: Calling vmod_free_all");
    struct w4uSignedRequest *req;
    req = vmodsigned_get_pointer(sp);
    if(req)
        vmodreq_free(req);
}

const char *vmod_get_app_data(struct sess *sp)
{
    return get_key(sp, APP_DATA);
}

const char *vmod_get_page_id(struct sess *sp)
{
    return get_key(sp, PAGE_ID);
}

const char *vmod_get_locale(struct sess *sp)
{
    return get_key(sp, LOCALE);
}

const char *vmod_get_is_admin(struct sess *sp)
{
    return get_key(sp, IS_ADMIN);
}

const char *vmod_get_is_liked(struct sess *sp)
{
    return get_key(sp, IS_LIKED);
}


