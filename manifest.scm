(use-modules (guix profiles)
             (gnu packages commencement)
             (gnu packages web)
             (gnu packages build-tools)
             (gnu packages documentation)
             (guix packages)
             (guix download)
             (guix git-download)
             (gnu packages ruby-xyz)
             (gnu packages ruby-check)
             (guix build-system ruby)
             ((guix licenses)
              #:prefix license:)
             (guix gexp))

(define-public ruby-rubocop-minitest
  (package
    (name "ruby-rubocop-minitest")
    (version "0.38.2")
    (source
     (origin
       (method git-fetch)
       (uri (git-reference
             (url "https://github.com/rubocop/rubocop-minitest")
             (commit (string-append "v" version))))
       (file-name (git-file-name name version))
       (sha256
        (base32 "0yhn2jflwiklp6nc9i90z17yd2a8ll0chi8s3pxhxk91h7sy8pha"))))
    (build-system ruby-build-system)
    (arguments
     (list
      #:phases
      #~(modify-phases %standard-phases
          (add-before 'check 'remove-Bundler-execution
            (lambda _
              (substitute* "Rakefile"
                (("Bundler.setup\\(.+")
                 "")
                ((" \\|\\| RuboCop::Platform[.]windows[?]")
                 "")
                (("sh\\(\"bundle exec (.+)" _ rest)
                 "sh(\"" rest "\n"))))
          (add-before 'check 'delete-unused-task
            (lambda _
              (delete-file "tasks/cops_documentation.rake")
              (delete-file "tasks/cut_release.rake"))))))
    (native-inputs (list ruby-minitest ruby-minitest-proveit))
    (propagated-inputs (list ruby-lint-roller ruby-rubocop ruby-rubocop-ast))
    (synopsis
     "Automatic Minitest code style checking tool.
A RuboCop extension focused on enforcing Minitest best practices and coding conventions.
")
    (description
     "Automatic Minitest code style checking tool.  A @code{RuboCop} extension focused
on enforcing Minitest best practices and coding conventions.")
    (home-page "https://docs.rubocop.org/rubocop-minitest/")
    (license license:expat)))

(define-public ruby-lsp
  (package
    (name "ruby-lsp")
    (version "0.26.3")
    (source
     (origin
       (method git-fetch) ;for tests
       (uri (git-reference
             (url "https://github.com/Shopify/ruby-lsp")
             (commit (string-append "v" version))))
       (file-name (git-file-name name version))
       (sha256
        (base32 "1by5alfvy414g0dgxpia64a0jkb01s5fcb3qqaw3cw9pdc0f64zb"))))
    (build-system ruby-build-system)
    (arguments
     (list
      #:phases
      #~(modify-phases %standard-phases
          ;; rakelib/index.rake requires its library path is set.
          (add-before 'check 'add-Ruby-library
            (lambda _
              (setenv "RUBYLIB"
                      (string-append (getcwd) "/lib:"
                                     (or (getenv "RUBYLIB") "")))))
          (add-before 'check 'delete-failing-tests
            (lambda _
              (delete-file "test/integration_test.rb")
              ;; These tests requires rubocop-sorbet gem or file paths go wrong.
              (delete-file "test/server_test.rb")
              (delete-file-recursively "test/requests")
              ;; These tests requires rubocop-minitest gem.
              (delete-file-recursively "test/rubocop/cop/ruby_lsp")
              ;; This uses Bundler.
              (delete-file "test/setup_bundler_test.rb")
              ;; These tests don't finish.
              (delete-file-recursively "test/test_reporters")))
          (replace 'check
            (lambda* (#:key tests? #:allow-other-keys)
              (when tests?
                (for-each (lambda (file)
                            (display file)
                            (invoke "ruby" "-Ilib:test" file "--verbose"))
                          (find-files "test" "_test[.]rb$"))))))))
    (native-inputs (list ruby-minitest ruby-test-unit ruby-mocha ruby-rubocop
                         ruby-syntax-tree))
    (propagated-inputs (list ruby-language-server-protocol ruby-prism))
    (synopsis "An opinionated language server for Ruby")
    (description "An opinionated language server for Ruby.")
    (home-page "https://github.com/Shopify/ruby-lsp")
    (license license:expat)))

(packages->manifest (list gcc-toolchain uriparser doxygen bear))
