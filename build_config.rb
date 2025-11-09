require "tmpdir"
require "shellwords"
require "open3"

def check_function(name:, checker:, macro:, conf:)
  Dir.mktmpdir("mruby_check_") do |tmpdir|
    exe = File.join(tmpdir, "check_#{name}")
    command = [conf.cc.command,
               *conf.cc.flags.flatten,
               "-l", "uriparser",
               '-o', exe,
               File.join(__dir__, "build/#{checker}")]
    output, status = Open3.capture2e(ENV, *command)
    puts "status: #{status}"
    output.strip!
    if output
      puts ">>> output >>>"
      puts output
      puts "<<< output <<<"
    end
    if status.success?
      conf.defines << macro
      puts "found #{name}"
    else
      warn "not found #{name}"
    end
  end
end

MRuby::Build.new do |conf|
  toolchain :gcc
  conf.gem File.expand_path(File.dirname(__FILE__))
  conf.enable_test

  conf.linker.libraries << 'uriparser'

  check_function(name: "uriHasHostA",
                 checker: "check_uri_has_host.c",
                 macro: "HAVE_URI_HAS_HOST",
                 conf:)
  check_function(name: "uriCopyUriA",
                 checker: "check_uri_copy_uri.c",
                 macro: "HAVE_URI_COPY_URI",
                 conf:)
  check_function(name: "uriEqualsUriA",
                 checker: "check_uri_equals_uri.c",
                 macro: "HAVE_URI_EQUALS_URI",
                 conf:)
  check_function(name: "uriSetSchemeA",
                 checker: "check_uri_set_scheme.c",
                 macro: "HAVE_URI_SET_SCHEME",
                 conf:)
  check_function(name: "uriSetUserInfoA",
                 checker: "check_uri_set_userinfo.c",
                 macro: "HAVE_URI_SET_USERINFO",
                 conf:)
  check_function(name: "uriSetHostAutoA",
                 checker: "check_uri_set_host.c",
                 macro: "HAVE_URI_SET_HOST",
                 conf:)
  check_function(name: "uriSetPortTextA",
                 checker: "check_uri_set_port.c",
                 macro: "HAVE_URI_SET_PORT",
                 conf:)
  check_function(name: "uriSetPathA",
                 checker: "check_uri_set_path.c",
                 macro: "HAVE_URI_SET_PATH",
                 conf:)
  check_function(name: "uriSetQueryA",
                 checker: "check_uri_set_query.c",
                 macro: "HAVE_URI_SET_QUERY",
                 conf:)

  if ENV['DEBUG'] == 'true'
    conf.enable_debug
    conf.defines << "MRB_ENABLE_DEBUG_HOOK"
    conf.gem core: 'mruby-bin-debugger'
    conf.gem core: 'mruby-bin-mirb'
  end
end
