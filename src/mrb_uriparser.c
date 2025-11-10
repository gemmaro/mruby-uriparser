/**
 * @file mrb_uriparser.c
 * @brief mruby uriparser implementation
 *
 * # mruby uriparser
 *
 * ## Other Functions
 *
 * ```ruby
 * URIParser.join(uri_str, *path)
 * URIParser::URI.join(uri_str, *path)
 * ```
 *
 * where `uri` is kind of `URIParser::URI`.
 *
 * ## Planned Functions
 *
 * ```ruby
 * URIParser.split(uri)
 * URIParser::URI.split(uri)
 * uri.hierarchical?
 * ```
 *
 * where `uri` is kind of `URIParser::URI`.
 *
 * ## Comparison of Supported Features
 *
 * Below is a comparison of supported features with CRuby's URI gem.
 * A dash (`-`) indicates not supported or no plans to support.
 *
 * @htmlinclude comparison.html
 *
 * ## License
 *
 * Copyright (C) 2025  gemmaro
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "mrb_uriparser.h"

#include <mruby/array.h>
#include <mruby/boxing_word.h>
#include <mruby/data.h>
#include <mruby/hash.h>
#include <mruby/string.h>
#include <mruby/value.h>
#include <mruby/variable.h>

#include <stdbool.h>
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
#define MRB_URIPARSER_URI_CLASS(mrb)                                           \
  mrb_class_get_under(mrb, MRB_URIPARSER(mrb), MRB_URIPARSER_URI_MODULE_NAME)

/** @brief Error class.
 *
 * ```ruby
 * URIParser::Error
 * ```
 */
#define MRB_URIPARSER_ERROR(mrb)                                               \
  mrb_class_get_under(mrb, MRB_URIPARSER(mrb), "Error")

/** @brief No memory error class.
 *
 * ```ruby
 * URIParser::NoMemoryError
 * ```
 */
#define MRB_URIPARSER_NOMEM(mrb) mrb_class_get(mrb, "NoMemoryError")

#define MRB_URIPARSER_RAISE(mrb, message)                                      \
  mrb_raise(mrb, MRB_URIPARSER_ERROR(mrb), message)
#define MRB_URIPARSER_RAISE_NOMEM(mrb, message)                                \
  mrb_raise(mrb, MRB_URIPARSER_NOMEM(mrb), message)

#define MRB_URIPARSER_PARSE_FAILED "URI parse failed at"

#define MRB_URIPARSER_STR_IN_RANGE(mrb, range, field)                          \
  (!range->field.afterLast || !range->field.first)                             \
      ? mrb_nil_value()                                                        \
      : mrb_str_new(mrb, range->field.first,                                   \
                    range->field.afterLast - range->field.first)

#define MRB_URIPARSER_URI(value) ((mrb_uriparser_data *)DATA_PTR(value))->uri

#define MRB_URIPARSER_NEW(mrb, uri_val)                                        \
  const mrb_value value =                                                      \
      mrb_obj_new(mrb, MRB_URIPARSER_URI_CLASS(mrb), 0, NULL);                 \
  DATA_TYPE(value) = &mrb_uriparser_data_type;                                 \
  mrb_uriparser_data *const data =                                             \
      mrb_malloc(mrb, sizeof(mrb_uriparser_data));                             \
  data->uri = uri_val;                                                         \
  DATA_PTR(value) = data;                                                      \
  return value;

/**
 * Get the specific component of the URI.
 *
 * ```ruby
 * uri.scheme
 * uri.userinfo
 * uri.hostname
 * uri.port
 * uri.query
 * uri.fragment
 * ```
 *
 * where `uri` is a `URIParser::URI` instance.
 *
 * Note for hostname method: it returns `::1` for `http://[::1]/bar`
 * so it corresponds to CRuby's URI gem's `URI::Generic#hostname`
 * method.
 *
 * @return String of the component or `nil`.
 * @sa mrb_uriparser_path_segments
 */
