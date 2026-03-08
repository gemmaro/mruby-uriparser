MRuby::Gem::Specification.new('mruby-uriparser') do |spec|
  spec.license = 'GPL-3.0-or-later'
  spec.authors = 'gemmaro'
  spec.summary = 'URI parser for mruby'
  spec.version = '0.2.2'
  spec.homepage = 'https://github.com/gemmaro/mruby-uriparser'
  spec.linker.libraries << 'uriparser'
  spec.add_test_dependency 'mruby-io'
end
