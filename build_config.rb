require "tmpdir"
require "shellwords"
require "open3"

MRuby::Build.new do |conf|
  toolchain :gcc
  conf.gem File.expand_path(File.dirname(__FILE__))
  conf.enable_test

  conf.linker.libraries << 'uriparser'

  Dir.mktmpdir("mruby_check_") do |tmpdir|
    exe = File.join(tmpdir, "check_uri_has_host")
    command = [conf.cc.command,
               *conf.cc.flags.flatten,
               "-l", "uriparser",
               '-o', exe,
               File.join(__dir__, "build/check_uri_has_host.c")]
    output, status = Open3.capture2e(ENV, *command)
    puts ">>> output (status: #{status}) >>>"
    puts output
    puts "<<< output <<<"
    if status.success?
      conf.defines << "HAVE_URI_HAS_HOST"
    else
      warn "don't have uriHasHostA"
    end
  end

  if ENV['DEBUG'] == 'true'
    conf.enable_debug
    conf.cc.defines = %w(MRB_ENABLE_DEBUG_HOOK)
    conf.gem core: 'mruby-bin-debugger'
  end
end