#define MRB_URIPARSER_DEFUN_GETTER(component)                                  \
  static mrb_value mrb_uriparser_##component(mrb_state *const mrb,             \
                                             const mrb_value self) {           \
    return MRB_URIPARSER_STR_IN_RANGE(mrb, MRB_URIPARSER_URI(self),            \
                                      component);                              \
  }

/**
 * @brief Set the scheme component of the URI.
 *
 * ```ruby
 * uri.scheme = ...
 * uri.userinfo = ...
 * uri.host = ...
 * ```
 *
 * where `uri` is a `URIParser::URI` instance.
 *
 * @return `nil`.
 */
#define MRB_URIPARSER_DEFUN_SETTER(component_name)                             \
  static mrb_value mrb_uriparser_set_##component_name(mrb_state *mrb,          \
                                                      mrb_value self) {        \
    char *component;                                                           \
    mrb_get_args(mrb, "z", &component);                                        \
    if (uriSet##component_name##A(MRB_URIPARSER_URI(self), component,          \
                                  component + strlen(component)))              \
      MRB_URIPARSER_RAISE(mrb, "failed to set " #component_name);              \
    return mrb_nil_value();                                                    \
  }

/**
 * @brief Internal data structure for wrapping a `UriUriA` pointer in mruby.
 *
 * This structure is used to associate a parsed URI (represented by a
 * `UriUriA` pointer) with an mruby object.  It enables integration
 * between the uriparser C library and mruby's object system via the
 * `DATA_PTR` mechanism.
 */
