SUBDIRS = .

AM_CPPFLAGS = -I$(top_builddir)/src/lib -I$(top_srcdir)/src/lib
AM_CPPFLAGS += $(BOOST_INCLUDES)
AM_CXXFLAGS = $(KEA_CXXFLAGS)
AM_LDADD    =
AM_LDFLAGS  =

if USE_STATIC_LINK
AM_LDFLAGS += -static
endif

CLEANFILES = *.gcno *.gcda *.lock
DISTCLEANFILES = buffer_logger_test.sh
DISTCLEANFILES += console_test.sh destination_test.sh init_logger_test.sh
DISTCLEANFILES += local_file_test.sh logger_lock_test.sh severity_test.sh
DISTCLEANFILES += tempdir.h

EXTRA_DIST = log_test_messages.mes

noinst_PROGRAMS = logger_example
logger_example_SOURCES = logger_example.cc
logger_example_CPPFLAGS = $(AM_CPPFLAGS)
logger_example_LDFLAGS = $(AM_LDFLAGS)
logger_example_LDADD  = $(top_builddir)/src/lib/log/libkea-log.la
logger_example_LDADD += $(top_builddir)/src/lib/util/libkea-util.la
logger_example_LDADD += $(top_builddir)/src/lib/exceptions/libkea-exceptions.la
logger_example_LDADD += $(AM_LDADD) $(LOG4CPLUS_LIBS)

noinst_PROGRAMS += init_logger_test
init_logger_test_SOURCES = init_logger_test.cc
init_logger_test_CPPFLAGS = $(AM_CPPFLAGS)
init_logger_test_LDFLAGS = $(AM_LDFLAGS)
init_logger_test_LDADD  = $(top_builddir)/src/lib/log/libkea-log.la
init_logger_test_LDADD += $(top_builddir)/src/lib/util/libkea-util.la
init_logger_test_LDADD += $(top_builddir)/src/lib/exceptions/libkea-exceptions.la
init_logger_test_LDADD += $(AM_LDADD) $(LOG4CPLUS_LIBS)

noinst_PROGRAMS += buffer_logger_test
buffer_logger_test_SOURCES = buffer_logger_test.cc
buffer_logger_test_CPPFLAGS = $(AM_CPPFLAGS)
buffer_logger_test_LDFLAGS = $(AM_LDFLAGS)
buffer_logger_test_LDADD  = $(top_builddir)/src/lib/log/libkea-log.la
buffer_logger_test_LDADD += $(top_builddir)/src/lib/util/libkea-util.la
buffer_logger_test_LDADD += $(top_builddir)/src/lib/exceptions/libkea-exceptions.la
buffer_logger_test_LDADD += $(AM_LDADD) $(LOG4CPLUS_LIBS)

noinst_PROGRAMS += logger_lock_test
logger_lock_test_SOURCES = logger_lock_test.cc
logger_lock_test_SOURCES += log_test_messages.cc log_test_messages.h
logger_lock_test_CPPFLAGS = $(AM_CPPFLAGS)
logger_lock_test_LDFLAGS = $(AM_LDFLAGS)
logger_lock_test_LDADD  = $(top_builddir)/src/lib/log/libkea-log.la
logger_lock_test_LDADD += $(top_builddir)/src/lib/util/libkea-util.la
logger_lock_test_LDADD += $(top_builddir)/src/lib/exceptions/libkea-exceptions.la
logger_lock_test_LDADD += $(AM_LDADD) $(LOG4CPLUS_LIBS)

TESTS_ENVIRONMENT = \
	$(LIBTOOL) --mode=execute $(VALGRIND_COMMAND)

if HAVE_GTEST
TESTS =

# Define the flags used in each set of tests
TESTSCXXFLAGS  = $(AM_CXXFLAGS)
TESTSCPPFLAGS  = $(AM_CPPFLAGS) $(GTEST_INCLUDES) $(LOG4CPLUS_INCLUDES)
TESTSLDFLAGS  = $(AM_LDFLAGS) $(GTEST_LDFLAGS)

TESTSLDADD  = $(AM_LDADD)
TESTSLDADD += $(top_builddir)/src/lib/log/libkea-log.la
TESTSLDADD += $(top_builddir)/src/lib/util/unittests/libutil_unittests.la
TESTSLDADD += $(top_builddir)/src/lib/util/libkea-util.la
TESTSLDADD += $(top_builddir)/src/lib/exceptions/libkea-exceptions.la
TESTSLDADD += $(LOG4CPLUS_LIBS) $(GTEST_LDADD)

