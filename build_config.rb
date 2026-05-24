MRuby::Build.new do |conf|
  toolchain :gcc
  conf.gem File.expand_path(File.dirname(__FILE__))
  conf.enable_test
  conf.linker.libraries << 'uriparser'

  if ENV['DEBUG'] == 'true'
    conf.enable_debug
    conf.defines << "MRB_ENABLE_DEBUG_HOOK"
    conf.gem core: 'mruby-bin-debugger'
    conf.gem core: 'mruby-bin-mirb'
  end

  if ENV['ENABLE_TESTS'] == 'true'
    conf.gem core: 'mruby-io'
    # conf.gem core: 'hal-posix-io'
  end
end
