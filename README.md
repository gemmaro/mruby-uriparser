# mruby uriparser

mruby uriparser provides [uriparser](https://uriparser.github.io/) bindings for [mruby](https://mruby.org/).

## Installation

Add the following to your `build_config.rb`:

```ruby
conf.linker.libraries << 'uriparser'
```

## Usage

```ruby
str = "http://user:pass@example.com:8000/some-path?some-query#some-fragment"
uri = URIParser.parse(str)
uri.class    #=> URIParser::URI
uri.scheme   #=> "http"
uri.userinfo #=> "user:pass"
uri.host     #=> "example.com"
uri.port     #=> "8000"
uri.query    #=> "some-query"
uri.fragment #=> "some-fragment"
uri.to_s     #=> same as str
```

You may use a constant alias for convenience:

```ruby
URI = URIParser::URI
```

## Features

Below is a comparison of supported features with CRuby's URI gem.
A dash (`-`) indicates not supported or no plans to support.

| mruby uriparser                                       | CRuby URI gem                                       | mruby-uri-parser                           |
|-------------------------------------------------------|-----------------------------------------------------|--------------------------------------------|
| `URIParser::URI#decode_www_form`[^1]                  | `URI.decode_www_form`                               |                                            |
| -                                                     | `URI.decode_www_form_component`                     | `URI.decode`                               |
| `URIParser.encode_www_form`[^2]                       | `URI.encode_www_form`                               |                                            |
| -                                                     | `URI.encode_www_form_component`                     | `URI.encode`                               |
| -                                                     | `URI.extract`[^4]                                   |                                            |
| `URIParser.join`, `URIParser::URI.join`               | `URI.join`                                          |                                            |
| `URIParser.parse`, `URIParser::URI.parse`             | `URI.parse`                                         | `URI.parse`                                |
| -                                                     | `URI.regexp`[^3]                                    |                                            |
| -                                                     | `URI.split`                                         |                                            |
| -                                                     | `URI::UNSAFE`                                       |                                            |
| -                                                     | `URI::Generic.build`                                |                                            |
| -                                                     | `URI::Generic.build2`                               |                                            |
| -                                                     | `URI::Generic.component`                            |                                            |
| -                                                     | `URI::Generic.default_port`                         |                                            |
| -                                                     | `URI::Generic.new`                                  | `URI::Parsed.new`                          |
| -                                                     | `URI::Generic.use_registry`                         |                                            |
| `URIParser::URI#merge`, `URIParser::URI#@+`[^5]       | `URI::Generic#merge`, `URI::Generic#@+`             |                                            |
| `URIParser::URI#route_from`, `URIParser::URI#@-`      | `URI::Generic#route_from`, `URI::Generic#@-`        |                                            |
| -                                                     | `URI::Generic#==`                                   |                                            |
| `URIParser::URI#absolute`, `URIParser::URI#absolute?` | `URI::Generic#absolute`, `URI::Generic#absolute?`   |                                            |
| -                                                     | `URI::Generic#coerce`                               |                                            |
| -                                                     | `URI::Generic#component`                            |                                            |
| -                                                     | `URI::Generic#find_proxy`                           |                                            |
| `URIParser::URI#fragment`                             | `URI::Generic#fragment`                             | `URI::Parsed#fragment`                     |
| -                                                     | `URI::Generic#fragment=`                            |                                            |
| `URIParser::URI#hierarchical?`                        | `URI::Generic#hierarchical?`                        |                                            |
| -                                                     | `URI::Generic#host`                                 | `URI::Parsed#host`                         |
| -                                                     | `URI::Generic#host=`                                |                                            |
| `URIParser::URI#hostname`                             | `URI::Generic#hostname`                             |                                            |
| -                                                     | `URI::Generic#hostname=`                            |                                            |
| `URIParser::URI#merge!`                               | `URI::Generic#merge!`                               |                                            |
| `URIParser::URI#normalize!`                           | `URI::Generic#normalize`, `URI::Generic#normalize!` |                                            |
| -                                                     | `URI::Generic#opaque`                               |                                            |
| -                                                     | `URI::Generic#opaque=`                              |                                            |
| -                                                     | `URI::Generic#parser`                               |                                            |
| -                                                     | `URI::Generic#password`                             |                                            |
| -                                                     | `URI::Generic#password=`                            |                                            |
| -                                                     | `URI::Generic#path`                                 | `URI::Parsed#path`                         |
| -                                                     | `URI::Generic#path=`                                |                                            |
| `URIParser::URI#port`                                 | `URI::Generic#port`                                 | `URI::Parsed#port`                         |
| -                                                     | `URI::Generic#port=`                                |                                            |
| `URIParser::URI#query`                                | `URI::Generic#query`                                | `URI::Parsed#query`                        |
| -                                                     | `URI::Generic#query=`                               |                                            |
| -                                                     | `URI::Generic#registry`                             |                                            |
| -                                                     | `URI::Generic#registry=`                            |                                            |
| `URIParser::URI#relative?`                            | `URI::Generic#relative?`                            |                                            |
| `URIParser::URI#route_to`                             | `URI::Generic#route_to`                             |                                            |
| `URIParser::URI#scheme`                               | `URI::Generic#scheme`                               | `URI::Parsed#schema`, `URI::Parsed#scheme` |
| -                                                     | `URI::Generic#scheme=`                              |                                            |
| -                                                     | `URI::Generic#select`                               |                                            |
| `URIParser::URI#to_s`                                 | `URI::Generic#to_s`                                 | `URI::Parsed#to_s`                         |
| -                                                     | `URI::Generic#user`                                 |                                            |
| -                                                     | `URI::Generic#user=`                                |                                            |
| `URIParser::URI#userinfo`                             | `URI::Generic#userinfo`                             | `URI::Parsed#userinfo`                     |
| -                                                     | `URI::Generic#userinfo=`                            |                                            |
| -                                                     | `URI::Generic::COMPONENT`                           |                                            |
| -                                                     | `URI::Generic::DEFAULT_PORT`                        |                                            |
| `URIParser.filename_to_uri_string`                    | -                                                   |                                            |
| `URIParser.uri_string_to_filename`                    | -                                                   |                                            |
| `URIParser::URI#path_segments`                        | -                                                   |                                            |
| `URIParser::URI#absolute_path?`                       | -                                                   |                                            |

[^1]: No `enc=Encoding::UTF_8` parameter as in CRuby's URI gem.

[^2]: Only supports `Array[String, String | nil]` for `enum`. No `enc=nil` parameter.

[^3]: Obsolete since Ruby 2.2.

[^4]: Obsolete since Ruby 2.2.

[^5]: Relative path must be a URI. See API docs.

Refer to the API documentation for details.

## Running Tests

To run tests:

```shell
rake --directory /path/to/mruby all test MRUBY_CONFIG=$PWD/build_config.rb
```

Alternatively, use the `test/run` script with `MRUBY_SRC` set in your `.env` file.

## Prior Work

See [mruby-uri-parser](https://github.com/Asmod4n/mruby-uri-parser), which uses [uri\_parser](https://github.com/Zewo/uri_parser).

## License

Copyright (C) 2025  gemmaro

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
