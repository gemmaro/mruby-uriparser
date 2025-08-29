# mruby uriparser

The [uriparser](https://uriparser.github.io/) binding for mruby.

```ruby
conf.linker.libraries << 'uriparser'
```

in `build_config.rb`.

Here is an example.  Please refer to the API documentation for more details.

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

The below is the supported features list.
"-" means it is not supported yet or has no plan to support it.

| this gem                                                | CRuby's URI gem                                     |
|---------------------------------------------------------|-----------------------------------------------------|
| `URIParser::URI#decode_www_form`[^1]                    | `URI.decode_www_form`                               |
| -                                                       | `URI.decode_www_form_component`                     |
| `URIParser.encode_www_form`[^2]                         | `URI.encode_www_form`                               |
| -                                                       | `URI.encode_www_form_component`                     |
| -                                                       | `URI.extract`[^4]                                   |
| `URIParser.join`, `URIParser::URI.join`                 | `URI.join`                                          |
| `URIParser.parse`, `URIParser::URI.parse`               | `URI.parse`                                         |
| -                                                       | `URI.regexp`[^3]                                    |
| -                                                       | `URI.split`                                         |
| -                                                       | `URI::UNSAFE`                                       |
| -                                                       | `URI::Generic.build`                                |
| -                                                       | `URI::Generic.build2`                               |
| -                                                       | `URI::Generic.component`                            |
| -                                                       | `URI::Generic.default_port`                         |
| -                                                       | `URI::Generic.new`                                  |
| -                                                       | `URI::Generic.use_registry`                         |
| `URIParser::URI#merge`, `URIParser::URI#@+`[^5]         | `URI::Generic#merge`, `URI::Generic#@+`             |
| `URIParser::URI#route_from`, `URIParser::URI#@-`        | `URI::Generic#route_from`, `URI::Generic#@-`        |
| -                                                       | `URI::Generic#==`                                   |
| `URIParser::URI#absolute`, `URIParser::URI#absolute?`   | `URI::Generic#absolute`, `URI::Generic#absolute?`   |
| -                                                       | `URI::Generic#coerce`                               |
| -                                                       | `URI::Generic#component`                            |
| -                                                       | `URI::Generic#find_proxy`                           |
| `URIParser::URI#fragment`                               | `URI::Generic#fragment`                             |
| -                                                       | `URI::Generic#fragment=`                            |
| `URIParser::URI#hierarchical?`                          | `URI::Generic#hierarchical?`                        |
| -                                                       | `URI::Generic#host`                                 |
| -                                                       | `URI::Generic#host=`                                |
| `URIParser::URI#hostname`                               | `URI::Generic#hostname`                             |
| -                                                       | `URI::Generic#hostname=`                            |
| `URIParser::URI#merge!`                                 | `URI::Generic#merge!`                               |
| `URIParser::URI#normalize`, `URIParser::URI#normalize!` | `URI::Generic#normalize`, `URI::Generic#normalize!` |
| -                                                       | `URI::Generic#opaque`                               |
| -                                                       | `URI::Generic#opaque=`                              |
| -                                                       | `URI::Generic#parser`                               |
| -                                                       | `URI::Generic#password`                             |
| -                                                       | `URI::Generic#password=`                            |
| -                                                       | `URI::Generic#path`                                 |
| -                                                       | `URI::Generic#path=`                                |
| -                                                       | `URI::Generic#port`                                 |
| -                                                       | `URI::Generic#port=`                                |
| `URIParser::URI#query`                                  | `URI::Generic#query`                                |
| `URIParser::URI#query=`                                 | `URI::Generic#query=`                               |
| -                                                       | `URI::Generic#registry`                             |
| -                                                       | `URI::Generic#registry=`                            |
| `URIParser::URI#relative?`                              | `URI::Generic#relative?`                            |

[^1]: No `enc=Encoding::UTF_8` parameter as CRuby's URI gem.

[^2]: `enum` is only `Array[String, String | nil]`.  No `enc=nil` parameter as CRuby's URI gem.

[^3]: This method is obsolete since Ruby 2.2.

[^4]: This method is obsolete since Ruby 2.2.

[^5]: Passed relative path must be URI at the moment.

Use constant alias (e.g. `URI = URIParser::URI`) as you like.

```shell
rake --directory /path/to/mruby all test MRUBY_CONFIG=$PWD/build_config.rb
```

to run tests.  You can also use `test/run` script with `MRUBY_SRC` env var in `.env` file.

## Prior works

There is [mruby-uri-parser](https://github.com/Asmod4n/mruby-uri-parser "GitHub"),
which uses [uri\_parser](https://github.com/Zewo/uri_parser "GitHub").

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
