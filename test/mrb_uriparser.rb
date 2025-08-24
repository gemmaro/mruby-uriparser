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
  assert_equal("docs/ja/protocol-specification.gmi", uri.path)
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

assert("URIParser::URI.parse") do
  uri = URIParser::URI.parse("http://example.com")
  assert_kind_of(URIParser::URI, uri)
end

assert("URIParser::URI#path") do
  uri = URIParser.parse("/abs")
  assert_equal("abs", uri.path)

  uri = URIParser.parse("./rel")
  assert_equal("./rel", uri.path)

  uri = URIParser.parse("rel")
  assert_equal("rel", uri.path)
end

assert("URIParser::URI#to_s") do
  uri = URIParser.parse("s://u:p@h:0/p?q#f")
  assert_equal("s://u:p@h:0/p?q#f", uri.to_s)
end

assert("URIParser::URI#merge!") do
  uri = URIParser.parse("file:///one/two/three")
  rel = URIParser.parse("../TWO")
  resolved = uri.merge!(rel)
  assert_kind_of(URIParser::URI, resolved)
  assert_equal("file", resolved.scheme)
  assert_equal("one/TWO", resolved.path)
  assert_equal("one/TWO", uri.path)
  assert_equal("../TWO", rel.path)
end

assert("URIParser::URI#merge") do
  uri = URIParser.parse("file:///one/two/three")
  rel = URIParser.parse("../TWO")
  resolved = uri.merge(rel)
  assert_kind_of(URIParser::URI, resolved)
  assert_equal("one/TWO", resolved.path)
  assert_equal("one/two/three", uri.path)
  assert_equal("../TWO", rel.path)

  resolved = uri + rel
  assert_kind_of(URIParser::URI, resolved)
  assert_equal("one/TWO", resolved.path)
  assert_equal("one/two/three", uri.path)
end

assert("URIParser::URI#route_from") do
  uri = URIParser.parse("file:///one/TWO")
  base = URIParser.parse("file:///one/two/three")
  dest = uri.route_from(base)
  assert_kind_of(URIParser::URI, dest)
  assert_equal("../TWO", dest.path)

  dest = uri.route_from(base, domain_root: true)
  assert_equal("one/TWO", dest.path)
end
