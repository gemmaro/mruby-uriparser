/**
 * @file mrb_uriparser.c
 * @brief mruby uriparser implementation
 *
 * Copyright (C) 2025  gemmaro
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

#include <mruby/array.h>
#include <mruby/boxing_word.h>
#include <mruby/data.h>
#include <mruby/value.h>
#include <mruby/variable.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* https://uriparser.github.io/doc/api/latest/ */
#include <uriparser/Uri.h>

#define DONE mrb_gc_arena_restore(mrb, 0);

#define MRB_URIPARSER_MODULE_NAME "URIParser"
#define MRB_URIPARSER_URI_MODULE_NAME "URI"

#define MRB_URIPARSER(mrb) mrb_module_get(mrb, MRB_URIPARSER_MODULE_NAME)
#define MRB_URIPARSER_URI(mrb)                                                 \
  mrb_class_get_under(mrb, MRB_URIPARSER(mrb), MRB_URIPARSER_URI_MODULE_NAME)
#define MRB_URIPARSER_ERROR(mrb)                                               \
  mrb_class_get_under(mrb, MRB_URIPARSER(mrb), "Error")
#define MRB_URIPARSER_NOMEM(mrb) mrb_class_get(mrb, "NoMemoryError")

#define MRB_URIPARSER_RAISE(mrb, message)                                      \
  mrb_raise(mrb, MRB_URIPARSER_ERROR(mrb), message)
#define MRB_URIPARSER_RAISE_NOMEM(mrb, message)                                \
  mrb_raise(mrb, MRB_URIPARSER_NOMEM(mrb), message)

#define MRB_URIPARSER_PARSE_FAILED "URI parse failed at"

typedef struct {
  UriUriA *uri;
} mrb_uriparser_data;

static void mrb_uriparser_free(mrb_state *const mrb, void *const p) {
  mrb_uriparser_data *const data = p;
  uriFreeUriMembersA(data->uri);
  mrb_free(mrb, data);
}

static const struct mrb_data_type mrb_uriparser_data_type = {
    .struct_name = "mrb_uriparser_data_type",
    .dfree = mrb_uriparser_free,
};

/* utilities */

static char *mrb_uriparser_cstr_in_range(mrb_state *const mrb,
                                         const UriTextRangeA range) {
  if (range.first == NULL || range.afterLast == NULL)
    return NULL;
  const size_t len = range.afterLast - range.first;
  char *const str = malloc(len + 1);
  strncpy(str, range.first, len);
  str[len] = '\0';
  return str;
}

static mrb_value mrb_uriparser_str_in_range(mrb_state *const mrb,
                                            const UriTextRangeA range) {
  char *const str = mrb_uriparser_cstr_in_range(mrb, range);
  if (str == NULL)
    return mrb_nil_value();
  const mrb_value val = mrb_str_new_cstr(mrb, str);
  free(str);
  return val;
}

static mrb_value mrb_uriparser_int_in_range(mrb_state *const mrb,
                                            const UriTextRangeA range) {
  char *const str = mrb_uriparser_cstr_in_range(mrb, range);
  if (str == NULL)
    return mrb_nil_value();
  const mrb_value val = mrb_int_value(mrb, atoi(str));
  free(str);
  return val;
}

static mrb_value mrb_uriparser_new(mrb_state *const mrb, UriUriA *uri) {
  const mrb_value value = mrb_obj_new(mrb, MRB_URIPARSER_URI(mrb), 0, NULL);
  DATA_TYPE(value) = &mrb_uriparser_data_type;
  mrb_uriparser_data *const data = mrb_malloc(mrb, sizeof(mrb_uriparser_data));
  data->uri = uri;
  DATA_PTR(value) = data;
  return value;
}

/* initialized functions */

/**
 * @brief Parse string into URI
 *
 * ```ruby
 * URIParser.parse(str) #=> kind of URIParser::URI
 * ```
 *
 * or
 *
 * ```ruby
 * URIParser::URI.parse(str) #=> kind of URIParser::URI
 * ```
 *
 * @sa mrb_uriparser_recompose
 */