# Set of unit tests for the general logging classes
TESTS += run_unittests
run_unittests_SOURCES  = run_unittests.cc
run_unittests_SOURCES += log_formatter_unittest.cc
run_unittests_SOURCES += logger_level_impl_unittest.cc
run_unittests_SOURCES += logger_level_unittest.cc
run_unittests_SOURCES += logger_manager_unittest.cc
run_unittests_SOURCES += logger_name_unittest.cc
run_unittests_SOURCES += logger_support_unittest.cc
run_unittests_SOURCES += logger_unittest.cc
run_unittests_SOURCES += logger_specification_unittest.cc
run_unittests_SOURCES += message_dictionary_unittest.cc
run_unittests_SOURCES += message_reader_unittest.cc
run_unittests_SOURCES += output_option_unittest.cc
run_unittests_SOURCES += buffer_appender_unittest.cc
run_unittests_SOURCES += log_test_messages.cc log_test_messages.h

run_unittests_CPPFLAGS = $(TESTSCPPFLAGS)
run_unittests_CXXFLAGS = $(TESTSCXXFLAGS)
run_unittests_LDADD    = $(TESTSLDADD)
run_unittests_LDFLAGS  = $(TESTSLDFLAGS)

# logging initialization tests.  These are put in separate programs to
# ensure that the initialization status at the start of each test is known,
# and to prevent circumstances where the execution of one test affects the
# starting conditions of the next.
TESTS += initializer_unittests_1
initializer_unittests_1_SOURCES  = run_initializer_unittests.cc
initializer_unittests_1_SOURCES += message_initializer_1_unittest.cc
initializer_unittests_1_SOURCES += message_initializer_1a_unittest.cc

initializer_unittests_1_CPPFLAGS = $(TESTSCPPFLAGS)
initializer_unittests_1_CXXFLAGS = $(TESTSCXXFLAGS)
initializer_unittests_1_LDADD    = $(TESTSLDADD)
initializer_unittests_1_LDFLAGS  = $(TESTSLDFLAGS)

noinst_PROGRAMS += $(TESTS)
endif

noinst_SCRIPTS  = console_test.sh
noinst_SCRIPTS += destination_test.sh
noinst_SCRIPTS += init_logger_test.sh
noinst_SCRIPTS += local_file_test.sh
noinst_SCRIPTS += logger_lock_test.sh
noinst_SCRIPTS += severity_test.sh

# Additional test using the shell.  These are principally tests
# where the global logging environment is affected, and where the
# output needs to be compared with stored output (where "cut" and
# "diff" are useful utilities).
check-local:
	$(SHELL) $(abs_builddir)/console_test.sh
	$(SHELL) $(abs_builddir)/destination_test.sh
	$(SHELL) $(abs_builddir)/init_logger_test.sh
	$(SHELL) $(abs_builddir)/buffer_logger_test.sh
	$(SHELL) $(abs_builddir)/local_file_test.sh
	$(SHELL) $(abs_builddir)/logger_lock_test.sh
	$(SHELL) $(abs_builddir)/severity_test.sh

# If we want to get rid of all generated messages files, we need to use
# make maintainer-clean. The proper way to introduce custom commands for
# that operation is to define maintainer-clean-local target. However,
# make maintainer-clean also removes Makefile, so running configure script
# is required.  To make it easy to rebuild messages without going through
# reconfigure, a new target messages-clean has been added.
maintainer-clean-local:
	rm -f log_test_messages.h log_test_messages.cc

# To regenerate messages files, one can do:
#
# make messages-clean
# make messages
#
# This is needed only when a .mes file is modified.
messages-clean: maintainer-clean-local

if GENERATE_MESSAGES

# Define rule to build logging source files from message file
messages: log_test_messages.h log_test_messages.cc
	@echo Message files regenerated

log_test_messages.h log_test_messages.cc: log_test_messages.mes
	$(top_builddir)/src/lib/log/compiler/kea-msg-compiler $(top_srcdir)/src/lib/log/tests/log_test_messages.mes

else

messages log_test_messages.h log_test_messages.cc:
	@echo Messages generation disabled. Configure with --enable-generate-messages to enable it.

endif
