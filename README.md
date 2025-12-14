# mruby uriparser

mruby uriparser provides [uriparser](https://uriparser.github.io/) bindings for [mruby](https://mruby.org/).

## Installation

This mgem requires the uriparser library to be available.
In most cases, you only need to install it [using your system's package manager](https://repology.org/project/uriparser/versions).
If you installed it manually, make sure to update the relevant C environment variables, such as `C_INCLUDE_PATH` and `LIBRARY_PATH`.
If you are using [GNU Guix](https://guix.gnu.org/) together with [Direnv](https://direnv.net/), simply run `direnv allow .`.

Then add the following to your `build_config.rb`:

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

## Links

* [API documentation](https://gemmaro.github.io/mruby-uriparser/ "mruby-uriparser")
* [GitHub repository](https://github.com/gemmaro/mruby-uriparser "GitHub")

For prior works, see [mruby-uri-parser](https://github.com/Asmod4n/mruby-uri-parser), which uses [uri\_parser](https://github.com/Zewo/uri_parser), and [mruby-uri](https://github.com/zzak/mruby-uri), which is written in pure Ruby.
Also see [ruby-uriparser](https://github.com/tlewin/ruby-uriparser) as in [uriparser - uriparser Bindings & 3rd-party Wrappers](https://uriparser.github.io/doc/bindings/ "uriparser").

## Running Tests

To run tests:

```shell
rake --directory /path/to/mruby all test MRUBY_CONFIG=$PWD/build_config.rb
```

Alternatively, use the `test/run` script with `MRUBY_SRC` set in your `.env` file.

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