static mrb_value mrb_uriparser_parse(mrb_state *const mrb,
                                     const mrb_value self) {
  const char *str = NULL;
  mrb_get_args(mrb, "z", &str);

  UriUriA *const uri = mrb_malloc(mrb, sizeof(UriUriA));
  const char *error_pos;
  if (uriParseSingleUriA(uri, str, &error_pos) != URI_SUCCESS) {
    const size_t len = strlen(MRB_URIPARSER_PARSE_FAILED) + strlen(error_pos) +
                       5 /* for punctuations and null */;
    char *const message = malloc(len);
    if (message == NULL)
      MRB_URIPARSER_RAISE_NOMEM(mrb, "no space for error message");
    sprintf(message, "%s: `%s'", MRB_URIPARSER_PARSE_FAILED, error_pos);
    MRB_URIPARSER_RAISE(mrb, message);
  }
  return mrb_uriparser_new(mrb, uri);
}

static mrb_value mrb_uriparser_filename_to_uri_string(mrb_state *const mrb,
                                                      const mrb_value self) {
  const char *abs_filename = NULL;
  const mrb_int kw_num = 1;
  const mrb_sym windows_key = mrb_intern_lit(mrb, "windows");
  const mrb_sym *const kw_table = {&windows_key};
  mrb_value kw_values[kw_num];
  const mrb_kwargs kwargs = {.num = kw_num,
                             .required = 0,
                             .rest = NULL,
                             .table = kw_table,
                             .values = kw_values};
  mrb_get_args(mrb, "z:", &abs_filename, &kwargs);
  if (mrb_undef_p(kw_values[0]))
    kw_values[0] = mrb_false_value();
  const mrb_value windows_value = kw_values[0];
  const mrb_bool windows = mrb_test(windows_value);
  const int bytes_needed =
      (windows ? 8 : 7 /* Unix */) + 3 * strlen(abs_filename) + 1;
  char *const abs_uri = malloc(bytes_needed * sizeof(char));
  if ((windows ? uriWindowsFilenameToUriStringA(abs_filename, abs_uri)
               : uriUnixFilenameToUriStringA(abs_filename, abs_uri)) !=
      URI_SUCCESS) {
    free(abs_uri);
    MRB_URIPARSER_RAISE(mrb, "failed to convert to URI");
  }
  const mrb_value uri = mrb_str_new_cstr(mrb, abs_uri);
  free(abs_uri);
  return uri;
}

/**
 * @brief Get scheme string
 *
 * ```ruby
 * uri.scheme
 * ```
 *
 * where `uri` is `URIParser::URI`.
 */
static mrb_value mrb_uriparser_scheme(mrb_state *const mrb,
                                      const mrb_value self) {
  return mrb_uriparser_str_in_range(
      mrb, ((mrb_uriparser_data *)DATA_PTR(self))->uri->scheme);
}

/**
 * @brief Get userinfo string
 *
 * ```ruby
 * uri.userinfo
 * ```
 *
 * where `uri` is `URIParser::URI`.
 */
static mrb_value mrb_uriparser_userinfo(mrb_state *const mrb,
                                        const mrb_value self) {
  return mrb_uriparser_str_in_range(
      mrb, ((mrb_uriparser_data *)DATA_PTR(self))->uri->userInfo);
}

/**
 * @brief Get host string
 *
 * ```ruby
 * uri.host
 * ```
 *
 * where `uri` is `URIParser::URI`.
 */
static mrb_value mrb_uriparser_host(mrb_state *const mrb,
                                    const mrb_value self) {
  return mrb_uriparser_str_in_range(
      mrb, ((mrb_uriparser_data *)DATA_PTR(self))->uri->hostText);
}

/**
 * @brief Get port as integer
 *
 * ```ruby
 * uri.port
 * ```
 *
 * where `uri` is `URIParser::URI`.
 */
static mrb_value mrb_uriparser_port(mrb_state *const mrb,
                                    const mrb_value self) {
  return mrb_uriparser_int_in_range(
      mrb, ((mrb_uriparser_data *)DATA_PTR(self))->uri->portText);
}

/**
 * @brief Get path segments as array of strings
 *
 * ```ruby
 * uri.path_segments
 * ```
 *
 * where `uri` is `URIParser::URI`.
 */
static mrb_value mrb_uriparser_path_segments(mrb_state *const mrb,
                                             const mrb_value self) {
  UriPathSegmentA *segment =
      ((mrb_uriparser_data *)DATA_PTR(self))->uri->pathHead;
  mrb_value ary = mrb_ary_new(mrb);
  while (segment != NULL) {
    mrb_ary_push(mrb, ary, mrb_uriparser_str_in_range(mrb, segment->text));
    segment = segment->next;
  }
  return ary;
}

