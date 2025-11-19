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

module URIParser
  Error = Class.new(StandardError)

  module ClassMethods
    def join(uri_str, *path)
      # TODO: Remove parsing each path after implement accepting URI by merge methods.
      path.reduce(URIParser.parse(uri_str)) { |memo, pat| memo.merge!(URIParser.parse(pat)) }
    end
  end

  class << self
    include ClassMethods
  end

  class URI
    def self.parse(str)
      URIParser.parse(str)
    end

    def self.from_filename(filename, windows: false)
      parse(URIParser.filename_to_uri_string(filename, windows:))
    end

    class << self
      include ClassMethods
    end

    # https://github.com/uriparser/uriparser/blob/1c2ed87f3a2dbfcb99e4f4f6a86f8c4e04f51ddf/src/UriRecompose.c#L435-L499
    def path
      segments = path_segments
      result = ""
      if absolute_path? || !segments.empty? && host?
        result += "/"
      end
      result += segments.join("/")
      result
    end

    def absolute?
      scheme ? true : false
    end

    alias + merge
    alias - route_from
    alias absolute absolute?

    def route_to(dest, domain_root: false)
      dest.route_from(self, domain_root:)
    end

    def to_filename(windows: false)
      URIParser.uri_string_to_filename(to_s, windows:)
    end

    def relative?
      !absolute?
    end
  end
end
