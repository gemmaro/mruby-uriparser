# Copyright (C) 2025  gemmaro
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

assert("URIParser.parse") do
  uri = URIParser.parse("gemini://geminiprotocol.net/docs/ja/protocol-specification.gmi")
  assert_kind_of(URIParser::URI, uri)
  assert_equal("gemini", uri.scheme)
  assert_equal("geminiprotocol.net", uri.host)
  assert_equal(nil, uri.port)
  assert_equal("/docs/ja/protocol-specification.gmi", uri.path)
  assert_nil(uri.query)
  assert_nil(uri.fragment)

  uri = URIParser.parse("gemini://example.com:1965")
  assert_equal(1965, uri.port)
  assert_equal(nil, uri.path)

  uri = URIParser.parse("http://u:p@example.com?q#f")
  assert_equal("u:p", uri.userinfo)
  assert_equal("q", uri.query)
  assert_equal("f", uri.fragment)

  assert_raise_with_message(URIParser::Error, "URI parse failed at: ` bar'") do
    URIParser.parse("foo bar")
  end
end
