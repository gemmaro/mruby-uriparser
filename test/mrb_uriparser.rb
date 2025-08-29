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
  assert_equal("geminiprotocol.net", uri.hostname)
  assert_equal(nil, uri.port)
  assert_nil(uri.query)
  assert_nil(uri.fragment)

  uri = URIParser.parse("gemini://example.com:1965")
  assert_equal('1965', uri.port)

  uri = URIParser.parse("http://u:p@example.com?q#f")
  assert_equal("u:p", uri.userinfo)
  assert_equal("q", uri.query)
  assert_equal("f", uri.fragment)

  uri = URIParser.parse("http://[::1]/bar")
  assert_equal "::1", uri.hostname
  # assert_equal "[::1]", uri.host

  assert_raise_with_message(URIParser::Error, "URI parse failed at: ` bar'") do
    URIParser.parse("foo bar")
  end
end

assert("URIParser::URI.parse") do
  uri = URIParser::URI.parse("http://example.com")
  assert_kind_of(URIParser::URI, uri)
end

assert("URIParser.filename_to_uri_string") do
  uri_str = URIParser.filename_to_uri_string("E:\\Documents and Settings",
                                             windows: true)
  assert_equal("file:///E:/Documents%20and%20Settings", uri_str)

  uri_str = URIParser.filename_to_uri_string("/usr/bin/env")
  assert_equal("file:///usr/bin/env", uri_str)
end

assert("URIParser.uri_string_to_filename") do
  uri_str = URIParser.uri_string_to_filename("file:///E://Documents%20and%20Settings",
                                             windows: true)
  assert_equal("E:\\\\Documents and Settings", uri_str)

  uri_str = URIParser.uri_string_to_filename("file:///usr/bin/env")
  assert_equal("/usr/bin/env", uri_str)
end

assert("URIParser.encode_www_form") do
  assert_equal "a=1&b=2&c=x+yz",
               URIParser.encode_www_form([["a", "1"], ["b", "2"], ["c", "x yz"]])
  assert_equal "a=&b",
               URIParser.encode_www_form([["a", ""], ["b", nil]])

  # TODO: Implement Hash case.
  assert_raise_with_message(TypeError, "Hash cannot be converted to Array") do
    URIParser.encode_www_form({"a"=>"1", "b"=>"2", "c"=>"x yz"})
    # "a=1&b=2&c=x+yz"
  end
end

assert("URIParser.join") do
  uri = URIParser.join('http://www.ruby-lang.org/', '/ja/man-1.6/')
  assert_equal 'http://www.ruby-lang.org/ja/man-1.6/', uri.to_s

  uri = URIParser::URI.join('http://www.ruby-lang.org/', '/ja/man-1.6/')
  assert_equal 'http://www.ruby-lang.org/ja/man-1.6/', uri.to_s
end

# assert("URIParser.split") do
#   assert_equal ["http", nil, "www.ruby-lang.org", nil, nil, "/", nil, nil, nil],
#                URIParser.split("http://www.ruby-lang.org/")
# end

assert("URIParser::URI#path_segments") do
  uri = URIParser::URI.parse("/a/b")
  assert_equal(["a", "b"], uri.path_segments)

  uri = URIParser::URI.parse("a/b")
  assert_equal(["a", "b"], uri.path_segments)

  uri = URIParser::URI.parse("/")
  assert_equal([], uri.path_segments)
end

assert("URIParser::URI#absolute?") do
  uri = URIParser.parse("/abs")
  assert_equal(true, uri.absolute?)

  uri = URIParser.parse("rel")
  assert_equal(false, uri.absolute?)
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
  assert_equal("file:///one/TWO", resolved.to_s)
  assert_equal("file:///one/TWO", uri.to_s)
  assert_equal("../TWO", rel.to_s)
end

assert("URIParser::URI#merge") do
  uri = URIParser.parse("file:///one/two/three")
  rel = URIParser.parse("../TWO")
  resolved = uri.merge(rel)
  assert_kind_of(URIParser::URI, resolved)
  assert_equal("file:///one/TWO", resolved.to_s)
  assert_equal("file:///one/two/three", uri.to_s)
  assert_equal("../TWO", rel.to_s)

  resolved = uri + rel
  assert_kind_of(URIParser::URI, resolved)
  assert_equal("file:///one/TWO", resolved.to_s)
  assert_equal("file:///one/two/three", uri.to_s)

  rel = URIParser.parse('/foo/bar.html')
  assert_equal("http://example.com/foo/bar.html",
               (URIParser.parse('http://example.com/') + rel).to_s)

  assert_equal("http://a/b/c/d;p?y", URIParser.parse('http://a/b/c/d;p?q').merge(URIParser.parse('?y')).to_s)
  assert_equal("http://a/g", URIParser.parse('http://a/b/c/d;p?q').merge(URIParser.parse('/./g')).to_s)
  assert_equal("http://a/g", URIParser.parse('http://a/b/c/d;p?q').merge(URIParser.parse('/../g')).to_s)
  assert_equal("http://a/g", URIParser.parse('http://a/b/c/d;p?q').merge(URIParser.parse('../../../g')).to_s)
  assert_equal("http://a/g", URIParser.parse('http://a/b/c/d;p?q').merge(URIParser.parse('../../../../g')).to_s)
end

assert("URIParser::URI#route_from") do
  uri = URIParser.parse("file:///one/TWO")
  base = URIParser.parse("file:///one/two/three")
  dest = uri.route_from(base)
  assert_kind_of(URIParser::URI, dest)
  assert_equal("../TWO", dest.to_s)

  dest = uri.route_from(base, domain_root: true)
  assert_equal("/one/TWO", dest.to_s)
end

assert("URIParser::URI#normalize!") do
  uri = URIParser.parse("http://example.org/one/two/../../one")
  uri.normalize!
  assert_equal "http://example.org/one", uri.to_s

  uri = URIParser.parse("http://example.org/one/two/../../one")
  uri.normalize!(path: false)
  assert_equal "http://example.org/one/two/../../one", uri.to_s

  uri = URIParser.parse("http://EXAMPLE.ORG")
  uri.normalize!(scheme: false,
                 userinfo: false,
                 path: false,
                 query: false,
                 fragment: false)
  assert_equal "http://example.org", uri.to_s
end

assert("URIParser::URI#decode_www_form") do
  uri = URIParser.parse("http://example.com?a=1&a=2&b=&c")
  assert_equal [['a', '1'], ['a', '2'], ['b', ''], ['c', nil]],
               uri.decode_www_form
end