/**
 * @brief Get query string
 *
 * ```ruby
 * uri.query
 * ```
 *
 * where `uri` is `URIParser::URI`.
 */
static mrb_value mrb_uriparser_query(mrb_state *const mrb,
                                     const mrb_value self) {
  return mrb_uriparser_str_in_range(
      mrb, ((mrb_uriparser_data *)DATA_PTR(self))->uri->query);
}

/**
 * @brief Get fragment string
 *
 * ```ruby
 * uri.fragment
 * ```
 *
 * where `uri` is `URIParser::URI`.
 */
static mrb_value mrb_uriparser_fragment(mrb_state *const mrb,
                                        const mrb_value self) {
  return mrb_uriparser_str_in_range(
      mrb, ((mrb_uriparser_data *)DATA_PTR(self))->uri->fragment);
}

/**
 * @brief Check if absolute path
 *
 * ```ruby
 * uri.absolute_path?
 * ```
 *
 * where `uri` is `URIParser::URI`.
 */
static mrb_value mrb_uriparser_absolute_path(mrb_state *const mrb,
                                             const mrb_value self) {
  return mrb_bool_value(
      ((mrb_uriparser_data *)DATA_PTR(self))->uri->absolutePath);
}

/**
 * @brief Recomposing URI
 *
 * Recomposing means serializing.
 *
 * ```ruby
 * uri.to_s
 * ```
 *
 * where `uri` is `URIParser::URI`.
 *
 * @sa mrb_uriparser_parse
 */
static mrb_value mrb_uriparser_recompose(mrb_state *const mrb,
                                         const mrb_value self) {
  const mrb_uriparser_data *const data = DATA_PTR(self);
  int chars_required;
  if (uriToStringCharsRequiredA(data->uri, &chars_required) != URI_SUCCESS)
    MRB_URIPARSER_RAISE(mrb, "could not calculate chars required");
  chars_required++; /* zero terminator */
  char *const uri_string = malloc(chars_required * sizeof(char));
  if (uri_string == NULL)
    MRB_URIPARSER_RAISE_NOMEM(mrb, "no space for URI string");
  if (uriToStringA(uri_string, data->uri, chars_required, NULL) != URI_SUCCESS)
    MRB_URIPARSER_RAISE(mrb, "URI recomposing failed");
  return mrb_str_new_cstr(mrb, uri_string);
}

/**
 * @brief Resolve references mutably
 *
 * ```ruby
 * uri.merge!(rel) #=> resolved URI
 * uri             #=> same
 * ```
 *
 * where `uri` and `rel` are kind of `URIParser::URI`.
 *
 * @sa mrb_uriparser_merge
 */
static mrb_value mrb_uriparser_merge_mutably(mrb_state *const mrb,
                                             const mrb_value self) {
  const mrb_value rel;
  mrb_get_args(mrb, "o", &rel);
  if (!mrb_obj_is_kind_of(mrb, rel, MRB_URIPARSER_URI(mrb)))
    MRB_URIPARSER_RAISE(mrb, "relative URI is expected to be URIParser::URI");
  const mrb_uriparser_data *const rel_data = DATA_PTR(rel);
  UriUriA *const resolved = mrb_malloc(mrb, sizeof(UriUriA));
  mrb_uriparser_data *const data = DATA_PTR(self);
  if (uriAddBaseUriA(resolved, rel_data->uri, data->uri) != URI_SUCCESS)
    MRB_URIPARSER_RAISE(mrb, "failed to resolve URI");
  mrb_free(mrb, data->uri);
  data->uri = resolved;
  return self;
}

/**
 * @brief Resolve references immutably
 *
 * ```ruby
 * uri.merge(rel) #=> resolved URI
 * uri            #=> original (not resolved)
 * ```
 *
 * or
 *
 * ```ruby
 * uri + rel
 * ```
 *
 * where `uri` and `rel` are kind of `URIParser::URI`.
 *
 * @sa mrb_uriparser_merge_mutably
 * @sa mrb_uriparser_create_reference
 */
static mrb_value mrb_uriparser_merge(mrb_state *const mrb,
                                     const mrb_value self) {
  const mrb_value rel;
  mrb_get_args(mrb, "o", &rel);
  if (!mrb_obj_is_kind_of(mrb, rel, MRB_URIPARSER_URI(mrb)))
    MRB_URIPARSER_RAISE(mrb, "relative URI is expected to be URIParser::URI");
  const mrb_uriparser_data *const rel_data = DATA_PTR(rel);
  UriUriA *const resolved = mrb_malloc(mrb, sizeof(UriUriA));
  mrb_uriparser_data *const data = DATA_PTR(self);
  if (uriAddBaseUriA(resolved, rel_data->uri, data->uri) != URI_SUCCESS)
    MRB_URIPARSER_RAISE(mrb, "failed to resolve URI");
  return mrb_uriparser_new(mrb, resolved);
}