typedef struct {
  /**
   * Pointer to a `UriUriA` structure representing the parsed URI.
   */
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

/* initialized functions */

/**
 * @brief Parse a string into a URI object.
 *
 * ```ruby
 * URIParser.parse(str)
 * URIParser::URI.parse(str)
 * ```
 *
 * where `str` is a URI string to parse.
 *
 * @return `URIParser::URI` instance.
 * @sa mrb_uriparser_recompose
 */
static mrb_value mrb_uriparser_parse(mrb_state *const mrb,
                                     const mrb_value self) {
  const char *str = NULL;
  mrb_get_args(mrb, "z", &str);

  UriUriA *const uri = mrb_malloc(mrb, sizeof(UriUriA));
  const char *error_pos;
  if (uriParseSingleUriA(uri, str, &error_pos) != URI_SUCCESS) {
    char *const message =
        malloc((strlen(MRB_URIPARSER_PARSE_FAILED) + strlen(error_pos) +
                5 /* for punctuations and null */) *
               sizeof(char));
    if (!message)
      MRB_URIPARSER_RAISE_NOMEM(mrb, "no space for error message");
    sprintf(message, "%s: `%s'", MRB_URIPARSER_PARSE_FAILED, error_pos);
    MRB_URIPARSER_RAISE(mrb, message);
  }
  MRB_URIPARSER_NEW(mrb, uri);
}

/**
 * @brief Convert a filename to a URI string.
 *
 * ```ruby
 * URIParser.filename_to_uri_string(filename, windows: false)
 * URIParser::URI.from_filename(filename, windows: false)
 * ```
 *
 * where `filename` is an absolute filename.  If `windows` is true, use
 * Windows path conversion.
 *
 * @return URI string.
 * @sa mrb_uriparser_uri_string_to_filename
 */
static mrb_value mrb_uriparser_filename_to_uri_string(mrb_state *const mrb,
                                                      const mrb_value self) {
  const char *abs_filename = NULL;
  const mrb_int kw_num = 1;
  const mrb_sym windows_key = mrb_intern_lit(mrb, "windows");
  mrb_value kw_values[kw_num];
  const mrb_kwargs kwargs = {.num = kw_num,
                             .required = 0,
                             .rest = NULL,
                             .table = &windows_key,
                             .values = kw_values};
  mrb_get_args(mrb, "z:", &abs_filename, &kwargs);
  if (mrb_undef_p(kw_values[0]))
    kw_values[0] = mrb_false_value();
  const mrb_bool windows = mrb_test(kw_values[0]);
  char *const abs_uri =
      malloc(((windows ? 8 : 7 /* Unix */) + 3 * strlen(abs_filename) + 1) *
             sizeof(char));
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
 * @brief Convert a URI string to a filename.
 *
 * ```ruby
 * URIParser.uri_string_to_filename(uri, windows: false)
 * ```
 *
 * where `uri` is a URI string.  If `windows` is true, use Windows path
 * conversion.
 *
 * ```ruby
 * uri.to_filename(windows: false)
 * ```
 *
 * where `uri` is kind of `URIParser::URI`.
 *
 * @return Filename string.
 * @sa mrb_uriparser_filename_to_uri_string
 */
static mrb_value mrb_uriparser_uri_string_to_filename(mrb_state *const mrb,
                                                      const mrb_value self) {
  const char *abs_uri = NULL;
  const mrb_int kw_num = 1;
  const mrb_sym windows_key = mrb_intern_lit(mrb, "windows");
  mrb_value kw_values[kw_num];
  const mrb_kwargs kwargs = {.num = kw_num,
                             .required = 0,
                             .rest = NULL,
                             .table = &windows_key,
                             .values = kw_values};
  mrb_get_args(mrb, "z:", &abs_uri, &kwargs);
  if (mrb_undef_p(kw_values[0]))
    kw_values[0] = mrb_false_value();
  const mrb_bool windows = mrb_test(kw_values[0]);
  char *const abs_filename = malloc(
      (strlen(abs_uri) + 1 - (windows ? 8 : 7 /* Unix */)) * sizeof(char));
  if ((windows ? uriUriStringToWindowsFilenameA(abs_uri, abs_filename)
               : uriUriStringToUnixFilenameA(abs_uri, abs_filename)) !=
      URI_SUCCESS) {
    free(abs_filename);
    MRB_URIPARSER_RAISE(mrb, "failed to convert to filename");
  }
  const mrb_value filename = mrb_str_new_cstr(mrb, abs_filename);
  free(abs_filename);
  return filename;
}

/**
 * @brief Encode an array of key-value pairs as a WWW form query string.
 *
 * ```ruby
 * URIParser.encode_www_form(query_list)
 * ```
 *
 * where `query_list` is `Array` of `[key, value]` pairs.  Key is `String`.
 * Value may be `nil` or `String`.
 *
 * @return Encoded query string.
 * @sa mrb_uriparser_dissect_query
 *
 * No `enc=nil` parameter as seen in CRuby's URI gem.
 */
static mrb_value mrb_uriparser_compose_query(mrb_state *const mrb,
                                             const mrb_value self) {
  UriQueryListA *query_list = NULL;
  mrb_value ary;
  mrb_get_args(mrb, "A", &ary);
  for (mrb_int index = RARRAY_LEN(ary) - 1; index >= 0; index--) {
    UriQueryListA *current = malloc(sizeof(UriQueryListA));
    current->next = query_list;
    mrb_value entry = mrb_ary_ref(mrb, ary, index);
    current->key = mrb_str_to_cstr(mrb, mrb_ary_ref(mrb, entry, 0));

    mrb_value value = mrb_ary_ref(mrb, entry, 1);
    current->value = mrb_nil_p(value) ? NULL : mrb_str_to_cstr(mrb, value);

    query_list = current;
  }
  int chars_required;
  char *query_string;
  if (uriComposeQueryCharsRequiredA(query_list, &chars_required) != URI_SUCCESS)
    MRB_URIPARSER_RAISE(
        mrb, "failed to calculate characters required to compose query");
  query_string = malloc((chars_required + 1) * sizeof(char));
  if (!query_string)
    MRB_URIPARSER_RAISE_NOMEM(mrb, "no space for query string");
  int chars_written;
  if (uriComposeQueryA(query_string, query_list, chars_required + 1,
                       &chars_written) != URI_SUCCESS)
    MRB_URIPARSER_RAISE(mrb, "failed to compose query");
  mrb_value str = mrb_str_new(mrb, query_string, chars_written - 1);
  free(query_string);
  return str;
}

#ifdef HAVE_URI_COPY_URI
/**
 * @brief Copy URI.
 *
 * ```ruby
 * uri.dup
 * uri.clone
 * ```
 *
 * where `uri` is a `URIParser::URI` instance.
 *
 * @return New URI.
 */
static mrb_value mrb_uriparser_initialize_copy(mrb_state *mrb, mrb_value self) {
  mrb_value original;
  mrb_get_args(mrb, "o", &original);
  UriUriA *uri = MRB_URIPARSER_URI(original);
  UriUriA *new_uri = mrb_malloc(mrb, sizeof(UriUriA));
  if (uriCopyUriA(new_uri, uri)) {
    MRB_URIPARSER_RAISE(mrb, "failed to copy URI");
  }
  DATA_TYPE(self) = &mrb_uriparser_data_type;
  mrb_uriparser_data *data = mrb_malloc(mrb, sizeof(mrb_uriparser_data));
  data->uri = new_uri;
  DATA_PTR(self) = data;
  return self;
}
#endif

#ifdef HAVE_URI_EQUALS_URI
/**
 * @brief Check two URIs for equivalence.
 *
 * ```ruby
 * uri == another_uri
 * ```
 *
 * where `uri` and `another_uri` are `URIParser::URI` instances.
 *
 * @return Boolean.
 */
static mrb_value mrb_uriparser_equals(mrb_state *mrb, mrb_value self) {
  mrb_value another;
  mrb_get_args(mrb, "o", &another);
  UriBool result =
      uriEqualsUriA(MRB_URIPARSER_URI(self), MRB_URIPARSER_URI(another));
  return mrb_bool_value(result);
}
#endif

MRB_URIPARSER_DEFUN_GETTER(scheme);
MRB_URIPARSER_DEFUN_GETTER(userInfo);
MRB_URIPARSER_DEFUN_GETTER(hostText);
MRB_URIPARSER_DEFUN_GETTER(portText);
MRB_URIPARSER_DEFUN_GETTER(query);
MRB_URIPARSER_DEFUN_GETTER(fragment);

#ifdef HAVE_URI_SET_SCHEME
MRB_URIPARSER_DEFUN_SETTER(Scheme);
#endif

#ifdef HAVE_URI_SET_USERINFO
MRB_URIPARSER_DEFUN_SETTER(UserInfo);
#endif

#ifdef HAVE_URI_SET_HOST
MRB_URIPARSER_DEFUN_SETTER(HostAuto)
#endif

#ifdef HAVE_URI_HAS_HOST
/**
 * @brief Check if the URI has host.
 *
 * ```ruby
 * uri.host?
 * ```
 *
 * where `uri` is a `URIParser::URI` instance.
 *
 * @return Boolean.
 */
static mrb_value mrb_uriparser_has_host(mrb_state *mrb, mrb_value self) {
  return mrb_bool_value(uriHasHostA(MRB_URIPARSER_URI(self)));
}
#endif

#ifdef HAVE_URI_SET_PORT
/**
 * @brief Set the port component of the URI.
 * ```ruby
 * uri.port = ...
 * ```
 *
 * where `uri` is a `URIParser::URI` instance.
 *
 * @return `nil`.
 */
static mrb_value mrb_uriparser_set_port(mrb_state *mrb, mrb_value self) {
  char *component;
  mrb_get_args(mrb, "z", &component);
  if (uriSetPortTextA(MRB_URIPARSER_URI(self), component,
                      component + strlen(component)))
    MRB_URIPARSER_RAISE(mrb, "failed to set port");
  return mrb_nil_value();
}
#endif

/**
 * @brief Get the path segments as an array of strings.
 *
 * ```ruby
 * uri.path_segments
 * ```
 *
 * where `uri` is a `URIParser::URI` instance.
 *
 * @return Array of path segment strings.
 * @sa MRB_URIPARSER_DEFUN_GETTER
 */
static mrb_value mrb_uriparser_path_segments(mrb_state *const mrb,
                                             const mrb_value self) {
  UriPathSegmentA *segment = MRB_URIPARSER_URI(self)->pathHead;
  mrb_value ary = mrb_ary_new(mrb);
  while (segment) {
    mrb_ary_push(mrb, ary, MRB_URIPARSER_STR_IN_RANGE(mrb, segment, text));
    segment = segment->next;
  }
  return ary;
}

#ifdef HAVE_URI_SET_PATH
/**
 * @brief Set the path component of the URI.
 * ```ruby
 * uri.path = ...
 * ```
 *
 * where `uri` is a `URIParser::URI` instance.
 *
 * @return `nil`.
 */
static mrb_value mrb_uriparser_set_path(mrb_state *mrb, mrb_value self) {
  char *component;
  mrb_get_args(mrb, "z", &component);
  if (uriSetPathA(MRB_URIPARSER_URI(self), component,
                  component + strlen(component)))
    MRB_URIPARSER_RAISE(mrb, "failed to set path");
  return mrb_nil_value();
}
#endif

#ifdef HAVE_URI_SET_QUERY
/**
 * @brief Set the query component of the URI.
 * ```ruby
 * uri.query = ...
 * ```
 *
 * where `uri` is a `URIParser::URI` instance.
 *
 * @return `nil`.
 */
static mrb_value mrb_uriparser_set_query(mrb_state *mrb, mrb_value self) {
  char *component;
  mrb_get_args(mrb, "z", &component);
  if (uriSetQueryA(MRB_URIPARSER_URI(self), component,
                   component + strlen(component)))
    MRB_URIPARSER_RAISE(mrb, "failed to set query");
  return mrb_nil_value();
}
#endif

#ifdef HAVE_URI_SET_FRAGMENT
/**
 * @brief Set the fragment component of the URI.
 * ```ruby
 * uri.fragment = ...
 * ```
 *
 * where `uri` is a `URIParser::URI` instance.
 *
 * @return `nil`.
 */
static mrb_value mrb_uriparser_set_fragment(mrb_state *mrb, mrb_value self) {
  char *component;
  mrb_get_args(mrb, "z", &component);
  if (uriSetFragmentA(MRB_URIPARSER_URI(self), component,
                      component + strlen(component)))
    MRB_URIPARSER_RAISE(mrb, "failed to set fragment");
  return mrb_nil_value();
}
#endif

/**
 * @brief Check if the URI has an absolute path.
 *
 * ```ruby
 * uri.absolute_path?
 * uri.absolute?
 * uri.absolute
 * uri.relative?
 * ```
 *
 * where `uri` is a `URIParser::URI` instance.
 *
 * @return Boolean.
 */
static mrb_value mrb_uriparser_absolute_path(mrb_state *const mrb,
                                             const mrb_value self) {
  return mrb_bool_value(MRB_URIPARSER_URI(self)->absolutePath);
}

/**
 * @brief Serialize the URI to a string.
 *
 * ```ruby
 * uri.to_s
 * ```
 *
 * where `uri` is a `URIParser::URI` instance.
 *
 * @return URI string.
 * @sa mrb_uriparser_parse
 *
 * Recomposing means serializing.
 */
static mrb_value mrb_uriparser_recompose(mrb_state *const mrb,
                                         const mrb_value self) {
  const mrb_uriparser_data *const data = DATA_PTR(self);
  int chars_required;
  if (uriToStringCharsRequiredA(data->uri, &chars_required) != URI_SUCCESS)
    MRB_URIPARSER_RAISE(mrb, "could not calculate chars required");
  chars_required++; /* zero terminator */
  char *const uri_string = malloc(chars_required * sizeof(char));
  if (!uri_string)
    MRB_URIPARSER_RAISE_NOMEM(mrb, "no space for URI string");
  if (uriToStringA(uri_string, data->uri, chars_required, NULL) != URI_SUCCESS)
    MRB_URIPARSER_RAISE(mrb, "URI recomposing failed");
  return mrb_str_new_cstr(mrb, uri_string);
}

/**
 * @brief Mutably resolve a relative URI reference.
 *
 * ```ruby
 * uri.merge!(rel)
 * ```
 *
 * where `uri` is a `URIParser::URI` instance, and `rel` is relative URI
 * (`URIParser::URI`).
 *
 * @return Modified `URIParser::URI` instance.
 * @sa mrb_uriparser_merge
 */
static mrb_value mrb_uriparser_merge_mutably(mrb_state *const mrb,
                                             const mrb_value self) {
  const mrb_value rel;
  mrb_get_args(mrb, "o", &rel);

  /* TODO: Accept string as well. */
  if (!mrb_obj_is_kind_of(mrb, rel, MRB_URIPARSER_URI_CLASS(mrb)))
    MRB_URIPARSER_RAISE(mrb, "relative URI is expected to be URIParser::URI");

  UriUriA *const resolved = mrb_malloc(mrb, sizeof(UriUriA));
  mrb_uriparser_data *const data = DATA_PTR(self);
  if (uriAddBaseUriA(resolved, MRB_URIPARSER_URI(rel), data->uri) !=
      URI_SUCCESS)
    MRB_URIPARSER_RAISE(mrb, "failed to resolve URI");
  mrb_free(mrb, data->uri);
  data->uri = resolved;
  return self;
}

/**
 * @brief Immutably resolve a relative URI reference.
 *
 * ```ruby
 * uri.merge(rel)
 * uri + rel
 * ```
 *
 * where `uri` is a `URIParser::URI` instance, and `rel` is relative URI
 * (`URIParser::URI`).
 *
 * @return New resolved `URIParser::URI` instance.
 * @sa mrb_uriparser_merge_mutably
 * @sa mrb_uriparser_create_reference
 */
static mrb_value mrb_uriparser_merge(mrb_state *const mrb,
                                     const mrb_value self) {
  const mrb_value rel;
  mrb_get_args(mrb, "o", &rel);

  /* TODO: Accept string as well. */
  if (!mrb_obj_is_kind_of(mrb, rel, MRB_URIPARSER_URI_CLASS(mrb)))
    MRB_URIPARSER_RAISE(mrb, "relative URI is expected to be URIParser::URI");

  UriUriA *const resolved = mrb_malloc(mrb, sizeof(UriUriA));
  if (uriAddBaseUriA(resolved, MRB_URIPARSER_URI(rel),
                     MRB_URIPARSER_URI(self)) != URI_SUCCESS)
    MRB_URIPARSER_RAISE(mrb, "failed to resolve URI");
  MRB_URIPARSER_NEW(mrb, resolved);
}

/**
 * @brief Create a relative reference from a base URI.
 *
 * ```ruby
 * uri.route_from(base, domain_root: false)
 * uri - base
 *
 * uri.route_to(dest, domain_root: false)
 * ```
 *
 * where `base` is a base URI (`URIParser::URI`).  If `domain_root` is true,
 * reference is from domain root.
 *
 * @return New relative `URIParser::URI` instance.
 * @sa mrb_uriparser_merge
 */
static mrb_value mrb_uriparser_create_reference(mrb_state *const mrb,
                                                const mrb_value self) {
  const mrb_value base;
  const mrb_int num = 1;
  const mrb_sym domain_root_key = mrb_intern_lit(mrb, "domain_root");
  mrb_value values[num];
  const mrb_kwargs kwargs = {.num = num,
                             .required = 0,
                             .rest = NULL,
                             .table = &domain_root_key,
                             .values = values};
  mrb_get_args(mrb, "o:", &base, &kwargs);
  if (mrb_undef_p(values[0]))
    values[0] = mrb_false_value();
  if (!mrb_obj_is_kind_of(mrb, base, MRB_URIPARSER_URI_CLASS(mrb)))
    MRB_URIPARSER_RAISE(mrb, "base URI is expected to be URIParser::URI");
  UriUriA *const dest = mrb_malloc(mrb, sizeof(UriUriA));
  if (uriRemoveBaseUriA(dest, MRB_URIPARSER_URI(self), MRB_URIPARSER_URI(base),
                        mrb_test(values[0]) ? URI_TRUE : URI_FALSE) !=
      URI_SUCCESS)
    MRB_URIPARSER_RAISE(mrb, "failed to remove base URI");
  MRB_URIPARSER_NEW(mrb, dest);
}

/**
 * @brief Normalize URI components in place.
 *
 * ```ruby
 * uri.normalize!(scheme: true,
 *                userinfo: true,
 *                host: true,
 *                path: true,
 *                query: true,
 *                fragment: true)
 * ```
 *
 * where `uri` is kind of `URIParser::URI`.  `scheme`, `userinfo`, `host`,
 * `path`, `query`, `fragment` are for enabling normalization for each
 * component.
 *
 * @return Modified `URIParser::URI` instance.
 *
 * By default all parts are normalized.  If path is empty, this
 * doen't set `/`, which is the difference against CRuby's URI gem's
 * `URI::Generic#normalize!` method.
 */
static mrb_value mrb_uriparser_normalize(mrb_state *const mrb,
                                         const mrb_value self) {
  const mrb_int kw_num = 6;
  const mrb_sym kw_table[] = {
      mrb_intern_lit(mrb, "scheme"), mrb_intern_lit(mrb, "userinfo"),
      mrb_intern_lit(mrb, "host"),   mrb_intern_lit(mrb, "path"),
      mrb_intern_lit(mrb, "query"),  mrb_intern_lit(mrb, "fragment")};
  mrb_value kw_values[kw_num];
  const mrb_kwargs kwargs = {.num = kw_num,
                             .required = 0,
                             .rest = NULL,
                             .table = kw_table,
                             .values = kw_values};
  mrb_get_args(mrb, ":", &kwargs);
  unsigned int mask = URI_NORMALIZED;
  if (mrb_undef_p(kw_values[0]) || mrb_test(kw_values[0]))
    mask |= URI_NORMALIZE_SCHEME;
  if (mrb_undef_p(kw_values[1]) || mrb_test(kw_values[1]))
    mask |= URI_NORMALIZE_USER_INFO;
  if (mrb_undef_p(kw_values[2]) || mrb_test(kw_values[2]))
    mask |= URI_NORMALIZE_HOST;
  if (mrb_undef_p(kw_values[3]) || mrb_test(kw_values[3]))
    mask |= URI_NORMALIZE_PATH;
  if (mrb_undef_p(kw_values[4]) || mrb_test(kw_values[4]))
    mask |= URI_NORMALIZE_QUERY;
  if (mrb_undef_p(kw_values[5]) || mrb_test(kw_values[5]))
    mask |= URI_NORMALIZE_FRAGMENT;
  if (uriNormalizeSyntaxExA(MRB_URIPARSER_URI(self), mask) != URI_SUCCESS)
    MRB_URIPARSER_RAISE(mrb, "failed to normalize");
  return self;
}

/**
 * @brief Decode the query string into an array of key-value pairs.
 *
 * ```ruby
 * uri.decode_www_form
 * ```
 *
 * where `uri` is a `URIParser::URI` instance.
 *
 * @return Array of `[key, value]` pairs.
 * @sa mrb_uriparser_compose_query
 *
 * No `enc=Encoding::UTF_8` parameter as in CRuby's URI gem.
 */
static mrb_value mrb_uriparser_dissect_query(mrb_state *const mrb,
                                             const mrb_value self) {
  const mrb_uriparser_data *const data = DATA_PTR(self);
  UriQueryListA *query_list;
  int item_count;
  if (uriDissectQueryMallocA(&query_list, &item_count, data->uri->query.first,
                             data->uri->query.afterLast) != URI_SUCCESS)
    MRB_URIPARSER_RAISE(mrb, "failed to dissect query");
  mrb_value ary = mrb_ary_new(mrb);
  while (query_list) {
    mrb_value entry = mrb_ary_new(mrb);
    mrb_ary_push(mrb, entry, mrb_str_new_cstr(mrb, query_list->key));
    mrb_ary_push(mrb, entry,
                 query_list->value ? mrb_str_new_cstr(mrb, query_list->value)
                                   : mrb_nil_value());
    mrb_ary_push(mrb, ary, entry);
    query_list = query_list->next;
  }
  uriFreeQueryListA(query_list);
  return ary;
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
  mrb_define_module_function(mrb, uriparser, "uri_string_to_filename",
                             mrb_uriparser_uri_string_to_filename,
                             MRB_ARGS_ANY());
  mrb_define_module_function(mrb, uriparser, "encode_www_form",
                             mrb_uriparser_compose_query, MRB_ARGS_REQ(1));
  struct RClass *const uri = mrb_define_class_under(
      mrb, uriparser, MRB_URIPARSER_URI_MODULE_NAME, mrb->object_class);
#ifdef HAVE_URI_COPY_URI
  mrb_define_method(mrb, uri, "initialize_copy", mrb_uriparser_initialize_copy,
                    MRB_ARGS_REQ(1));
#endif
#ifdef HAVE_URI_EQUALS_URI
  mrb_define_method(mrb, uri, "==", mrb_uriparser_equals, MRB_ARGS_REQ(1));
#endif
  mrb_define_method(mrb, uri, "scheme", mrb_uriparser_scheme, MRB_ARGS_NONE());
#ifdef HAVE_URI_SET_SCHEME
  mrb_define_method(mrb, uri, "scheme=", mrb_uriparser_set_Scheme,
                    MRB_ARGS_REQ(1));
#endif
  mrb_define_method(mrb, uri, "userinfo", mrb_uriparser_userInfo,
                    MRB_ARGS_NONE());
#ifdef HAVE_URI_SET_USERINFO
  mrb_define_method(mrb, uri, "userinfo=", mrb_uriparser_set_UserInfo,
                    MRB_ARGS_REQ(1));
#endif
  mrb_define_method(mrb, uri, "hostname", mrb_uriparser_hostText,
                    MRB_ARGS_NONE());
#ifdef HAVE_URI_SET_HOST
  mrb_define_method(mrb, uri, "host=", mrb_uriparser_set_HostAuto,
                    MRB_ARGS_REQ(1));
#endif
#ifdef HAVE_URI_HAS_HOST
  mrb_define_method(mrb, uri, "host?", mrb_uriparser_has_host, MRB_ARGS_NONE());
#endif
  mrb_define_method(mrb, uri, "port", mrb_uriparser_portText, MRB_ARGS_NONE());
#ifdef HAVE_URI_SET_PORT
  mrb_define_method(mrb, uri, "port=", mrb_uriparser_set_port, MRB_ARGS_REQ(1));
#endif
  mrb_define_method(mrb, uri, "path_segments", mrb_uriparser_path_segments,
                    MRB_ARGS_NONE());
#ifdef HAVE_URI_SET_PATH
  mrb_define_method(mrb, uri, "path=", mrb_uriparser_set_path, MRB_ARGS_REQ(1));
#endif
  mrb_define_method(mrb, uri, "query", mrb_uriparser_query, MRB_ARGS_NONE());
#ifdef HAVE_URI_SET_QUERY
  mrb_define_method(mrb, uri, "query=", mrb_uriparser_set_query,
                    MRB_ARGS_REQ(1));
#endif
  mrb_define_method(mrb, uri, "fragment", mrb_uriparser_fragment,
                    MRB_ARGS_NONE());
#ifdef HAVE_URI_SET_FRAGMENT
  mrb_define_method(mrb, uri, "fragment=", mrb_uriparser_set_fragment,
                    MRB_ARGS_REQ(1));
#endif
  mrb_define_method(mrb, uri, "absolute_path?", mrb_uriparser_absolute_path,
                    MRB_ARGS_NONE());
  mrb_define_method(mrb, uri, "to_s", mrb_uriparser_recompose, MRB_ARGS_NONE());
  mrb_define_method(mrb, uri, "merge!", mrb_uriparser_merge_mutably,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, uri, "merge", mrb_uriparser_merge, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, uri, "route_from", mrb_uriparser_create_reference,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, uri, "normalize!", mrb_uriparser_normalize,
                    MRB_ARGS_KEY(6, 0));
  mrb_define_method(mrb, uri, "decode_www_form", mrb_uriparser_dissect_query,
                    MRB_ARGS_NONE());
  DONE;
}

void mrb_mruby_uriparser_gem_final(const mrb_state *const mrb) {}
