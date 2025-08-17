# mruby uriparser

The uriparser binding for mruby.

``` ruby
conf.linker.libraries << 'uriparser'
```

in `build_config.rb`.

Here is an example.
Use constant alias (e.g. `URI = URIParser::URI`) as you like.

```ruby
str = "http://user:pass@example.com:8000/some-path?some-query#some-fragment"
uri = URIParser.parse(str)
uri.class    #=> URIParser::URI
uri.scheme   #=> "http"
uri.userinfo #=> "user:pass"
uri.host     #=> "example.com"
uri.port     #=> 8000
uri.path     #=> "/some-path"
uri.query    #=> "some-query"
uri.fragment #=> "some-fragment"
```

``` shell
rake --directory /path/to/mruby all test MRUBY_CONFIG=$PWD/build_config.rb
```

to run tests.

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