/**
 * @brief Create references
 *
 * ```ruby
 * uri.route_from(base, domain_root: false) #=> relative to the base URI
 * ```
 *
 * or
 *
 * ```ruby
 * uri - base
 * ```
 *
 * where `uri` and `base` are kind of `URIParser::URI`.
 * `domain_root` makes the result relative URI from domain root.
 *
 * There is also
 *
 * ```ruby
 * uri.base_to(dest) #=> relative to destination URI
 * ```
 *
 * @sa mrb_uriparser_merge
 */
static mrb_value mrb_uriparser_create_reference(mrb_state *const mrb,
                                                const mrb_value self) {
  const mrb_value base;
  const mrb_int num = 1;
  const mrb_sym domain_root_key = mrb_intern_lit(mrb, "domain_root");
  const mrb_sym *const table = {&domain_root_key};
  mrb_value values[num];
  const mrb_kwargs kwargs = {.num = num,
                             .required = 0,
                             .rest = NULL,
                             .table = table,
                             .values = values};
  mrb_get_args(mrb, "o:", &base, &kwargs);
  if (mrb_undef_p(values[0]))
    values[0] = mrb_false_value();
  const mrb_value domain_root = values[0];
  if (!mrb_obj_is_kind_of(mrb, base, MRB_URIPARSER_URI(mrb)))
    MRB_URIPARSER_RAISE(mrb, "base URI is expected to be URIParser::URI");
  const mrb_uriparser_data *const base_data = DATA_PTR(base);
  const mrb_uriparser_data *const data = DATA_PTR(self);
  UriUriA *const dest = mrb_malloc(mrb, sizeof(UriUriA));
  if (uriRemoveBaseUriA(dest, data->uri, base_data->uri,
                        mrb_test(domain_root) ? URI_TRUE : URI_FALSE) !=
      URI_SUCCESS)
    MRB_URIPARSER_RAISE(mrb, "failed to remove base URI");
  return mrb_uriparser_new(mrb, dest);
}

void mrb_mruby_uriparser_gem_init(mrb_state *const mrb) {
  /* C have to define classes here before Ruby does. */
  struct RClass *const uriparser =
      mrb_define_module(mrb, MRB_URIPARSER_MODULE_NAME);
  mrb_define_module_function(mrb, uriparser, "parse", mrb_uriparser_parse,
                             MRB_ARGS_REQ(1));
  mrb_define_module_function(mrb, uriparser, "filename_to_uri_string",
                             mrb_uriparser_filename_to_uri_string,
                             MRB_ARGS_ANY());
  struct RClass *const uri = mrb_define_class_under(
      mrb, uriparser, MRB_URIPARSER_URI_MODULE_NAME, mrb->object_class);
  mrb_define_class_method(mrb, uri, "parse", mrb_uriparser_parse,
                          MRB_ARGS_REQ(1));
  mrb_define_method(mrb, uri, "scheme", mrb_uriparser_scheme, MRB_ARGS_NONE());
  mrb_define_method(mrb, uri, "userinfo", mrb_uriparser_userinfo,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, uri, "host", mrb_uriparser_host, MRB_ARGS_NONE());
  mrb_define_method(mrb, uri, "port", mrb_uriparser_port, MRB_ARGS_NONE());
  mrb_define_method(mrb, uri, "path_segments", mrb_uriparser_path_segments,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, uri, "query", mrb_uriparser_query, MRB_ARGS_NONE());
  mrb_define_method(mrb, uri, "fragment", mrb_uriparser_fragment,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, uri, "absolute_path?", mrb_uriparser_absolute_path,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, uri, "to_s", mrb_uriparser_recompose, MRB_ARGS_NONE());
  mrb_define_method(mrb, uri, "merge!", mrb_uriparser_merge_mutably,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, uri, "merge", mrb_uriparser_merge, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, uri, "route_from", mrb_uriparser_create_reference,
                    MRB_ARGS_REQ(1));
  DONE;
}

void mrb_mruby_uriparser_gem_final(const mrb_state *const mrb) {}
