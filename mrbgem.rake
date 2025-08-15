MRuby::Gem::Specification.new('mruby-uriparser') do |spec|
  spec.license = 'GPL-3.0-or-later'
  spec.authors = 'gemmaro'
  spec.linker.libraries << 'uriparser'
end
