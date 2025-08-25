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

  def self.from_filename(filename)
    parse(filename_to_uri_string(filename))
  end

  class URI
    def self.parse(str)
      URIParser.parse(str)
    end

    alias + merge
    alias - route_from

    def route_to(dest, domain_root: false)
      dest.route_from(self, domain_root:)
    end
  end
end
