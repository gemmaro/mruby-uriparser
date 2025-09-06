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

Refer to the API documentation for details.

## Running Tests

To run tests:

```shell
rake --directory /path/to/mruby all test MRUBY_CONFIG=$PWD/build_config.rb
```

Alternatively, use the `test/run` script with `MRUBY_SRC` set in your `.env` file.

## Prior Work

See [mruby-uri-parser](https://github.com/Asmod4n/mruby-uri-parser), which uses [uri\_parser](https://github.com/Zewo/uri_parser).
Also see [ruby-uriparser](https://github.com/tlewin/ruby-uriparser).

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
