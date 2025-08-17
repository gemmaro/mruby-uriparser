# mruby uriparser

The uriparser binding for mruby.

``` ruby
conf.linker.libraries << 'uriparser'
```

in `build_config.rb`.

``` shell
rake --directory /path/to/mruby all test MRUBY_CONFIG=$PWD/build_config.rb
```

to run tests.

## Prior works

There is [mruby-uri-parser](https://github.com/Asmod4n/mruby-uri-parser "GitHub"),
which is based on the NGINX's implementation.

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
