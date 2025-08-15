/* Copyright (C) 2025  gemmaro
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mrb_uriparser.h"

#define DONE mrb_gc_arena_restore(mrb, 0);

#define URIPARSER_MODULE "URIParser"
#define URI_CLASSNAME "URI"

#define URIPARSER(mrb) mrb_module_get(mrb, URIPARSER_MODULE)
#define URI(mrb) mrb_class_get_under(mrb, URIPARSER(mrb), URI_CLASSNAME)
#define ERROR(mrb) mrb_class_get_under(mrb, URIPARSER(mrb), "Error")

#define RAISE(mrb, message) mrb_raise(mrb, ERROR(mrb), message)

/**
 * Returns 0 if fails.  Be sure to free str.
 */
static size_t cstr_in_range(mrb_state *const mrb, const UriTextRangeA range,
                            char **const str) {
  if (!range.first || !range.afterLast)
    return 0;
  const size_t len = range.afterLast - range.first;
  if ((*str = realloc(*str, len * sizeof(char))) == NULL) {
    RAISE(mrb, "no space");
  }
  strncpy(*str, range.first, len);
  return len;
}

static mrb_value str_in_range(mrb_state *const mrb, const UriTextRangeA range) {
  char *str = NULL;
  const size_t len = cstr_in_range(mrb, range, &str);
  if (!len) {
    free(str);
    return mrb_nil_value();
  }
  const mrb_value val = mrb_str_new(mrb, str, len);
  free(str);
  return val;
}

static mrb_value int_in_range(mrb_state *const mrb, const UriTextRangeA range) {
  char *str = NULL;
  const size_t len = cstr_in_range(mrb, range, &str);
  if (!len) {
    free(str);
    return mrb_nil_value();
  }
  const mrb_value val = mrb_int_value(mrb, atoi(str));
  free(str);
  return val;
}

static mrb_value path_from_list(mrb_state *const mrb,
                                const UriPathSegmentA *segment) {
  char *str = NULL;
  size_t len = 0;
  while (segment != NULL) {
    char *add = NULL;
    const size_t add_len =
        cstr_in_range(mrb, segment->text, &add) + 1 /* for separator */;
    char *const tmp = realloc(str, len + add_len);
    if (tmp == NULL) {
      free(add);
      RAISE(mrb, "no space");
    }
    str = tmp;
    str[len] = '/';
    str[len + 1] = '\0';
    strcat(str, add);
    free(add);
    len += add_len;

    segment = segment->next;
  }
  if (len == 0) {
    free(str);
    return mrb_nil_value();
  }
  const mrb_value path = mrb_str_new(mrb, str, len);
  free(str);
  return path;
}

#define URI_PARSE_FAILED_MESSAGE "URI parse failed at"

static mrb_value parse(mrb_state *const mrb, const mrb_value self) {
  const char *str = NULL;
  mrb_get_args(mrb, "z", &str);

  UriUriA uri;
  const char *error_pos;
  if (uriParseSingleUriA(&uri, str, &error_pos) != URI_SUCCESS) {
    const size_t len = strlen(URI_PARSE_FAILED_MESSAGE) + strlen(error_pos) +
                       4 /* for punctuations */;
    char *const message = malloc(len);
    if (message == NULL) {
      RAISE(mrb, "no space");
    }
    sprintf(message, "%s: `%s'", URI_PARSE_FAILED_MESSAGE, error_pos);
    RAISE(mrb, message);
  }

  const mrb_value scheme = str_in_range(mrb, uri.scheme);
  const mrb_value userinfo = str_in_range(mrb, uri.userInfo);
  const mrb_value host = str_in_range(mrb, uri.hostText);
  const mrb_value port = int_in_range(mrb, uri.portText);
  const mrb_value path = path_from_list(mrb, uri.pathHead);
  const mrb_value query = str_in_range(mrb, uri.query);
  const mrb_value fragment = str_in_range(mrb, uri.fragment);

  uriFreeUriMembersA(&uri);

  const mrb_value args[] = {scheme, userinfo, host,    port,
                            path,   query,    fragment};
  return mrb_obj_new(mrb, URI(mrb), sizeof(args) / sizeof(mrb_value), args);
}

void mrb_mruby_uriparser_gem_init(mrb_state *const mrb) {
  /* C have to define classes here before Ruby does. */
  struct RClass *const uriparser = mrb_define_module(mrb, URIPARSER_MODULE);
  mrb_define_module_function(mrb, uriparser, "parse", parse, MRB_ARGS_REQ(1));
  DONE;
}

void mrb_mruby_uriparser_gem_final(const mrb_state *const mrb) {}
